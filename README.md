This ESP32 program reads ESPNow messages from ELRS backpack (https://www.expresslrs.org/software/backpack-telemetry/), transform CRSF telemetries frames to MAVLink packets and sends MAVLink data by UDP on port 14550.


**Setup:**

Upload program into your ESP32 device.

The programm creates WIFI access point with name "mavlink" and password "12345678".

Connect to WIFI access point.

Go to setup web page: http://192.168.4.1/
![setup-web-page](https://github.com/user-attachments/assets/cbd0d67f-2b58-4b16-b02a-48d874ea20a6)


Set ESP32 MAC address at setup page like yours ELRS TX UUID, what you can look at ELRS TX web page. (It is generated from yours ELRS binding phrase). 
![elrs_page](https://github.com/user-attachments/assets/e987d86e-125b-4f75-9def-61cf3009950c)

**Using with ELRS TX CRSF telemetry:**

Turn on ELRS transmitter with backpacs ESPNow telemetry enabled. 

Turn on drone with CRSF telemetry enabled.

Connect to 'mavlink' WIFI access point, what is created ESP32 device. The password is 12345678.

Receive MAVLink telemetry packets on 14550 UDP port by using MissionPlanner or other GCS program.
