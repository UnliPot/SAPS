# Code for power&control board

Please be aware that this code is specificaly made for custom made pcb power&constol board used inside the SAPS developing station.

The main programing environment is Arduino IDE in version 2.3.5 and higher.

## Board settings

The power&control board uses the ESP32-S3-WROOM-1U-N8 so select the board inside Arduino IDE accordingly. The recommended settings are:

- Board: ESP32S3 Dev Module
- USB CDC On Boot: "Disabled"
- CPU Frequency: "240 MHz (WiFi)"
- Code Debug Level: "None"
- USB DFU On Boot: "Disabled"
- Erase All Flash Before Sketch Upload: "Disabled"
- Events Run On: "Core 1"
- Flash Mode: "QIO 80 MHz"
- Flash Size: "8MB (64Mb)"
- JTAG Adapter: "Disabled"
- Arduino Runs On: "Core 1"
- USB Firmware MSC On Boot: "Disabled"
- Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
- PSRAM: "Disabled"
- Upload Mode: "UART0/Hardware CDC"
- Upload Speed: "921600"
- USB Mode: "Hardware CDC and JTAG"
- Zigbee Mode: "Disabled"