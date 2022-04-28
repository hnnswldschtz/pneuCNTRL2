# pneuCNTRL2 Firmware
4 channel pressure controlled inlation/deflation system for the _Fludic Data_ Haptics experiment

Setup:

- Arduino Mega
- 12 ch tip120 setup
- ads1115 adc
- 4x festo 571482 pressure sensors
- 4x smc 5/3 electrc valves
- LCD i2c display

libraries used:
ADS1115_lite: https://github.com/terryjmyers/ADS1115-Lite
Adafruit_MCP4728: https://github.com/adafruit/Adafruit_MCP4728
Adafruit_NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
LiquidCrystal_I2C: https://github.com/johnrickman/LiquidCrystal_I2C -> this gives a massive delay when used! maybe avoid!!
Bauhaus-University Weimar \
hnns 2021
