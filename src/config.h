#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
// 4E:52:A6:FB:23:EA my MAC
const uint16_t UDP_PORT = 14550; // GCS UDP port
const char apSSID[] = "mavlink";
const char apPassword[] = "12345678";
typedef enum {
    AP_WIFI_MODE = 0,      // Точка доступа
    STA_WIFI_MODE = 1      // Клиент
} WiFiOperationMode;

struct Config {
  uint8_t customMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Custom MAC
  WiFiOperationMode wifiMode;
  char wifiSSID[16];
  char wifiPassword[32];
  uint8_t wifiChannel;
};

void saveMacToStorage();
void loadMacFromStorage();
void saveWifiToStorage();
void loadWifiFromStorage();
String macToString(uint8_t* mac);

extern Config config;
#endif