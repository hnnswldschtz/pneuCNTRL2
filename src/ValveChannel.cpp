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

//public methods

//ValveChannel::ValveChannel(int inf_pin, int def_pin, Adafruit_ADS1115 * _adc, int _adc_ch, int pot_pin, int button_pin){
ValveChannel::ValveChannel(int inf_pin, int def_pin, int pot_pin, int button_pin){

    #define DEBOUNCE_DELAY 30UL


    //Serial.begin(9600);
    infValvePin = inf_pin;
    defValvePin = def_pin;
    potPin = pot_pin;
    buttonPin = button_pin;

    //----default global gui mapping values. std: 0-100
    GUI_pMapMin = 0;
    GUI_pMapMax = 100;


}

int ValveChannel::begin(int min_pressure, int max_pressure, int safety_stop, long _timeout){
    pinMode(infValvePin,OUTPUT);
    pinMode(defValvePin,OUTPUT);
    digitalWrite(infValvePin,LOW);
    digitalWrite(defValvePin,LOW);
    pinMode(buttonPin,INPUT_PULLUP);


    time_out = _timeout;
    minPressure = min_pressure;
    maxPressure = max_pressure;
    safetyStop = safety_stop;

    inflate();
    delay(100);
    deflate();
    delay(100);
    stop();

    //Serial.print("Getting single-ended readings from AIN ");
    //Serial.println (adc_ch);

    return 0;
}

#if ADS_L
  void ValveChannel::setAdc(ADS1115_lite *_adc, int _adc_ch)
  {
    adc = _adc;
    adc_ch = _adc_ch;
  }
#else
  void ValveChannel::setAdc(Adafruit_ADS1115 *_adc, int _adc_ch)
  {
    adc = _adc;
    adc_ch = _adc_ch;
  }
#endif

void ValveChannel::setInertia(int infIn = 1000, int defIn = 500){
  // change hysteresis boundarys according to inflation and defltion behaviour
  inflationInertia = infIn;
  deflationInertia = defIn;
}

void ValveChannel::setGuiMappingRange(int _mapMin, int _mapMax){
  GUI_pMapMin = _mapMin;
  GUI_pMapMax = _mapMax;
}

byte ValveChannel::operate_manual(){
    readPressure();
    emergency_watchdog(safetyStop);
    mapPressureToGui();

    readPotentiometer();
    readButton(DEBOUNCE_DELAY);


    if (millis()-lastFill>=0){

        lastFill=millis();
        if (pressure < potiValMapped && buttonPressed) {
            inflate();
            inFlate = true;
            inFlate_time = millis();
        }
        else if (pressure > (potiValMapped-inflationInertia) && inFlate){
            stop();
            inFlate = false;
        }
        else if (pressure > potiValMapped && buttonPressed) {
            deflate();
            deFlate = true;
            deFlate_time = millis();
        }
        else if (pressure < (potiValMapped+deflationInertia) && deFlate){
            stop();
            deFlate = false;
        }
    }
  if (inFlate&&(millis() - inFlate_time > time_out)) stop();
  if (deFlate&&(millis() - deFlate_time > time_out)) stop();

  return 0;
}

byte ValveChannel::trigger(int p_set, int trig){

    readPressure();
    emergency_watchdog(safetyStop);
    mapPressureToGui();

    int p_set_mapped_to_raw_p = mapToRawP(p_set); // we work internally with the raw input value range from adc

    if (millis()-lastFill>=0){

        lastFill=millis();
        // -------start inflating
        if (old_p_set!=p_set_mapped_to_raw_p&&(pressure < p_set_mapped_to_raw_p && trig == 1)) {
            deFlate = false;
            stop();
            // Serial.print("in, ");
            // Serial.print("p_set: ");
            // Serial.print(p_set);
            // Serial.print(", p_set: ");
            // Serial.print(p_set_mapped_to_raw_p);
            // Serial.print(", pressure: ");
            // Serial.println(pressure);
            inflate();
            inFlate = true;
            inFlate_time = millis();
            old_p_set=p_set_mapped_to_raw_p;
        }
        // ------ stop inflating
        else if (pressure > (p_set_mapped_to_raw_p-calcInflationInertia(pressure)) && inFlate){

            // Serial.print("at ");
            // Serial.print(pressure);
            // Serial.print(" inflation ");
            stop();

            inFlate = false;
        } // -------start deflating
        else if (old_p_set!=p_set_mapped_to_raw_p&&(pressure > p_set_mapped_to_raw_p && trig == 1)) {
            inFlate = false;
            stop();
            // Serial.print("out, ");
            // Serial.print("p_set: ");
            // Serial.print(p_set);
            // Serial.print(", p_set: ");
            // Serial.print(p_set_mapped_to_raw_p);
            // Serial.print(", pressure: ");
            // Serial.println(pressure);
            deflate();
            deFlate = true;
            deFlate_time = millis();
            old_p_set=p_set_mapped_to_raw_p;
        }

        //------ stop deflating
        else if (pressure < (p_set_mapped_to_raw_p-deflationInertia) && deFlate ){

          // Serial.print("at ");
          // Serial.print(pressure);
          // Serial.print(" deflation ");
          if (p_set_mapped_to_raw_p != 0) stop(); // dont close valve when val is set to zero
            deFlate = false;
        }
    }
    //timeout if inflation never reaches top
    if (inFlate && (millis() - inFlate_time > time_out)) {
        Serial.println ("timeout_inF ");
        inFlate=false;
        stop();
    }
    //timeout if inflation never reaches bottom
    if (deFlate&&(millis() - deFlate_time>time_out)) {
        Serial.println ("timeout_deF ");
        deFlate=false;
        stop();
    }

    /*REFILL i pressure fals lower then calculated offset to set pressure
    might probably collide with timeout inflation control
    */
    if ((!inFlate&&!deFlate)&&(pressure < (p_set_mapped_to_raw_p-calcRefillOffset(pressure)) && trig == 0)){
      //if (millis()-deFlate_time>100){
         // Serial.print("Refill offset: ");
         // Serial.println(calcRefillOffset(pressure));
        // Serial.println(" deflation ");
          deFlate = false;
          inflate();
          inFlate = true;
          inFlate_time = millis();
        //}
    }

    // if ((!inFlate&&!deFlate)&&(pressure > (p_set_mapped_to_raw_p+calcRefillOffset(pressure)) && trig == 0)) {
    //   if (millis()-inFlate_time>500){
    //    Serial.print("Refill offset: ");
    //    Serial.println(calcRefillOffset(pressure));
    //   // Serial.println(" deflation ");
    //     inFlate = false;
    //     deflate();
    //     deFlate = true;
    //     deFlate_time = millis();
    //   }
    // }

  return 0;
}
int ValveChannel::mapToRawP(int p){ // maps pressure setting from GUI land to internal adc raw value
  return int(map(p,GUI_pMapMin,GUI_pMapMax,minPressure,maxPressure));
}
int ValveChannel::calcRefillOffset(int p){
  return (p-minPressure)/10; // add some hysteresis
}

int ValveChannel::calcInflationInertia(int p){
  return (p-minPressure)/20;
}
void ValveChannel::mapPressureToGui(){ // maps pressuref from internal adc raw value to GUI land pressure range
    mappedPressure = int(map(pressure,minPressure,maxPressure,GUI_pMapMin,GUI_pMapMax));
}

int ValveChannel::get_MappedPressure(){ // maps pressuref from internal adc raw value to GUI land pressure range
  return mappedPressure;
}

int ValveChannel::get_Pressure(){
    //pressure = adc->readADC_SingleEnded(adc_ch);
    return pressure;
}

int ValveChannel::get_MappedPoti(){
    return int(map(potiValMapped,minPressure,maxPressure,GUI_pMapMin,GUI_pMapMax));
}

int ValveChannel::get_Poti(){
    return potiValMapped;
}

bool ValveChannel::get_button(){
  return buttonPressed;
}
bool ValveChannel::get_buttonNow(unsigned long d ){
  readButton(d);
  return buttonPressed;
}
bool ValveChannel::get_state(){
  return inFlate==true;
}



// private methods

  int ValveChannel::readPressure(){
      //int adc = adc->readADC_SingleEnded(adc_ch);
      //pressure = filter(adc, 0.3, pressure);
      //pressure = adc->readADC_SingleEnded(adc_ch);

      #if ADS_L
          starttime = micros();
          adc->setMux(adc_ch); //Set single ended mode between AIN0 and GND
          adc->triggerConversion(); //Start a conversion.  This immediatly returns
          pressure =  adc->getConversion(); //This polls the ADS1115 and wait for conversion to finish, THEN returns the value
          endtime = micros();
          Serial.print("Convcompl: ");
          Serial.print(pressure); Serial.print(",  ");
          Serial.print(endtime - starttime);
          Serial.println("us");
      #else
          pressure = adc->readADC_SingleEnded(adc_ch);
      #endif

      return pressure;
  }

    int ValveChannel::readPotentiometer(){
        potiVal = filter(analogRead(potPin), 0.3, potiVal); //check for analog input 1...4
        potiValMapped = int(map(potiVal,0,1023,maxPressure,minPressure));///potiToPressureFactor;
        return potiValMapped;
    }

  void ValveChannel::readButton(unsigned long debounce_delay){
      buttonState = digitalRead(buttonPin);
      if (buttonState != lastFlickerState) {
          lastDebounceTime = millis();
          lastFlickerState = buttonState;
      }
      if ((millis() - lastDebounceTime) > debounce_delay) {
          if(lastButtonState == HIGH && buttonState == LOW) buttonPressed=true;
          else if(lastButtonState == LOW && buttonState == HIGH) buttonPressed=false;
      lastButtonState = buttonState;
      }
  }


  void ValveChannel::inflate(){
      digitalWrite(infValvePin, HIGH);
      digitalWrite(defValvePin, LOW);
  }

  void ValveChannel::deflate(){
      digitalWrite(infValvePin, LOW);
      digitalWrite(defValvePin, HIGH);
  }

  void ValveChannel::stop(){
      //Serial.println("stop called");
      digitalWrite(infValvePin, LOW);
      digitalWrite(defValvePin, LOW);
  }

  void ValveChannel::emergency_watchdog(int maxP){
    if (pressure>maxP) stop();
  }


float ValveChannel::filter(float rawValue, float weight, float lastValue){
  //weighted average (IIR) filter (also accepts integers)
    return  weight * rawValue + (1.0 - weight) * lastValue;
}
