#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// WIFI access point setup
const char* ap_ssid = "mavlink";
const char* ap_password = "12345678";  // It needs 8 symbols

const uint16_t UDP_PORT = 14550; // GCS UDP port

struct Config {
  uint8_t mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Default MAC
  String wifi_mode = "ap";
  String wifi_ssid = "mavlink";
  String wifi_password = "12345678";
};
extern Config config;
#endif