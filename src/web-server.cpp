#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "config.h"

static WebServer server(80);
Preferences preferences;

Config config;

// HTML page to input MAC-Address
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 setup</title>
  <meta charset="UTF-8">
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 500px;
      margin: 50px auto;
      padding: 20px;
      background-color: #f5f5f5;
    }
    .container {
      background: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h2 {
      color: #333;
      text-align: center;
      margin-bottom: 30px;
    }
    .input-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: bold;
      color: #555;
    }
    input {
      width: 200px;
      padding: 4px;
      border: 2px solid #ddd;
      border-radius: 4px;
      font-size: 12px;
      font-family: monospace;
      letter-spacing: 1px;
    }
    input:focus {
      border-color: #4CAF50;
      outline: none;
    }
    .mac-example {
      font-size: 14px;
      color: #666;
      margin-top: 5px;
      font-family: monospace;
    }
    .button-group {
      display: flex;
      gap: 10px;
      margin-top: 30px;
    }
    button {
      flex: 1;
      padding: 14px;
      border: none;
      border-radius: 6px;
      font-size: 16px;
      font-weight: bold;
      cursor: pointer;
      transition: background 0.3s;
    }
    .btn-save {
      background: #4CAF50;
      color: white;
    }
    .btn-save:hover {
      background: #45a049;
    }
    .btn-current {
      background: #2196F3;
      color: white;
    }
    .btn-current:hover {
      background: #0b7dda;
    }
    .status {
      margin-top: 20px;
      padding: 15px;
      border-radius: 6px;
      text-align: center;
      display: none;
    }
    .success {
      background: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
    }
    .error {
      background: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
    }
    .info-box {
      background: #e3f2fd;
      border-left: 4px solid #2196F3;
      padding: 15px;
      margin-bottom: 25px;
      border-radius: 4px;
    }
    .current-mac {
      font-family: monospace;
      background: #f8f9fa;
      padding: 10px;
      border-radius: 4px;
      margin: 10px 0;
      font-weight: bold;
      text-align: center;
      font-size: 18px;
      color: #333;
    }
    .input-row {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-bottom: 5px;
    }
    label {
      min-width: 80px; /* Фиксированная ширина для выравнивания */
    }

    .wifi-input, select.wifi-input {
      width: 200px;
      padding: 4px;
      border: 2px solid #ddd;
      border-radius: 4px;
      font-size: 12px;
      font-family: monospace;
      letter-spacing: 1px;
      box-sizing: border-box !important; /* ← Это ключевое! */
    }

  </style>
</head>
<body>
  <div class="container">
    <h2>🔧 ESP32 MAC-Address setup</h2>

    <div class="info-box">
      <strong>Current ESP32 MAC-Address:</strong>
      <div id="currentMac" class="current-mac">Load...</div>
    </div>

    <div class="input-group">
      <label for="mac">New ESP32 MAC-Address:</label>
      <input type="text" id="mac" name="mac"
             placeholder="AA:BB:CC:DD:EE:FF"
             pattern="^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"
             title="Enter the MAC in the format XX:XX:XX:XX:XX:XX">
      <div class="mac-example">Format: AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF</div>
    </div>

    <div class="button-group">
      <button class="btn-save" onclick="saveMac()">💾 Save MAC</button>
      <button class="btn-current" onclick="getCurrentMac()">🔄 Current MAC</button>
    </div>

    <h2>🔧 WIFI setup</h2>
    <div class="input-group">
      <div class="input-row">
        <label for="wifi-mode">WiFi Mode</label>
        <select class="wifi-input" id="wifi-mode">
          <option value="ap">WiFi Access Point</option>
          <option value="sta">WiFi Client (Connect to network)</option>
        </select>
      </div>
      <div class="input-row">
        <label for="ssid">SSID</label>
        <input class="wifi-input" type="text" id="ssid"
               title="WIFI SSID">
      </div>
      <div class="input-row">
        <label for="password">Password</label>
        <input class="wifi-input" type="text" id="password"
               title="WIFI password">
      </div>
      <div class="button-group">
        <button class="btn-save" onclick="saveWifi()">💾 Save WIFI</button>
        <button class="btn-current" onclick="getCurrentWifi()">🔄 Current WIFI</button>
      </div>
    </div>
    <div id="status" class="status"></div>
  </div>

  <script>
    // Get current MAC by page loading
    window.onload = function() {
      getCurrentMac();
      getCurrentWifi();
    };

    // Get MAC from server
    function getCurrentMac() {
      fetch('/current_mac')
        .then(response => response.text())
        .then(data => {
          document.getElementById('currentMac').textContent = data;
          document.getElementById('mac').value = data;
        })
        .catch(error => {
          showStatus('Error retrieving MAC address', 'error');
        });
    }

    // Get WIFI from server
    function getCurrentWifi() {
      fetch('/current_wifi')
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
          }
          return response.json();
        })
        .then(data => {
          document.getElementById('wifi_ssid').value = data.wifi_ssid;
          document.getElementById('password').value = data.password;
          document.getElementById('wifi-mode').value = data.wifi_mode;
        })
        .catch(error => {
          showStatus('Error retrieving WIFI settings', 'error');
        });
    }

    // Save new MAC
    function saveMac() {
      const macInput = document.getElementById('mac').value.trim();

      // Validate MAC
      if (!macInput || !isValidMac(macInput)) {
        showStatus('Enter valid MAC-address', 'error');
        return;
      }

      fetch('/save', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'mac=' + encodeURIComponent(macInput)
      })
      .then(response => response.text())
      .then(data => {
        if (data === 'OK') {
          showStatus('MAC successfully saved! Device reboot...', 'success');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          showStatus('Error: ' + data, 'error');
        }
      })
      .catch(error => {
        showStatus('Network error', 'error');
      });
    }

    // MAC format cheking
    function isValidMac(mac) {
      const regex = /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/;
      return regex.test(mac);
    }

    // Save new WIFI
    function saveWifi() {
      const wifiMode = document.getElementById('wifi-mode').value;
      const wifiSSID = document.getElementById('wifi_ssid').value.trim();
      const wifiPassword = document.getElementById('password').value.trim();

      // Validate WIFI
      if (!wifiSSID) {
        showStatus('Enter valid wifi ssid', 'error');
        return;
      }

      if (!wifiPassword) {
        showStatus('Enter valid password', 'error');
        return;
      }

      fetch('/save_wifi', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          wifi_mode: wifiMode,
          wifi_ssid: wifiSSID,
          wifi_password: wifiPassword,
        })
      })
      .then(response => response.text())
      .then(data => {
        if (data === 'OK') {
          showStatus('WIFI settings successfully saved! Device reboot...', 'success');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          showStatus('Error: ' + data, 'error');
        }
      })
      .catch(error => {
        showStatus('Network error', 'error');
      });
    }

    // Show status
    function showStatus(message, type) {
      const statusDiv = document.getElementById('status');
      statusDiv.textContent = message;
      statusDiv.className = 'status ' + type;
      statusDiv.style.display = 'block';

      // Автоматически скрыть через 5 секунд
      setTimeout(() => {
        statusDiv.style.display = 'none';
      }, 5000);
    }

    // Separators autofill
    document.getElementById('mac').addEventListener('input', function(e) {
      let value = e.target.value.replace(/[^0-9A-Fa-f]/g, '').toUpperCase();
      if (value.length > 12) value = value.substr(0, 12);


      let formatted = '';
      for (let i = 0; i < value.length; i++) {
        if (i > 0 && i % 2 === 0) {
          formatted += ':';
        }
        formatted += value[i];
      }

      e.target.value = formatted;
    });
  </script>
</body>
</html>
)rawliteral";

// Store MAC to Preferences (EEPROM)
void saveMacToStorage() {
  preferences.begin("mac-config", false);
  preferences.putBytes("mac", config.mac, 6);
  preferences.end();
  Serial.println("MAC is saved in storage");
}

// Load MAC from Preferences
void loadMacFromStorage(uint8_t mac[6]) {
  preferences.begin("mac-config", false);
  size_t len = preferences.getBytes("mac", config.mac, 6);
  preferences.end();

  if (len != 6) {
    // Default MAC
    uint8_t factoryMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    esp_efuse_mac_get_default(factoryMac);
    memcpy(config.mac, factoryMac, 6);
    Serial.printf("The factory MAC is: %2x:%2x:%2x:%2x:%2x:%2x",
                                factoryMac[0], factoryMac[1], factoryMac[2],
                                factoryMac[3], factoryMac[4], factoryMac[5]);
    Serial.println();
  } else {
    Serial.println("MAC has been loaded from storage");
  }
  if (mac) {
    memcpy(mac, config.mac, 6);
  }
}

// Store WIFI to Preferences (EEPROM)
void saveWifiToStorage() {
  preferences.begin("wifi-config", false);
  preferences.putString("wifi-ssid", config.wifi_ssid);
  preferences.putString("wifi-password", config.wifi_password);
  preferences.putString("wifi-mode", config.wifi_mode);
  preferences.end();
  Serial.println("WIFI settings are saved in storage");
}

// Load MAC from Preferences
void loadWifiFromStorage() {
  preferences.begin("wifi-config", false);
  config.wifi_mode = preferences.getString("wifi-mode", "ap");
  config.wifi_ssid = preferences.getString("wifi-ssid", "mavlink");
  config.wifi_password = preferences.getString("wifi-password", "12345678");
  preferences.end();
}

// MAC string parsing
bool parseMacAddress(const char* macStr, uint8_t* mac) {
  if (strlen(macStr) != 17) return false;

  int values[6];
  int count = sscanf(macStr, "%x:%x:%x:%x:%x:%x",
                    &values[0], &values[1], &values[2],
                    &values[3], &values[4], &values[5]);

  if (count != 6) {

    count = sscanf(macStr, "%x-%x-%x-%x-%x-%x",
                  &values[0], &values[1], &values[2],
                  &values[3], &values[4], &values[5]);
  }

  if (count != 6) return false;

  for (int i = 0; i < 6; i++) {
    if (values[i] < 0 || values[i] > 255) return false;
    mac[i] = (uint8_t)values[i];
  }

  return true;
}

// MAC to string conversion
String macToString(uint8_t* mac) {
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

// Main page load handler
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Handler to obtain the current MAC
void handleCurrentMac() {
  String currentMac = macToString(config.mac);
  server.send(200, "text/plain", currentMac);
}

// Handler to obtain the current WIFI
void handleCurrentWifi() {
  StaticJsonDocument<100> doc;
  doc["wifi_ssid"] = config.wifi_ssid;
  doc["password"] = config.wifi_password;
  doc["wifi_mode"] = config.wifi_mode;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void safeReboot() {
    Serial.println("ESP32 Device reboot...");

    // 1. close all connections
    server.stop();

    // 2. Save data
    preferences.end();

    // 3. Wait
    delay(500);

    // 4. Reboot
    ESP.restart();
}

// Handler for storing MAC
void handleSaveMac() {
  if (server.hasArg("mac")) {
    String macStr = server.arg("mac");

    if (parseMacAddress(macStr.c_str(), config.mac)) {
      saveMacToStorage();

      server.send(200, "text/plain", "OK");
      Serial.println("New MAC saved: " + macStr);
      safeReboot();
    } else {
      server.send(400, "text/plain", "Wrong MAC format");
    }
  } else {
    server.send(400, "text/plain", "MAC is not specified");
  }
}

// Handler for storing WIFI
void handleSaveWifi() {
  if (server.hasArg("wifi_ssid")) {
    config.wifi_ssid = server.arg("wifi_ssid");
  }

  if (server.hasArg("wifi_password")) {
    config.wifi_password = server.arg("wifi_password");
  }

  if (server.hasArg("wifi_mode")) {
    config.wifi_mode = server.arg("wifi_mode");
  }

  saveWifiToStorage();

  server.send(200, "text/plain", "OK");
  Serial.println("New WIFI settings are saved");
}

// Handler for resetting to factory MAC
void handleReset() {
  // Getting the factory MAC
  uint8_t factoryMac[6];
  esp_efuse_mac_get_default(factoryMac);

  memcpy(config.mac, factoryMac, 6);
  saveMacToStorage();
  //esp_base_mac_addr_set(config.mac);

  server.send(200, "text/plain", "MAC reset to factory settings completed");
  safeReboot();
}

// Handler for WiFi information
void handleInfo() {
  String info = "<html><body>";
  info += "<h2>Device information</h2>";
  info += "<p><strong>Current MAC:</strong> " + macToString(config.mac) + "</p>";

  uint8_t factoryMac[6];
  esp_efuse_mac_get_default(factoryMac);
  info += "<p><strong>Factory MAC:</strong> " + macToString(factoryMac) + "</p>";

  info += "<p><strong>IP address:</strong> " + WiFi.localIP().toString() + "</p>";
  info += "<p><strong>MAC WiFi:</strong> " + WiFi.macAddress() + "</p>";
  info += "<br><p><a href='/'>Go back to settings</a></p>";
  info += "</body></html>";

  server.send(200, "text/html", info);
}

void webSwerverSetup() {
  Serial.println("------------------------------------------------");
  // Loading saved MAC
  loadMacFromStorage(NULL);
  // Loading saved WIFI
  loadWifiFromStorage();

    // Setting up a web server
  server.on("/", handleRoot);
  server.on("/current_mac", handleCurrentMac);
  server.on("/current_wifi", handleCurrentWifi);
  server.on("/save_mac", HTTP_POST, handleSaveMac);
  server.on("/save_wifi", HTTP_POST, handleSaveWifi);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/info", handleInfo);

  server.begin();
  Serial.println("------------------------------------------------");
  Serial.println("HTTP server is running");
  Serial.print("Current MAC: ");
  Serial.println(macToString(config.mac));
  Serial.println("To change MAC address: Open the browser and go to the address: http://192.168.4.1");

}

void webServerRun() {
    server.handleClient();
}