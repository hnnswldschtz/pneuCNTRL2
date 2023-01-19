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
#ifndef CHANNELS_H
#define CHANNELS_H
#include "PneuCNTRL.h"
#include <ADS1115_lite.h>
class ValveChannel{

  public:
    //ValveChannel(int, int, Adafruit_ADS1115 *, int, int, int );
    ValveChannel(int, int, int, int );
    int begin(int, int, int, long);
    void setInertia(int, int);
    void setGuiMappingRange(int ,int);
    #if ADS_L
        void setAdc(ADS1115_lite *, int );
        ADS1115_lite * adc;
    #else
        void setAdc(Adafruit_ADS1115 *, int );
        Adafruit_ADS1115 * adc;
    #endif



    byte operate_manual();
    byte trigger(int,int);


    int get_Pressure();
    int get_MappedPressure();
    int mapToRawP(int p);
    int calcRefillOffset(int);
    int calcInflationInertia(int);
    bool get_state();
    int get_Poti();
    int get_MappedPoti();
    bool get_button();
    bool get_buttonNow(unsigned long d = 30);





  protected:

    //Adafruit_ADS1115 * adc;

    int minPressure;
    int maxPressure;
    int safetyStop;

    int infValvePin;
    int defValvePin;
    byte adc_ch;
    int potPin;
    int buttonPin;
    int pressure;
    int mappedPressure;
    int old_p_set;


    int inflationInertia;
    int deflationInertia;
    int potiValMapped;
    int potiVal;
    int GUI_pMapMin;
    int GUI_pMapMax;

    unsigned long time_out;
    long lastFill;
    bool inFlate;
    bool deFlate;
    unsigned long inFlate_time;
    unsigned long deFlate_time;
    long lastDebounceTime;
    bool lastFlickerState;
    bool buttonState;
    bool lastButtonState;
    bool buttonPressed;
    unsigned long starttime;
    unsigned long endtime ;

    int readPressure();
    void mapPressureToGui();
    void emergency_watchdog(int);
    void inflate();
    void deflate();
    void stop();
    void readButton(unsigned long);
    int readPotentiometer();

    float filter(float, float, float);
    int value_mapper (int, int, int);


  };



class ProportionalChannel{

  public:
    ProportionalChannel();
    int begin(int,int,int,int = 1 );
    //void setAdc(Adafruit_ADS1115 *, int );
    #if ADS_L
        void setAdc(ADS1115_lite *, int );
        ADS1115_lite * adc;
    #else
        void setAdc(Adafruit_ADS1115 *, int );
        Adafruit_ADS1115 * adc;
    #endif

    void setDac(Adafruit_MCP4728 *, MCP4728_channel_t  );
    void setGuiMappingRange(int , int );

    void goToPressure(int);
    void stop();
    int get_MappedPressure();
    void operate();


  private:
    int mapToRawVolts(int);
    int mapInputPressure();
    int readRawPressure();

    //Adafruit_ADS1115 * adc;

    Adafruit_MCP4728 * dac;
    MCP4728_channel_t  dac_ch;
    int adc_ch;
    int pressure;
    int mappedPressure;
    int valvePressureRange;
    int minPressure;
    int maxPressure;
    int safetyStop;
    int maxOutputVoltage;

    float DAC_MAX_V;
    float ADC_MAX_V;
    float DAC_CORRECTON_FAKTOR;
    float PV_V_CORRECTION_FAKTOR;
    int GUI_pMapMin;
    int GUI_pMapMax;
    unsigned long starttime;
    unsigned long endtime ;


};
#endif
