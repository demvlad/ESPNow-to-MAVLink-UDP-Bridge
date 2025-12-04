#include <Arduino.h>

// Телеметрия
struct TelemetryData {
    float voltage;
    float current;
    uint32_t capacity;
    uint8_t batteryRemaining;
    float pitch;
    float roll;
    float yaw;
    char flightMode[17];
    double latitude;
    double longitude;
    float altitude;
    float groundSpeed;
    float heading;
    uint8_t satellites;
    int8_t uplinkRSSI1;
    int8_t uplinkRSSI2;
    uint8_t uplinkLinkQuality;
    int8_t uplinkSNR;
    uint8_t activeAntenna;
    uint8_t rfMode;
    uint8_t uplinkTXPower;
    int8_t downlinkRSSI;
    uint8_t downlinkLinkQuality;
    int8_t downlinkSNR;
    uint32_t packetCount;
    uint32_t crsfPackets[256];
    unsigned long lastUpdate;
    uint8_t linkQuality;
};

bool parseCRSFPacket(const uint8_t *data, int len, TelemetryData* telemetry);
