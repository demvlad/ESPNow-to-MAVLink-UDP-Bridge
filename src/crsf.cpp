#include <Arduino.h>
#include "crsf.h"

// CRSF Frame Types
typedef enum {
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_GPS_EXTENDED = 0x06,
    CRSF_FRAMETYPE_VARIO = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_BARO_ALTITUDE = 0x09,
    CRSF_FRAMETYPE_HEARTBEAT = 0x0B,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED = 0x17,
    CRSF_FRAMETYPE_LINK_STATISTICS_RX = 0x1C,
    CRSF_FRAMETYPE_LINK_STATISTICS_TX = 0x1D,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
    CRSF_FRAMETYPE_COMMAND = 0x32,
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C,
    CRSF_FRAMETYPE_DISPLAYPORT_CMD = 0x7D,
} crsf_frame_type_e;

// CRSF data struct
typedef struct {
    uint16_t voltage;       // mV * 100
    uint16_t current;       // mA * 100
    uint32_t capacity : 24; // mAh
    uint8_t remaining;      // percent
} crsfBatterySensor_t;

typedef struct {
    int16_t pitch;          // rad / 10000
    int16_t roll;           // rad / 10000
    int16_t yaw;            // rad / 10000
} crsfAttitude_t;

typedef struct {
    int32_t latitude;       // degree / 10,000,000
    int32_t longitude;      // degree / 10,000,000
    uint16_t groundspeed;   // km/h * 10
    uint16_t heading;       // degree * 100
    uint16_t altitude;      // meters (meters + 1000)
    uint8_t satellites;
} crsfGps_t;

typedef struct {
    uint8_t fix_type;       // Current GPS fix quality
    int16_t n_speed;        // Northward (north = positive) Speed [cm/sec]
    int16_t e_speed;        // Eastward (east = positive) Speed [cm/sec]
    int16_t v_speed;        // Vertical (up = positive) Speed [cm/sec]
    int16_t h_speed_acc;    // Horizontal Speed accuracy cm/sec
    int16_t track_acc;      // Heading accuracy in degrees scaled with 1e-1 degrees times 10)
    int16_t alt_ellipsoid;  // Meters Height above GPS Ellipsoid (not MSL)
    int16_t h_acc;          // horizontal accuracy in cm
    int16_t v_acc;          // vertical accuracy in cm
    uint8_t reserved;
    uint8_t hDOP;           // Horizontal dilution of precision,Dimensionless in nits of.1.
    uint8_t vDOP;           // vertical dilution of precision, Dimensionless in nits of .1.
} crsfGpsExt_t;

typedef struct {
    uint8_t uplinkRSSI1;          // RSSI of uplink (signal strength)
    uint8_t uplinkRSSI2;          // RSSI of uplink (signal strength)
    uint8_t uplinkLinkQuality;    // Link quality of uplink (0-100%)
    int8_t uplinkSNR;             // SNR of uplink
    uint8_t activeAntenna;        // Active antenna
    uint8_t rfMode;               // RF mode
    uint8_t uplinkTXPower;        // Transmit power of uplink (enum)
    uint8_t downlinkRSSI;         // RSSI of downlink (signal strength)
    uint8_t downlinkLinkQuality;  // Link quality of downlink (0-100%)
    int8_t downlinkSNR;           // SNR of downlink
} crsfLinkStatistics_t;

// big-endian transform
int16_t bigEndian16(const uint8_t* bytes) {
    return (bytes[0] << 8) | bytes[1];
}


int32_t bigEndian24(const uint8_t* bytes) {
    return (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
}

int32_t bigEndian32(const uint8_t* bytes) {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

// ExpressLRS CRC checking
uint8_t crsfCRC(const uint8_t* data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0xD5;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Data parser
bool parseCRSFPacket(const uint8_t *data, int len, TelemetryData_t* telemetry) {
    // Fast checking
    if (len < 11) return false;
    if (data[0] != 0x24 || data[1] != 0x58 || data[2] != 0x3C) return false;

    // CRSF data start point
    const uint8_t *crsfData = data + 8;
    int crsfLen = len - 8;
    if (crsfLen < 4) return false;

    // read header
    uint8_t frame_len = crsfData[1];
    uint8_t frame_type = crsfData[2];

    if (frame_len < 3 || frame_len > crsfLen) return false;

    uint8_t payload_len = frame_len - 2;
    const uint8_t* payload = crsfData + 3;

    // Statistic update
    telemetry->statistic.packetCount++;
    if (frame_type < 256) {
        telemetry->statistic.crsfPackets[frame_type]++;
    }
    telemetry->lastUpdate = millis();

    // Data packets handle
    switch (frame_type) {

        case CRSF_FRAMETYPE_GPS: // GPS
            if (payload_len >= sizeof(crsfGps_t)) {
                telemetry->gps.enabled = true;
                telemetry->gps.latitude = bigEndian32(payload) / 10000000.0; // [degree]
                telemetry->gps.longitude = bigEndian32(payload + 4) / 10000000.0; // [degree]
                telemetry->gps.groundSpeed = bigEndian16(payload + 8) / 10.0f / 3.6; // [m/s]
                telemetry->gps.heading = bigEndian16(payload + 10) * 0.01f; // [degree]
                telemetry->gps.altitude = bigEndian16(payload + 12) - 1000.0f; // [m] The BF CRFS telemetry add  +1000
                telemetry->gps.satellites = payload[14];
            }
            break;

        case CRSF_FRAMETYPE_GPS_EXTENDED: // GPS ext
            if (payload_len >= sizeof(crsfGpsExt_t)) {
                telemetry->gps_ext.enabled = true;
                telemetry->gps_ext.fix_type = payload[0];
                telemetry->gps_ext.n_speed = bigEndian16(payload + 1) / 100.0f;     // [m/s]
                telemetry->gps_ext.e_speed = bigEndian16(payload + 3) / 100.0f;     // [m/s]
                telemetry->gps_ext.up_speed = bigEndian16(payload + 5) / 100.0f;     // [m/s]
                telemetry->gps_ext.h_speed_acc = bigEndian16(payload + 7) / 100.0f;  // [m/s]
                telemetry->gps_ext.track_acc = bigEndian16(payload + 9) / 10.0f;     // [deg]
                telemetry->gps_ext.alt_ellipsoid = (float)bigEndian16(payload + 11);     // [m]
                telemetry->gps_ext.h_acc = bigEndian16(payload + 13) / 10.0f;     // [m]
                telemetry->gps_ext.v_acc = bigEndian16(payload + 15) / 10.0f;     // [m]
                telemetry->gps_ext.hDOP = payload[18] / 10.0f;     // units
                telemetry->gps_ext.vDOP = payload[19] / 10.0f;     // units
            }
            break;

        case CRSF_FRAMETYPE_BATTERY_SENSOR: // Battery
            if (payload_len >= sizeof(crsfBatterySensor_t)) {
                telemetry->battery.enabled = true;
                telemetry->battery.voltage = bigEndian16(payload) * 0.1f;
                telemetry->battery.current = bigEndian16(payload + 2) * 0.1f;
                telemetry->battery.capacity = bigEndian24(payload + 4);
                telemetry->battery.remaining = payload[7];
            }
            break;

        case CRSF_FRAMETYPE_ATTITUDE: // Attitude
            if (payload_len >= sizeof(crsfAttitude_t)) {
                telemetry->attitude.enabled = true;
                telemetry->attitude.pitch = bigEndian16(payload) / 10000.0f; // rad*10000 → rad
                telemetry->attitude.roll = bigEndian16(payload + 2) / 10000.0f; // rad*10000 → rad
                telemetry->attitude.yaw = bigEndian16(payload + 4) / 10000.0f; // rad*10000 → rad
            }
            break;

        case CRSF_FRAMETYPE_FLIGHT_MODE: // Flight Mode
            if (payload_len >= 1) {
                telemetry->flightMode.enabled = true;
                int len = payload_len < 16 ? payload_len : 16;
                memcpy(telemetry->flightMode.mode, payload, len);
                telemetry->flightMode.mode[len] = '\0';
            }
            break;
    }

    return true;
}
