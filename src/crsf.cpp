#include <Arduino.h>
#include "crsf.h"

// CRSF Frame Types
typedef enum {
    CRSF_FRAMETYPE_GPS = 0x02,
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

// Структуры данных CRSF
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

// Функция проверки CRC (из ExpressLRS)
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

// Парсинг пакета - ОСНОВНОЕ ИСПРАВЛЕНИЕ
bool parseCRSFPacket(const uint8_t *data, int len, TelemetryData* telemetry) {
    // Быстрые проверки
    if (len < 11) return false;
    if (data[0] != 0x24 || data[1] != 0x58 || data[2] != 0x3C) return false;

    // CRSF данные начинаются с байта 8
    const uint8_t *crsfData = data + 8;
    int crsfLen = len - 8;
    if (crsfLen < 4) return false;

    // Чтение заголовка
    uint8_t frame_len = crsfData[1];
    uint8_t frame_type = crsfData[2];

    if (frame_len < 3 || frame_len > crsfLen) return false;

    uint8_t payload_len = frame_len - 2;
    const uint8_t* payload = crsfData + 3;

    // Обновление статистики
    telemetry->packetCount++;
    if (frame_type < 256) telemetry->crsfPackets[frame_type]++;
    telemetry->lastUpdate = millis();

    // Обработка пакетов
    switch (frame_type) {
        case CRSF_FRAMETYPE_BATTERY_SENSOR: // Battery
            if (payload_len >= 8) {
                uint16_t v = (payload[0] << 8) | payload[1];
                uint16_t c = (payload[2] << 8) | payload[3];
                telemetry->voltage = v * 0.1f;
                telemetry->current = c * 0.1f;
                telemetry->capacity = (payload[4] << 16) | (payload[5] << 8) | payload[6];
                telemetry->batteryRemaining = payload[7];
            }
            break;

        case CRSF_FRAMETYPE_ATTITUDE: // Attitude
            if (payload_len >= 6) {
                int16_t p = (int16_t)((payload[0] << 8) | payload[1]);
                int16_t r = (int16_t)((payload[2] << 8) | payload[3]);
                int16_t y = (int16_t)((payload[4] << 8) | payload[5]);
                telemetry->pitch = p * 0.00572957795f; // rad/10000 → градусы
                telemetry->roll = r * 0.00572957795f;
                telemetry->yaw = y * 0.00572957795f;
            }
            break;

        case CRSF_FRAMETYPE_LINK_STATISTICS: // Link Statistics
            if (payload_len >= 10) {
                telemetry->uplinkRSSI1 = (payload[0] / 2) - 120;
                telemetry->uplinkRSSI2 = (payload[1] / 2) - 120;
                telemetry->uplinkLinkQuality = payload[2];
                telemetry->uplinkSNR = (int8_t)payload[3];
                telemetry->downlinkRSSI = (payload[7] / 2) - 120;
                telemetry->downlinkLinkQuality = payload[8];
                telemetry->downlinkSNR = (int8_t)payload[9];
            }
            break;

        case CRSF_FRAMETYPE_FLIGHT_MODE: // Flight Mode
            if (payload_len >= 1) {
                int l = payload_len < 16 ? payload_len : 16;
                memcpy(telemetry->flightMode, payload, l);
                telemetry->flightMode[l] = '\0';
            }
            break;
    }

    return true;
}
