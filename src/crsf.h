#ifndef CRSF_H
#define CRSF_H
#include <Arduino.h>

// Телеметрия
struct TelemetryData_t {
    struct {
        bool enabled;
        float voltage;
        float current;
        uint32_t capacity;
        uint8_t remaining;
    } battery;

    struct {
        bool enabled;
        float pitch;
        float roll;
        float yaw;
     } attitude;

    struct {
        bool enabled;
        char mode[17];
    } flightMode;

    struct {
        bool enabled;
        double latitude;
        double longitude;
        float altitude;
        float groundSpeed;
        float heading;
        uint8_t satellites;
    } gps;

    struct {
        bool enabled;
        uint8_t fix_type;     // Current GPS fix quality
        float n_speed;        // Northward (north = positive) Speed [m/sec]
        float e_speed;        // Eastward (east = positive) Speed [m/sec]
        float up_speed;       // Vertical (up = positive) Speed [m/sec]
        float h_speed_acc;    // Horizontal Speed accuracy m/sec
        float track_acc;      // Heading accuracy in degrees scaled with 1e-1 degrees times 10)
        float alt_ellipsoid;  // Meters Height above GPS Ellipsoid (not MSL)
        float h_acc;          // horizontal accuracy in m
        float v_acc;          // vertical accuracy in m
        float hDOP;           // Horizontal dilution of precision,Dimensionless in nits
        float vDOP;           // vertical dilution of precision, Dimensionless in nits
    } gps_ext;

    struct {
        uint32_t packetCount;
        uint32_t crsfPackets[256];
    } statistic;
    unsigned long lastUpdate;
};

bool parseCRSFPacket(const uint8_t *data, int len, TelemetryData_t* telemetry);
#endif

