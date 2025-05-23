# Temperature station

this temperature station is created as aditional feature for the main SAPS developing station.

The code (inside **Code**) is created for the ESP32 DevKit C module.

It utilizes the DS18B20 digital temperature submersable sensor and it need to be soldered directly to the ESP32 (voltage to 3V3 pin, Ground to GND and Data to GPIO4)

!!**IMPORTANT**!! - Between the 3V3 and GPIO4 need to be a pull-up 4.7k Ohm resistor for the sensor to work properly