# pneuCNTRL2 Firmware
8 channel pressure controlled inlation/deflation system for the inflatable haptics experiments

Setup:

- custom PCB released as openhardware under cern-ohl-w v2 license
- based on teensy3.2
- ads1115 for ADC
- MCP4728 for DAC
- 4x festo 571482 pressure sensors
- 4x smc 5/3 electric valves
- 4x Festo proportional piezo valves (VEAB-L-26-D7-Q4-V1-1R1)
- LCD i2c display

<img width="652" alt="pneuCNTRL" src="https://user-images.githubusercontent.com/18725938/213315680-d9077383-0f79-4ea7-a215-53ef868a316f.png">

libraries used:
ADS1115_lite: https://github.com/terryjmyers/ADS1115-Lite
Adafruit_MCP4728: https://github.com/adafruit/Adafruit_MCP4728
Adafruit_NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
LiquidCrystal_I2C: https://github.com/johnrickman/LiquidCrystal_I2C -> this gives a massive delay when used! maybe avoid!!
Adafruit_ADS1015: https://github.com/adafruit/Adafruit_ADS1X15

Bauhaus-University Weimar \
hnns 2022
