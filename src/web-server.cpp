#include <WebServer.h>
#include <ArduinoJson.h>
#include <mbedtls/md5.h>
#include "config.h"
#include "crsf.h"

extern TelemetryData_t telemetriesData;
const char* resetReasonName(esp_reset_reason_t reason);

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
      height: auto;
    }
    .container {
      background: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      height: auto;
      min-height: auto;
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

    .input-row.stacked {
      display: block;
    }

    .input-row.stacked label {
      display: block;
      min-width: 0;
      margin-bottom: 5px;
    }

    .chain-help {
      margin-top: 15px;
      font-size: 13px;
      color: #444;
    }

    .chain-help summary {
      cursor: pointer;
      font-weight: bold;
      color: #333;
    }

    .chain-help ol {
      margin: 8px 0 0;
      padding-left: 20px;
    }

    .chain-help li {
      margin-bottom: 5px;
    }

    .btn-eye {
      border: none;
      background: transparent;
      cursor: pointer;
      font-size: 16px;
      padding: 0 4px;
      line-height: 1;
    }

    .tabs-container {
      width: 100%;
      background: white;
      overflow: hidden;
    }

    .tabs-header {
      display: flex;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-bottom: 1px solid #ddd;
    }

    .lang-select {
      margin-left: auto;
      align-self: center;
      margin-right: 10px;
      padding: 3px 4px;
      border: 1px solid rgba(255,255,255,0.35);
      border-radius: 4px;
      background: rgba(255,255,255,0.15);
      color: #fff;
      font-size: 12px;
      cursor: pointer;
    }

    .lang-select option {
      color: #333;
    }

    .tab-content {
      display: none;
      animation: fadeIn 0.5s ease-in-out;
      height: auto;
    }

    .tab-content.active {
      display: block;
    }

    .tab-btn {
      flex: 1;
      padding: 15px 10px;
      background: none;
      border: none;
      color: white;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      position: relative;
    }

    .tab-btn::after {
      content: '';
      position: absolute;
      bottom: 0;
      left: 50%;
      width: 0;
      height: 3px;
      background: #ffeb3b;
      transition: all 0.3s ease;
      transform: translateX(-50%);
    }

    .tab-btn:hover {
      background: rgba(255,255,255,0.1);
    }

    .tab-btn.active {
      background: rgba(255,255,255,0.15);
    }

    .tab-btn.active::after {
      width: 80%;
    }

  </style>
</head>

<body>
  <div class="container">
    <div class="tabs-container">
      <div class="tabs-header">
        <button class="tab-btn active" data-tab="mac-tab" id="mac-tab-btn">🔧 MAC-Address</button>
        <button class="tab-btn" data-tab="wifi-tab" id="wifi-tab-btn">📶 WIFI</button>
        <button class="tab-btn" data-tab="status-tab" id="status-tab-btn">📊 <span data-i18n="tab.status">Status</span></button>
        <select id="lang_select" class="lang-select" onchange="setLangPref(this.value)" title="Language">
          <option value="auto" data-i18n="lang.auto">Auto</option>
          <option value="en">English</option>
          <option value="ru">Русский</option>
        </select>
      </div>

      <div id="mac-tab" class="tab-content active">
        <div class="info-box">
          <strong data-i18n="mac.current">Current ESP32 MAC-Address:</strong>
          <div id="currentMac" class="current-mac">Load...</div>
        </div>

        <div class="input-group">
          <label for="mac" data-i18n="mac.new">New ESP32 MAC-Address:</label>
          <input type="text" id="mac" name="mac"
                 placeholder="AA:BB:CC:DD:EE:FF"
                 pattern="^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"
                 title="Enter the MAC in the format XX:XX:XX:XX:XX:XX">
          <div class="mac-example" data-i18n="mac.format">Format: AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF</div>
        </div>

        <div class="input-group">
          <label for="phrase" data-i18n="phrase.label">Or set from the ELRS binding phrase:</label>
          <input type="password" id="phrase" name="phrase"
                 placeholder="my_binding_phrase"
                 title="The same binding phrase you use in ExpressLRS">
          <button class="btn-eye" id="phrase_toggle"
                  onclick="toggleVisibility('phrase', 'phrase_toggle')"
                  title="Show binding phrase">👁</button>
          <div class="mac-example" data-i18n="phrase.hint">The MAC is derived from the phrase exactly like ExpressLRS does</div>
        </div>

        <div class="button-group">
          <button class="btn-save" onclick="saveMac()" data-i18n="mac.save">💾 Save MAC</button>
          <button class="btn-save" onclick="savePhrase()" data-i18n="phrase.save">🔑 Save from phrase</button>
          <button class="btn-current" onclick="getCurrentMac()" data-i18n="mac.load">🔄 Current MAC</button>
        </div>
      </div>

      <div id="wifi-tab" class="tab-content">
        <div class="input-group">
          <div class="input-row">
            <label for="wifi_mode" data-i18n="wifi.mode">WiFi Mode</label>
            <select class="wifi-input" id="wifi_mode" onchange="wifiModeChange(this.value)">
              <option value="ap" data-i18n="wifi.modeAp">WiFi Access Point</option>
              <option value="sta" data-i18n="wifi.modeSta">WiFi Client (Connect to network)</option>
            </select>
          </div>
          <div class="input-row">
            <label for="wifi_ssid">SSID</label>
            <input class="wifi-input" type="text" id="wifi_ssid"
                   title="WIFI SSID">
          </div>
          <div class="input-row">
            <label for="wifi_password" data-i18n="wifi.password">Password</label>
            <input class="wifi-input" type="password" id="wifi_password"
                   title="WIFI password">
            <button class="btn-eye" id="password_toggle"
                    onclick="toggleVisibility('wifi_password', 'password_toggle')"
                    title="Show password">👁</button>
          </div>
          <div class="input-row stacked">
            <label for="wifi_channel" data-i18n="wifi.channel">ESP-NOW channel (ELRS backpack: 1, change only if you are sure)</label>
            <input class="wifi-input" type="text" id="wifi_channel"
                   title="ESP-NOW channel, must match the ELRS backpack and the WiFi network">
          </div>

          <details class="chain-help">
            <summary data-i18n="chain.summary">How the ESP-NOW chain works</summary>
            <ol>
              <li data-i18n="chain.1">The drone sends CRSF telemetry to the ELRS transmitter (TX), and the TX backpack forwards it over ESP-NOW to the address derived from the binding phrase (UID), on channel 1.</li>
              <li data-i18n="chain.2">The ESP32 must use the same MAC as that UID: set it on the MAC tab manually or enter the binding phrase — the firmware derives the MAC exactly like ExpressLRS.</li>
              <li data-i18n="chain.3">The ESP32 receives ESP-NOW and parses CRSF frames: battery, attitude, GPS and the extended GPS frame (required by the flight identification system).</li>
              <li data-i18n="chain.4">MAVLink packets are built from that data and sent over WiFi to UDP port 14550 (broadcast). Mission Planner, QGroundControl and MAVLinkSkyArc receive them.</li>
              <li data-i18n="chain.5">The ESP32 has a single radio, so the WiFi network and ESP-NOW must share one channel: in STA mode it is the channel of the router/phone access point, in AP mode the channel of the board own mavlink access point.</li>
              <li data-i18n="chain.6">If the channels differ, telemetry will not work: the Status tab shows a warning and the mismatch is logged.</li>
              <li data-i18n="chain.7">MAVLink is only sent when GPS data is available: without it there is nothing to report, so on the ground without a fix the GCS will not see packets.</li>
            </ol>
          </details>
          <div class="button-group">
            <button class="btn-save" onclick="saveWifi()" data-i18n="wifi.save">💾 Save WIFI</button>
            <button class="btn-current" onclick="getCurrentWifi()" data-i18n="wifi.current">🔄 Current WIFI</button>
          </div>
        </div>
      </div>
      <div id="status-tab" class="tab-content">
        <div class="info-box">
          <strong data-i18n="status.title">Device status:</strong>
          <div id="statusTable">Load...</div>
        </div>
        <div class="button-group">
          <button class="btn-current" onclick="getStatus()" data-i18n="status.refresh">🔄 Refresh</button>
          <button class="btn-current" onclick="rebootDevice()" data-i18n="status.reboot">⏻ Reboot</button>
        </div>
      </div>
      <div id="status" class="status"></div>
    </div>
  </div>

  <script>

    // ---------- i18n (English is the fallback language) ----------
    const I18N = {
      en: {
        'tab.status': 'Status',
        'lang.auto': 'Auto',

        'mac.current': 'Current ESP32 MAC-Address:',
        'mac.new': 'New ESP32 MAC-Address:',
        'mac.format': 'Format: AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF',
        'mac.save': '💾 Save MAC',
        'mac.load': '🔄 Current MAC',
        'phrase.label': 'Or set from the ELRS binding phrase:',
        'phrase.hint': 'The MAC is derived from the phrase exactly like ExpressLRS does',
        'phrase.save': '🔑 Save from phrase',

        'wifi.mode': 'WiFi Mode',
        'wifi.modeAp': 'WiFi Access Point',
        'wifi.modeSta': 'WiFi Client (Connect to network)',
        'wifi.password': 'Password',
        'wifi.channel': 'ESP-NOW channel (ELRS backpack: 1, change only if you are sure)',
        'wifi.save': '💾 Save WIFI',
        'wifi.current': '🔄 Current WIFI',

        'chain.summary': 'How the ESP-NOW chain works',
        'chain.1': 'The drone sends CRSF telemetry to the ELRS transmitter (TX), and the TX backpack forwards it over ESP-NOW to the address derived from the binding phrase (UID), on channel 1.',
        'chain.2': 'The ESP32 must use the same MAC as that UID: set it on the MAC tab manually or enter the binding phrase - the firmware derives the MAC exactly like ExpressLRS.',
        'chain.3': 'The ESP32 receives ESP-NOW and parses CRSF frames: battery, attitude, GPS and the extended GPS frame (required by the flight identification system).',
        'chain.4': 'MAVLink packets are built from that data and sent over WiFi to UDP port 14550 (broadcast). Mission Planner, QGroundControl and MAVLinkSkyArc receive them.',
        'chain.5': 'The ESP32 has a single radio, so the WiFi network and ESP-NOW must share one channel: in STA mode it is the channel of the router/phone access point, in AP mode the channel of the board own mavlink access point.',
        'chain.6': 'If the channels differ, telemetry will not work: the Status tab shows a warning and the mismatch is logged.',
        'chain.7': 'MAVLink is only sent when GPS data is available: without it there is nothing to report, so on the ground without a fix the GCS will not see packets.',

        'status.title': 'Device status:',
        'status.refresh': '🔄 Refresh',
        'status.reboot': '⏻ Reboot',
        'status.notResponding': 'Device is not responding',
        'status.channelWarning': 'WiFi is on channel {w} but ESP-NOW needs channel {e}. Telemetry will not work: set the access point to this channel.',

        'row.mode': 'Mode',
        'row.wifiState': 'WiFi state',
        'row.ssid': 'SSID',
        'row.ip': 'IP address',
        'row.wifiChannel': 'WiFi channel',
        'row.espnowChannel': 'ESP-NOW channel',
        'row.signal': 'Signal',
        'row.mac': 'MAC (STA)',
        'row.packets': 'Telemetry packets',
        'row.gps': 'GPS data',
        'row.uptime': 'Uptime',
        'row.heap': 'Free heap',
        'row.reset': 'Reset reason',

        'common.yes': 'yes',
        'common.no': 'no',

        'state.idle': 'idle',
        'state.nossid': 'no ssid',
        'state.scan': 'scan completed',
        'state.connected': 'connected',
        'state.failed': 'connect failed',
        'state.lost': 'connection lost',
        'state.disconnected': 'disconnected',
        'state.unknown': 'unknown',

        'reset.poweron': 'power-on',
        'reset.ext': 'external pin',
        'reset.sw': 'software',
        'reset.panic': 'panic/exception',
        'reset.intwdt': 'interrupt watchdog',
        'reset.taskwdt': 'task watchdog',
        'reset.wdt': 'other watchdog',
        'reset.sleep': 'deep sleep',
        'reset.brownout': 'brownout (power supply!)',
        'reset.sdio': 'SDIO',
        'reset.unknown': 'unknown',

        'msg.errMacLoad': 'Error retrieving MAC address',
        'msg.errWifiLoad': 'Error retrieving WIFI settings',
        'msg.invalidMac': 'Enter a valid MAC address',
        'msg.macSaved': 'MAC saved! Device reboots...',
        'msg.enterPhrase': 'Enter the ELRS binding phrase',
        'msg.phraseSaved': 'MAC {mac} saved! Device reboots...',
        'msg.invalidSsid': 'Enter a valid WiFi SSID',
        'msg.wifiSaved': 'WIFI settings saved! Device reboots...',
        'msg.networkError': 'Network error',
        'msg.error': 'Error: ',
        'msg.confirmReboot': 'Reboot the device?',
        'msg.reboot': 'Device reboots...',
        'secret.show': 'Show',
        'secret.hide': 'Hide',

        'srv.Wrong MAC format': 'Wrong MAC format',
        'srv.MAC is not specified': 'MAC is not specified',
        'srv.Phrase is not specified': 'Phrase is not specified',
        'srv.Phrase is empty': 'Phrase is empty',
        'srv.Phrase is too long': 'Phrase is too long',
        'srv.Invalid JSON': 'Invalid JSON'
      },
      ru: {
        'tab.status': 'Статус',
        'lang.auto': 'Авто',

        'mac.current': 'Текущий MAC-адрес ESP32:',
        'mac.new': 'Новый MAC-адрес ESP32:',
        'mac.format': 'Формат: AA:BB:CC:DD:EE:FF или AA-BB-CC-DD-EE-FF',
        'mac.save': '💾 Сохранить MAC',
        'mac.load': '🔄 Текущий MAC',
        'phrase.label': 'Или задать из биндинг-фразы ELRS:',
        'phrase.hint': 'MAC вычисляется из фразы так же, как это делает ExpressLRS',
        'phrase.save': '🔑 Сохранить из фразы',

        'wifi.mode': 'Режим WiFi',
        'wifi.modeAp': 'Точка доступа WiFi',
        'wifi.modeSta': 'Клиент WiFi (подключение к сети)',
        'wifi.password': 'Пароль',
        'wifi.channel': 'ESP-NOW channel (ELRS backpack: 1, менять только если уверены)',
        'wifi.save': '💾 Сохранить WIFI',
        'wifi.current': '🔄 Текущий WIFI',

        'chain.summary': 'Как работает цепочка ESP-NOW',
        'chain.1': 'Дрон отдаёт CRSF-телеметрию на пульт ELRS (TX), а бэкпак пульта отправляет её по ESP-NOW на адрес, вычисленный из binding phrase (UID), на канале 1.',
        'chain.2': 'ESP32 должна иметь тот же MAC, что этот UID: задайте его на вкладке MAC вручную или введите binding phrase — прошивка посчитает MAC так же, как ExpressLRS.',
        'chain.3': 'ESP32 принимает ESP-NOW и разбирает CRSF-фреймы: батарея, attitude, GPS и расширенный GPS-фрейм (нужен для системы идентификации).',
        'chain.4': 'Из этих данных собираются MAVLink-пакеты и отправляются по Wi-Fi на UDP-порт 14550 (broadcast). Их принимают Mission Planner, QGroundControl и MAVLinkSkyArc.',
        'chain.5': 'Радиомодуль у ESP32 один, поэтому Wi-Fi-сеть и ESP-NOW обязаны быть на одном канале: в режиме STA это канал точки доступа роутера/телефона, в режиме AP — канал собственной точки mavlink.',
        'chain.6': 'Если каналы разошлись, телеметрия не пойдёт: вкладка Status покажет предупреждение, а в лог будет выведено несовпадение.',
        'chain.7': 'MAVLink уходит только при наличии GPS-данных: без них смотреть нечего, поэтому на земле без фикса пакетов в GCS не будет.',

        'status.title': 'Состояние устройства:',
        'status.refresh': '🔄 Обновить',
        'status.reboot': '⏻ Перезагрузить',
        'status.notResponding': 'Устройство не отвечает',
        'status.channelWarning': 'WiFi на канале {w}, а ESP-NOW нужен канал {e}. Телеметрия не пойдёт: переключите точку доступа на этот канал.',

        'row.mode': 'Режим',
        'row.wifiState': 'Состояние WiFi',
        'row.ssid': 'SSID',
        'row.ip': 'IP-адрес',
        'row.wifiChannel': 'Канал WiFi',
        'row.espnowChannel': 'Канал ESP-NOW',
        'row.signal': 'Сигнал',
        'row.mac': 'MAC (STA)',
        'row.packets': 'Пакетов телеметрии',
        'row.gps': 'Данные GPS',
        'row.uptime': 'Время работы',
        'row.heap': 'Свободная память',
        'row.reset': 'Причина перезагрузки',

        'common.yes': 'да',
        'common.no': 'нет',

        'state.idle': 'ожидание',
        'state.nossid': 'сеть не найдена',
        'state.scan': 'сканирование завершено',
        'state.connected': 'подключено',
        'state.failed': 'ошибка подключения',
        'state.lost': 'связь потеряна',
        'state.disconnected': 'отключено',
        'state.unknown': 'неизвестно',

        'reset.poweron': 'включение питания',
        'reset.ext': 'внешний сигнал',
        'reset.sw': 'программная',
        'reset.panic': 'паника/исключение',
        'reset.intwdt': 'сторожевой таймер прерываний',
        'reset.taskwdt': 'сторожевой таймер задач',
        'reset.wdt': 'другой сторожевой таймер',
        'reset.sleep': 'глубокий сон',
        'reset.brownout': 'просадка питания!',
        'reset.sdio': 'SDIO',
        'reset.unknown': 'неизвестно',

        'msg.errMacLoad': 'Ошибка получения MAC-адреса',
        'msg.errWifiLoad': 'Ошибка получения настроек WIFI',
        'msg.invalidMac': 'Введите корректный MAC-адрес',
        'msg.macSaved': 'MAC сохранён! Устройство перезагружается...',
        'msg.enterPhrase': 'Введите биндинг-фразу ELRS',
        'msg.phraseSaved': 'MAC {mac} сохранён! Устройство перезагружается...',
        'msg.invalidSsid': 'Введите SSID сети',
        'msg.wifiSaved': 'Настройки WIFI сохранены! Устройство перезагружается...',
        'msg.networkError': 'Ошибка сети',
        'msg.error': 'Ошибка: ',
        'msg.confirmReboot': 'Перезагрузить устройство?',
        'msg.reboot': 'Перезагрузка устройства...',
        'secret.show': 'Показать',
        'secret.hide': 'Скрыть',

        'srv.Wrong MAC format': 'Неверный формат MAC',
        'srv.MAC is not specified': 'MAC не указан',
        'srv.Phrase is not specified': 'Фраза не указана',
        'srv.Phrase is empty': 'Пустая фраза',
        'srv.Phrase is too long': 'Слишком длинная фраза',
        'srv.Invalid JSON': 'Некорректный JSON'
      }
    };

    let langPref = 'auto';
    let uiLang = 'en';

    function t(key) {
      const dict = I18N[uiLang] || {};
      if (key in dict) return dict[key];
      return (key in I18N.en) ? I18N.en[key] : key;
    }

    function detectLang(pref) {
      if (pref === 'en' || pref === 'ru') return pref;
      const nav = ((navigator.languages && navigator.languages[0]) || navigator.language || 'en').toLowerCase();
      return nav.indexOf('ru') === 0 ? 'ru' : 'en';
    }

    function applyLangPref(pref) {
      langPref = (pref === 'en' || pref === 'ru') ? pref : 'auto';
      uiLang = detectLang(langPref);
      document.documentElement.lang = uiLang;

      document.querySelectorAll('[data-i18n]').forEach(el => {
        el.textContent = t(el.getAttribute('data-i18n'));
      });
      document.querySelectorAll('[data-i18n-title]').forEach(el => {
        el.title = t(el.getAttribute('data-i18n-title'));
      });

      const sel = document.getElementById('lang_select');
      if (sel) sel.value = langPref;

      const statusTab = document.getElementById('status-tab');
      if (statusTab && statusTab.classList.contains('active')) getStatus();
    }

    function setLangPref(pref) {
      applyLangPref(pref);
      fetch('/lang', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ lang: langPref })
      }).catch(() => {});
    }

    function loadLangPref() {
      fetch('/lang')
        .then(response => response.json())
        .then(data => applyLangPref(data.lang))
        .catch(() => applyLangPref('auto'));
    }

    function serverMsg(msg) {
      const key = 'srv.' + msg;
      return (key in I18N.en) ? t(key) : msg;
    }

    function tState(name) {
      const map = {
        'idle': 'state.idle',
        'no ssid': 'state.nossid',
        'scan completed': 'state.scan',
        'connected': 'state.connected',
        'connect failed': 'state.failed',
        'connection lost': 'state.lost',
        'disconnected': 'state.disconnected'
      };
      return (name in map) ? t(map[name]) : t('state.unknown');
    }

    function tReset(name) {
      const map = {
        'power-on': 'reset.poweron',
        'external pin': 'reset.ext',
        'software': 'reset.sw',
        'panic/exception': 'reset.panic',
        'interrupt watchdog': 'reset.intwdt',
        'task watchdog': 'reset.taskwdt',
        'other watchdog': 'reset.wdt',
        'deep sleep': 'reset.sleep',
        'brownout (power supply!)': 'reset.brownout',
        'SDIO': 'reset.sdio'
      };
      return (name in map) ? t(map[name]) : t('reset.unknown');
    }

    // Disable ssid, password change in AP mode
    let externalSSID = "";
    let externalPassword = "";
    function wifiModeChange(wifi_mode) {
      const ssid_elem = document.getElementById('wifi_ssid');
      const password_elem = document.getElementById('wifi_password');

      if (wifi_mode === "ap") {
        externalSSID = ssid_elem.value;
        ssid_elem.value = "mavlink";
        ssid_elem.readOnly = true;
        ssid_elem.style.cursor = 'not-allowed';

        externalPassword = password_elem.value;
        password_elem.value = "12345678";
        password_elem.readOnly = true;
        password_elem.style.cursor = 'not-allowed';
      } else {
        ssid_elem.value = externalSSID;
        ssid_elem.readOnly = false;
        ssid_elem.style.cursor = 'text';

        password_elem.value = externalPassword;
        password_elem.readOnly = false;
        password_elem.style.cursor = 'text';
      }
    }

    // Show/hide a secret field (password, binding phrase)
    function toggleVisibility(fieldId, buttonId) {
      const field = document.getElementById(fieldId);
      const button = document.getElementById(buttonId);
      if (field.type === 'password') {
        field.type = 'text';
        button.textContent = '🙈';
        button.title = t('secret.hide');
      } else {
        field.type = 'password';
        button.textContent = '👁';
        button.title = t('secret.show');
      }
    }

    // Get current MAC by page loading
    window.onload = function() {
      prepareTabs();
      loadLangPref();
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
          showStatus(t('msg.errMacLoad'), 'error');
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

            channel_elem.value = data.wifi_channel;
            channel_elem.readOnly = false;
            channel_elem.style.cursor = 'text';
          } else {
              ssid_elem.value = "mavlink";
              ssid_elem.readOnly = true;
              ssid_elem.style.cursor = 'not-allowed';

              password_elem.value = "12345678";
              password_elem.readOnly = true;
              password_elem.style.cursor = 'not-allowed';

              channel_elem.value = data.wifi_channel;
              channel_elem.readOnly = false;
              channel_elem.style.cursor = 'text';
          }
        })
        .catch(error => {
          showStatus(t('msg.errWifiLoad'), 'error');
        });
    }

    // Save new MAC
    function saveMac() {
      const macInput = document.getElementById('mac').value.trim();

      // Validate MAC
      if (!macInput || !isValidMac(macInput)) {
        showStatus(t('msg.invalidMac'), 'error');
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
          showStatus(t('msg.macSaved'), 'success');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          showStatus(t('msg.error') + serverMsg(data), 'error');
        }
      })
      .catch(error => {
        showStatus(t('msg.networkError'), 'error');
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
        showStatus(t('msg.invalidSsid'), 'error');
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
          showStatus(t('msg.wifiSaved'), 'success');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          showStatus(t('msg.error') + serverMsg(data), 'error');
        }
      })
      .catch(error => {
        showStatus(t('msg.networkError'), 'error');
      });
    }

    // Save MAC calculated from the ELRS binding phrase
    function savePhrase() {
      const phrase = document.getElementById('phrase').value.trim();
      if (!phrase) {
        showStatus(t('msg.enterPhrase'), 'error');
        return;
      }

      fetch('/save_phrase', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'phrase=' + encodeURIComponent(phrase)
      })
      .then(response => response.text())
      .then(data => {
        if (data.startsWith('OK')) {
          document.getElementById('mac').value = data.slice(3);
          showStatus(t('msg.phraseSaved').replace('{mac}', data.slice(3)), 'success');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          showStatus(t('msg.error') + serverMsg(data), 'error');
        }
      })
      .catch(error => {
        showStatus(t('msg.networkError'), 'error');
      });
    }

    // Live device status
    let statusTimer = null;
    function getStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const rows = [
            [t('row.mode'), data.mode],
            [t('row.wifiState'), tState(data.wifi_state)],
            [t('row.ssid'), data.ssid],
            [t('row.ip'), data.ip],
            [t('row.wifiChannel'), data.wifi_channel],
            [t('row.espnowChannel'), data.espnow_channel],
            [t('row.signal'), data.wifi_state === 'connected' ? data.rssi + ' dBm' : '-'],
            [t('row.mac'), data.mac],
            [t('row.packets'), data.packets],
            [t('row.gps'), data.gps ? t('common.yes') : t('common.no')],
            [t('row.uptime'), data.uptime + ' s'],
            [t('row.heap'), data.heap + ' B'],
            [t('row.reset'), tReset(data.reset_reason)]
          ];

          let html = '';
          if (!data.channel_ok) {
            html += '<div class="status error" style="display:block;margin-bottom:10px">' +
                    t('status.channelWarning')
                      .replace('{w}', data.wifi_channel)
                      .replace('{e}', data.espnow_channel) +
                    '</div>';
          }
          html += '<table style="width:100%;border-collapse:collapse">';
          rows.forEach(row => {
            html += '<tr><td style="padding:3px 8px;color:#666">' + row[0] +
                    '</td><td style="padding:3px 8px;font-family:monospace">' + row[1] + '</td></tr>';
          });
          html += '</table>';
          document.getElementById('statusTable').innerHTML = html;
        })
        .catch(error => {
          document.getElementById('statusTable').textContent = t('status.notResponding');
        });
    }

    function rebootDevice() {
      if (!confirm(t('msg.confirmReboot'))) return;
      fetch('/reboot', { method: 'POST' })
        .then(() => showStatus(t('msg.reboot'), 'success'));
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

        function prepareTabs() {
      document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
          switchTab(btn.dataset.tab);
        });
      });
    }

    function switchTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(tab => {
        tab.classList.remove('active');
      });
      document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.classList.remove('active');
      });

      document.getElementById(tabId).classList.add('active');
      document.querySelector(`[data-tab="${tabId}"]`).classList.add('active');

      if (statusTimer) {
        clearInterval(statusTimer);
        statusTimer = null;
      }
      if (tabId === 'status-tab') {
        getStatus();
        statusTimer = setInterval(getStatus, 2000);
      }
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
  server.send(200, "text/html; charset=utf-8", htmlPage);
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
    Serial.printf("Password: %u chars\n", (unsigned)strlen(password));
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
    int channel = doc["wifi_channel"];
    if (channel < 1) channel = 1;
    if (channel > 13) channel = 13;
    config.wifiChannel = (uint8_t)channel;
    Serial.printf("Channel: %d\n", config.wifiChannel);
  }

  saveWifiToStorage();

  server.send(200, "text/plain", "OK");
  Serial.println("New WIFI settings are saved");
  safeReboot();
}

// The UID (6 bytes) is the first 6 bytes of the MD5 of the ELRS bind key
// and phrase, exactly as the ExpressLRS firmware calculates it.
static void macFromBindingPhrase(const char* phrase, uint8_t* mac) {
  static const char BIND_KEY[] = "-DMY_BINDING_PHRASE=\"";
  uint8_t digest[16];
  mbedtls_md5_context ctx;

  mbedtls_md5_init(&ctx);
  mbedtls_md5_update_ret(&ctx, (const unsigned char*)BIND_KEY, strlen(BIND_KEY));
  mbedtls_md5_update_ret(&ctx, (const unsigned char*)phrase, strlen(phrase));
  mbedtls_md5_update_ret(&ctx, (const unsigned char*)"\"", 1);
  mbedtls_md5_finish_ret(&ctx, digest);
  mbedtls_md5_free(&ctx);

  memcpy(mac, digest, 6);
}

// Handler for storing MAC calculated from the ELRS binding phrase
void handleSavePhrase() {
  if (!server.hasArg("phrase")) {
    server.send(400, "text/plain", "Phrase is not specified");
    return;
  }

  String phrase = server.arg("phrase");
  phrase.trim();

  if (phrase.length() == 0) {
    server.send(400, "text/plain", "Phrase is empty");
    return;
  }
  if (phrase.length() > 63) {
    server.send(400, "text/plain", "Phrase is too long");
    return;
  }

  macFromBindingPhrase(phrase.c_str(), config.customMAC);
  saveMacToStorage();

  String mac = macToString(config.customMAC);
  Serial.println("MAC from binding phrase: " + mac);

  server.send(200, "text/plain", "OK " + mac);
  safeReboot();
}

static const char* wifiStateName(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:      return "idle";
    case WL_NO_SSID_AVAIL:    return "no ssid";
    case WL_SCAN_COMPLETED:   return "scan completed";
    case WL_CONNECTED:        return "connected";
    case WL_CONNECT_FAILED:   return "connect failed";
    case WL_CONNECTION_LOST:  return "connection lost";
    case WL_DISCONNECTED:     return "disconnected";
    default:                  return "unknown";
  }
}

// Handler for live device status (JSON)
void handleStatus() {
  JsonDocument doc;

  bool staMode = config.wifiMode == STA_WIFI_MODE;
  bool connected = WiFi.isConnected();
  bool channelOk = !(staMode && connected && WiFi.channel() != config.wifiChannel);

  doc["mode"] = staMode ? "STA" : "AP";
  doc["wifi_state"] = wifiStateName(WiFi.status());
  doc["ssid"] = staMode ? config.wifiSSID : String(apSSID);
  doc["ip"] = (staMode && connected) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  doc["wifi_channel"] = connected ? WiFi.channel() : config.wifiChannel;
  doc["espnow_channel"] = config.wifiChannel;
  doc["channel_ok"] = channelOk;
  if (!channelOk) {
    doc["warning"] = "WiFi is on channel " + String(WiFi.channel()) +
                     " but ESP-NOW needs channel " + String(config.wifiChannel) +
                     ". Telemetry will not work: set the access point to this channel.";
  }
  doc["rssi"] = connected ? WiFi.RSSI() : 0;
  doc["mac"] = macToString(config.customMAC);
  doc["packets"] = telemetriesData.statistic.packetCount;
  doc["gps"] = telemetriesData.gps.enabled;
  doc["uptime"] = (uint32_t)(millis() / 1000);
  doc["heap"] = ESP.getFreeHeap();
  doc["reset_reason"] = resetReasonName(esp_reset_reason());

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

static const char* uiLangName() {
  switch (config.uiLang) {
    case UI_LANG_EN: return "en";
    case UI_LANG_RU: return "ru";
    default:         return "auto";
  }
}

// Handler to obtain the UI language preference
void handleGetLang() {
  String response = String("{\"lang\":\"") + uiLangName() + "\"}";
  server.send(200, "application/json", response);
}

// Handler to store the UI language preference (no reboot needed)
void handleSaveLang() {
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char* lang = doc["lang"] | "auto";
  if (strcmp(lang, "en") == 0) {
    config.uiLang = UI_LANG_EN;
  } else if (strcmp(lang, "ru") == 0) {
    config.uiLang = UI_LANG_RU;
  } else {
    config.uiLang = UI_LANG_AUTO;
  }

  saveUiLangToStorage();
  Serial.printf("UI language: %s\n", uiLangName());
  server.send(200, "text/plain", "OK");
}

// Handler for a plain reboot (keeps all settings)
void handleReboot() {
  server.send(200, "text/plain", "OK");
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
  info += "<p><strong>WiFi state:</strong> " + String(wifiStateName(WiFi.status())) + "</p>";
  info += "<p><strong>Channel:</strong> " + String(WiFi.channel()) + " (ESP-NOW: " + String(config.wifiChannel) + ")</p>";
  info += "<p><strong>Telemetry packets:</strong> " + String(telemetriesData.statistic.packetCount) + "</p>";
  info += "<p><strong>Reset reason:</strong> " + String(resetReasonName(esp_reset_reason())) + "</p>";
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
  // Loading saved UI language
  loadUiLangFromStorage();

    // Setting up a web server
  server.on("/", handleRoot);
  server.on("/current_mac", handleCurrentMac);
  server.on("/current_wifi", handleCurrentWifi);
  server.on("/save_mac", HTTP_POST, handleSaveMac);
  server.on("/save_wifi", HTTP_POST, handleSaveWifi);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/reboot", HTTP_POST, handleReboot);
  server.on("/info", handleInfo);
  server.on("/save_phrase", HTTP_POST, handleSavePhrase);
  server.on("/status", handleStatus);
  server.on("/lang", HTTP_GET, handleGetLang);
  server.on("/lang", HTTP_POST, handleSaveLang);

  server.begin();
  Serial.println("------------------------------------------------");
  Serial.println("HTTP server is running");
}

void webServerRun() {
    server.handleClient();
}
