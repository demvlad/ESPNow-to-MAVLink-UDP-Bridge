#include <Preferences.h>
#include <esp_wifi.h>
#include <string.h>

#include "config.h"

static Preferences preferences;
Config config;

// Store MAC to Preferences (EEPROM)
void saveMacToStorage() {
  preferences.begin("mac-config", false);
  preferences.putBytes("mac", config.customMAC, 6);
  preferences.end();
  Serial.println("MAC is saved in storage");
}

// Load MAC from Preferences
void loadMacFromStorage() {
  preferences.begin("mac-config", false);
  size_t len = preferences.getBytes("mac", config.customMAC, 6);
  preferences.end();
  if (len != 6) {
    // Default MAC
    Serial.println("MAC congig not fount. The factory MAC is used");
    uint8_t factoryMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    esp_efuse_mac_get_default(factoryMac);
    memcpy(config.customMAC, factoryMac, 6);
    Serial.printf("The factory MAC is: %2x:%2x:%2x:%2x:%2x:%2x",
                                factoryMac[0], factoryMac[1], factoryMac[2],
                                factoryMac[3], factoryMac[4], factoryMac[5]);
    Serial.println("Set MAC of your ELRS instead of!!!!");
    Serial.println();
  } else {
    Serial.println("MAC has been loaded from storage");
  }
}

// Store WIFI settings to Preferences (EEPROM)
void saveWifiToStorage() {
  preferences.begin("wifi_config", false);
  preferences.putString("wifi_ssid", config.wifiSSID);
  preferences.putString("wifi_password", config.wifiPassword);
  preferences.putUChar("wifi_mode", config.wifiMode);
  preferences.putUChar("wifi_channel", config.wifiChannel);
  preferences.end();
  Serial.println("WIFI settings are saved in storage");
}

// Load WIFI settings from Preferences
void loadWifiFromStorage() {
  preferences.begin("wifi_config", false);
  config.wifiMode = (WiFiOperationMode)preferences.getUChar("wifi_mode", 0);
  String ssidValue = preferences.getString("wifi_ssid", apSSID);
  strcpy(config.wifiSSID, ssidValue.c_str());
  String passValue = preferences.getString("wifi_password", apPassword);
  strcpy(config.wifiPassword, passValue.c_str());

  config.wifiChannel = preferences.getUChar("wifi_channel", 1);
  preferences.end();
}

// MAC to string conversion
String macToString(uint8_t* mac) {
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}