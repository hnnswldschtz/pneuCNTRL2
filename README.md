# pneuCNTRL2 Firmware
8 channel pressure controlled inlation/deflation system for the inflatable haptics experiments

Setup:

- custom PCB released as openhardware under cern-ohl-w v2 license 
- based on teensy3.2
- ads1115 for adc
- 
- 4x festo 571482 pressure sensors
- 4x smc 5/3 electrc valves
- 4x Festo proportional piezo valves (VEAB-L-26-D7-Q4-V1-1R1)
- LCD i2c display

libraries used:
ADS1115_lite: https://github.com/terryjmyers/ADS1115-Lite
Adafruit_MCP4728: https://github.com/adafruit/Adafruit_MCP4728
Adafruit_NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
LiquidCrystal_I2C: https://github.com/johnrickman/LiquidCrystal_I2C -> this gives a massive delay when used! maybe avoid!!
Bauhaus-University Weimar \
hnns 2022
