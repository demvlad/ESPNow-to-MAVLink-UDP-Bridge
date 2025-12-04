#ifndef MAVLINK_H
#define MAVLINK_H
#include <Arduino.h>
bool buildMAVLinkDataStream(TelemetryData_t* telemetry, uint8_t** ptrMavlinkData, uint16_t* ptrDataLength);
#endif