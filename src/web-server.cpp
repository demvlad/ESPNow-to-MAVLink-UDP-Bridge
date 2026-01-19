#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"

static WebServer server(80);

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
      min-width: 80px;
    }

    .wifi-input, select.wifi-input {
      width: 200px;
      padding: 4px;
      border: 2px solid #ddd;
      border-radius: 4px;
      font-size: 12px;
      font-family: monospace;
      letter-spacing: 1px;
      box-sizing: border-box !important;
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
        <label for="wifi_mode">WiFi Mode</label>
        <select class="wifi-input" id="wifi_mode" onchange="wifiModeChange(this.value)">
          <option value="ap">WiFi Access Point</option>
          <option value="sta">WiFi Client (Connect to network)</option>
        </select>
      </div>
      <div class="input-row">
        <label for="wifi_ssid">SSID</label>
        <input class="wifi-input" type="text" id="wifi_ssid"
               title="WIFI SSID">
      </div>
      <div class="input-row">
        <label for="wifi_password">Password</label>
        <input class="wifi-input" type="text" id="wifi_password"
               title="WIFI password">
      </div>
      <div class="input-row">
        <label for="wifi_channel">Channel</label>
        <input class="wifi-input" type="text" id="wifi_channel"
               title="WIFI channel">
      </div>
      <div class="button-group">
        <button class="btn-save" onclick="saveWifi()">💾 Save WIFI</button>
        <button class="btn-current" onclick="getCurrentWifi()">🔄 Current WIFI</button>
      </div>
    </div>
    <div id="status" class="status"></div>
  </div>

  <script>

    // Disable ssid, password change in AP mode
    let externalSSID = "";
    let externalPassword = "";
    let apChannel = "1";
    function wifiModeChange(wifi_mode) {
      const ssid_elem = document.getElementById('wifi_ssid');
      const password_elem = document.getElementById('wifi_password');
      const channel_elem = document.getElementById('wifi_channel');

      if (wifi_mode === "ap") {
        externalSSID = ssid_elem.value;
        ssid_elem.value = "mavlink";
        ssid_elem.readOnly = true;
        ssid_elem.style.cursor = 'not-allowed';

        externalPassword = password_elem.value;
        password_elem.value = "12345678";
        password_elem.readOnly = true;
        password_elem.style.cursor = 'not-allowed';

        channel_elem.value = apChannel;
        channel_elem.readOnly = false;
        channel_elem.style.cursor = 'text';

      } else {
        ssid_elem.value = externalSSID;
        ssid_elem.readOnly = false;
        ssid_elem.style.cursor = 'text';

        password_elem.value = externalPassword;
        password_elem.readOnly = false;
        password_elem.style.cursor = 'text';

        apChannel = channel_elem.value;
        channel_elem.value = "1";
        channel_elem.readOnly = true;
        channel_elem.style.cursor = 'not-allowed';
      }
    }

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
          const ssid_elem = document.getElementById('wifi_ssid');
          const password_elem = document.getElementById('wifi_password');
          const channel_elem = document.getElementById('wifi_channel');
          const wifi_mode_elem = document.getElementById('wifi_mode');

          wifi_mode_elem.value = data.wifi_mode;

          if (data.wifi_mode === "sta") {
            externalSSID = data.wifi_ssid;
            externalPassword = data.wifi_password;

            ssid_elem.value = data.wifi_ssid;
            ssid_elem.readOnly = false;
            ssid_elem.style.cursor = 'text';

            password_elem.value = data.wifi_password;
            password_elem.readOnly = false;
            password_elem.style.cursor = 'text';

            channel_elem.value = "1";
            channel_elem.readOnly = true;
            channel_elem.style.cursor = 'not-allowed';
          } else {
              apChannel = data.wifi_channel;

              ssid_elem.value = "mavlink";
              ssid_elem.readOnly = true;
              ssid_elem.style.cursor = 'not-allowed';

              password_elem.value = "12345678";
              password_elem.readOnly = true;
              password_elem.style.cursor = 'not-allowed';

              channel_elem.value = apChannel;
              channel_elem.readOnly = false;
              channel_elem.style.cursor = 'text';
          }
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

      fetch('/save_mac', {
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
      const wifiMode = document.getElementById('wifi_mode').value;
      const wifiSSID = document.getElementById('wifi_ssid').value.trim();
      const wifiPassword = document.getElementById('wifi_password').value.trim();
      const wifiChannel = document.getElementById('wifi_channel').value;

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
          wifi_channel: parseInt(wifiChannel),
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

// Main page load handler
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Handler to obtain the current MAC
void handleCurrentMac() {
  String currentMac = macToString(config.customMAC);
  server.send(200, "text/plain", currentMac);
}

// Handler to obtain the current WIFI
void handleCurrentWifi() {
  JsonDocument doc;
  doc["wifi_ssid"] = config.wifiSSID;
  doc["wifi_password"] = config.wifiPassword;
  doc["wifi_mode"] = config.wifiMode == AP_WIFI_MODE ? "ap" : "sta";
  doc["wifi_channel"] = config.wifiChannel;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void safeReboot() {
    Serial.println("ESP32 Device reboot...");

    // 1. close all connections
    server.stop();

    // 2. Wait
    delay(500);

    // 3. Reboot
    ESP.restart();
}

// Handler for storing MAC
void handleSaveMac() {
  if (server.hasArg("mac")) {
    String macStr = server.arg("mac");
    Serial.printf(macStr.c_str());

    if (parseMacAddress(macStr.c_str(), config.customMAC)) {
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
#include <ArduinoJson.h>

void handleSaveWifi() {
  String body = server.arg("plain"); // Получаем сырое тело запроса

  Serial.println("Received body: " + body);

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  // Get values
  if (doc["wifi_ssid"].is<String>()) {
    const char* ssid = doc["wifi_ssid"];
    Serial.printf("SSID: %s\n", ssid);
    strcpy(config.wifiSSID, ssid);
  }

  if (doc["wifi_password"].is<String>()) {
    const char* password = doc["wifi_password"];
    Serial.printf("Password: %s\n", password);
    strcpy(config.wifiPassword, password);
  }

  if (doc["wifi_mode"].is<String>()) {
    const char* modeParam = doc["wifi_mode"];
    Serial.printf("Mode: %s\n", modeParam);

    if (strcmp(modeParam, "ap") == 0 || strcmp(modeParam, "0") == 0) {
      config.wifiMode = AP_WIFI_MODE;
    } else if (strcmp(modeParam, "sta") == 0 || strcmp(modeParam, "1") == 0) {
      config.wifiMode = STA_WIFI_MODE;
    } else {
      config.wifiMode = AP_WIFI_MODE; // default
    }
  }

  if (doc["wifi_channel"].is<int>()) {
    config.wifiChannel = doc["wifi_channel"];
    Serial.printf("Channel: %d\n", config.wifiChannel);
  }

  saveWifiToStorage();

  server.send(200, "text/plain", "OK");
  Serial.println("New WIFI settings are saved");
  safeReboot();
}

// Handler for resetting to factory MAC
void handleReset() {
  // Getting the factory MAC
  uint8_t factoryMac[6];
  esp_efuse_mac_get_default(factoryMac);

  memcpy(config.customMAC, factoryMac, 6);
  saveMacToStorage();
  //esp_base_mac_addr_set(config.customMAC);

  server.send(200, "text/plain", "MAC reset to factory settings completed");
  safeReboot();
}

// Handler for WiFi information
void handleInfo() {
  String info = "<html><body>";
  info += "<h2>Device information</h2>";
  info += "<p><strong>Current MAC:</strong> " + macToString(config.customMAC) + "</p>";

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
  loadMacFromStorage();
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
}

void webServerRun() {
    server.handleClient();
}
