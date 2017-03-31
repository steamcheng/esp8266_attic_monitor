# Attic monitor: 20 Mar 2017

esp8266_attic_monitor

This controls an ESP8266 in the attic which monitors temperature humidity and attic vent fan status and publishes data to the network broker via MQTT.

The device relies on having a Wemos Mini D1 ESP8266 as the brain, receiving temp and humidity information from a DHT22 sensor.  Fan status is sensed using a current transformer on the power lead to the fan, which is connected to the ADC pin on the 8266.  The temperature, humidity and fan status is published out to the MQTT broker on my openHAB Raspberry Pi, and is further stored in a mysql database on another machine for further analysis.  The device also publishes various status reports to indicate node status, status of the DHT sensor and running current for the fan.

The original version of this tool only monitored temp and humidity, but I recently had an attic vent fan installed (partially due to the temps reported by this device) and I wanted to know when it was running.  The new schematic is provided as "Attic monitor v3_schem.pdf" in the schematics folder.  The code relies on the EmonLib.h library available from Open Energy Monitor at:

https://learn.openenergymonitor.org/electricity-monitoring/ctac/how-to-build-an-arduino-energy-monitor

While I could have either rectified the signal and/or handled it in my own code as a simple current detector, I elected to use the library instead.  I do not really need the calculations for current for this application, but decided to include them in their crudest form anyway, in case I want to expand capability later.  No attempt was made to calibrate the calculation, so the current (Irms) reported by the device is not accurate.

I also added a transistor to control power to the DHT sensor, as it sometimes hangs up and does not record data.  The only way to reset it was to reset power to the node (by manually unplugging it and then plugging it back in), so I added a routine in the code to reset power to the DHT sensor after three cycles of 'NaN' reported.  That way the node will continue to operate on its own and not hang up for a week while I am out of town.  I'll probably improve this to make it three consecutive cycles instead of a simple count to three.  Somehow didn't think of that when I was doing it.


