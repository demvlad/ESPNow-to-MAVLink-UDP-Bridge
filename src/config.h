#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// WIFI access point setup
const char* ap_ssid = "mavlink";
const char* ap_password = "12345678";  // It needs 8 symbols

const uint16_t UDP_PORT = 14550; // GCS UDP port

//#define DEBUG_TO_LOG    // set this define to log telemetries data
#endif