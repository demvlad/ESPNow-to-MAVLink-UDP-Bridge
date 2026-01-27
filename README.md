This ESP32 program reads ESPNow messages from ELRS backpack (https://www.expresslrs.org/software/backpack-telemetry/), transform CRSF telemetries frames to MAVLink packets and sends MAVLink data by UDP on port 14550.


**Setup:**

Upload program into your ESP32 device.

The programm creates WIFI access point with name "mavlink" and password "12345678".

Connect to WIFI access point.

Go to setup web page: http://192.168.4.1/
![MAC_setup](https://github.com/user-attachments/assets/48574790-c034-4df2-99df-ace050954b48)

Set ESP32 MAC address at setup page like yours ELRS TX UUID, what you can look at ELRS TX web page. (It is generated from yours ELRS binding phrase). 
![elrs_page](https://github.com/user-attachments/assets/e987d86e-125b-4f75-9def-61cf3009950c)

Go to the WiFi setup tab.
Select WiFi mode and set login and password.

**WiFi settings for AP mode**

![wifi_ap_setup](https://github.com/user-attachments/assets/fe252ba4-fc74-469e-b228-fbf976e508c5)

**WiFi settings for STA mode**

![wifi_sta_setup](https://github.com/user-attachments/assets/3be1a237-3b43-41b4-a792-ec812def4189)


**Using with ELRS TX CRSF telemetry:**

Turn on ELRS transmitter with backpacs ESPNow telemetry enabled. 

Turn on drone with CRSF telemetry enabled.

Connect to 'mavlink' WIFI access point, what is created ESP32 device for Access point mode. The password is 12345678.

In STA mode device connects to external WiFi point by using login and password configuration. In this mode It is possible to use the WiFi channel 1 only.

Receive MAVLink telemetry packets on 14550 UDP port by using MissionPlanner or other GCS program.
