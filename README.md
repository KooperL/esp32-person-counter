# esp32-person-counter

This project scans for nearby WiFi and Bluetooth Low Energy devices using an ESP32. It reports the number of unique MAC addresses observed during a fixed time window to estimate person presence.

![Demo](https://github.com/KooperL/esp32-person-counter/blob/main/demo.png)

## Description

The firmware periodically:
- Scans for WiFi networks and records BSSIDs
- Scans for BLE advertisements and records device addresses
- Outputs counts over the serial interface (115200 baud)

## Serial output:

Outcome is reported from `void reportDevices`, currently configured to print: `WiFi:<count>,Bluetooth:<count>`. 
The counts are derived from the number of unique MAC addresses stored in arrays during the reporting interval. Future extensions could forward this data to MQTT topics (for example, for Home Assistant integration) or store aggregated results for later processing.
