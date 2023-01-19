/*
pneuCNTRlbox Firmware.
copyright hnnz 2023

GPL-3.0-or-later

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.


*/
#include "PneuCNTRL.h"
#include "Channels.h"


ProportionalChannel::ProportionalChannel()
{
    #define TIMEOUT 10000UL
    DAC_MAX_V = 4096;
    ADC_MAX_V = 32767;
    DAC_CORRECTON_FAKTOR = 1.22;
    //----default global gui mapping values. std: 0-100
    GUI_pMapMin = 0;
    GUI_pMapMax = 100;
    maxOutputVoltage = 0;


}

int ProportionalChannel::begin(int min_pressure, int max_pressure, int safety_stop, int _maxPressureInBar )
{
    // adc->setGain(GAIN_ONE);
    // adc->begin();
    minPressure = min_pressure;
    maxPressure = max_pressure/_maxPressureInBar;
    safetyStop = safety_stop/_maxPressureInBar;
    //---------------Calculates inital maximum DAC output Voltage for mapping
    maxOutputVoltage = int(((float(max_pressure)/ADC_MAX_V)*DAC_MAX_V)/DAC_CORRECTON_FAKTOR);
    return 0;
}

// void ProportionalChannel::setAdc(Adafruit_ADS1115 * _adc, int _adc_ch)


#if ADS_L
  void ProportionalChannel::setAdc(ADS1115_lite *_adc, int _adc_ch)
  {
    adc = _adc;
    adc_ch = _adc_ch;
  }
#else
  void ProportionalChannel::setAdc(Adafruit_ADS1115 *_adc, int _adc_ch)
  {
    adc = _adc;
    adc_ch = _adc_ch;
  }
#endif

void ProportionalChannel::setDac(Adafruit_MCP4728 *_dac, MCP4728_channel_t _dac_ch)
{
  dac = _dac;
  dac_ch = _dac_ch;
}


// int ProportionalChannel::readRawPressure(){
//     pressure = adc->readADC_SingleEnded(adc_ch);
//     return pressure;
// }

int ProportionalChannel::readRawPressure(){
  #if ADS_L
      starttime = micros();
      adc->setMux(adc_ch); //Set single ended mode between AIN0 and GND
      adc->triggerConversion(); //Start a conversion.  This immediatly returns
      pressure =  adc->getConversion(); //This polls the ADS1115 and wait for conversion to finish, THEN returns the value
      endtime = micros();
      Serial.print("propcompl: ");
      Serial.print(pressure); Serial.print(",  ");
      Serial.print(endtime - starttime);
      Serial.println("us");
  #else
      pressure = adc->readADC_SingleEnded(adc_ch);
  #endif

  return pressure;
}

int ProportionalChannel::mapInputPressure(){ // maps pressuref from internal adc raw value to GUI land pressure range
    mappedPressure = int(map(readRawPressure(),minPressure,maxPressure,GUI_pMapMin,GUI_pMapMax));
    return mappedPressure;
}

int ProportionalChannel::get_MappedPressure(){ // maps pressuref from internal adc raw value to GUI land pressure range
    return mappedPressure;
}

void ProportionalChannel::operate(){ // maps pressuref from internal adc raw value to GUI land pressure range
    mapInputPressure();
}


void ProportionalChannel::goToPressure(int p){
  // Serial.print("maxOutputVoltage: ");
  // Serial.println(maxOutputVoltage);
  // Serial.print("gotoPressure received: ");
  // Serial.println(p);
  // Serial.print("and forwards: ");
  // Serial.println(mapToRawVolts(p));
  dac->setChannelValue(dac_ch, mapToRawVolts(p));

}

void ProportionalChannel::stop(){
  dac->setChannelValue(dac_ch, 0);
}

int ProportionalChannel::mapToRawVolts(int p){ // maps pressure setting from GUI land to internal adc raw value
  return int(map(p, GUI_pMapMin, GUI_pMapMax, 0, maxOutputVoltage));
}

void ProportionalChannel::setGuiMappingRange(int _mapMin, int _mapMax){
  GUI_pMapMin = _mapMin;
  GUI_pMapMax = _mapMax;
}
