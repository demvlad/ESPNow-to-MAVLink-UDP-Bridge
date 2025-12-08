This is a ESP32 program. 

It reads ESPNow messages from ELRS backpack (https://www.expresslrs.org/software/backpack-telemetry/), transform CRSF telemetries frames to MAVLink packets and sends MAVLink data by UDP on port 14550.

Build programm and load it to your ESP32 device.

Run ESP32 device.

The programm creates WIFI access point with name "mavlink" and password "12345678".

Connect to WIFI access point.

Go to setup web page: http://192.168.4.1/

Set ESP32 MAC address at setup page like yours ELRS TX UUID, what you can look at ELRS TX web page. (It is generated from yours ELRS binding phrase). 

Turn on ELRS transmitter with backpacs ESPNow telemetry enabled. 

Run this programm on ESP32 device.

Turn on drone with CRSF telemetry enabled.

Connect to 'mavlink' WIFI access point, what is created ESP32 device. The password is 12345678.

Receive MAVLink telemetry packets by 14550 UDP port by using MissionPlanner or other GCS program.
