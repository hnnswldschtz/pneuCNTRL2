#include "PneuCNTRL.h"

//public methods

ValveChannel::ValveChannel(int inf_pin, int def_pin, int adc_port, int pot_pin, int button_pin){
    #define DEBOUNCE_DELAY 30UL
    #define TIMEOUT 10000UL

    //Serial.begin(9600);
    infValvePin = inf_pin;
    defValvePin = def_pin;
    adcPort = adc_port;
    potPin = pot_pin;
    buttonPin = button_pin;
    Adafruit_ADS1115 ads; // instantiate out of all scopes
    pMapMin = 1000;
    pMapMax = 1510;
}

int ValveChannel::begin(int min_pressure, int max_pressure, int safety_stop){
    pinMode(infValvePin,OUTPUT);
    pinMode(defValvePin,OUTPUT);
    pinMode(buttonPin,INPUT_PULLUP);

    ads.setGain(GAIN_TWO);
    ads.begin();// put i2c adress here.
    minPressure = min_pressure;
    maxPressure = max_pressure;
    safetyStop = safety_stop;

    inflate();
    delay(100);
    deflate();
    delay(100);
    stop();

    //Serial.print("Getting single-ended readings from AIN ");
    //Serial.println (adcPort);

    return 0;
}

void ValveChannel::setInertia(int infIn=1000, int defIn=500){
  // change hysteresis boundarys according to inflation and defltion behaviour
  inflationInertia = infIn;
  deflationInertia = defIn;
}

void ValveChannel::setMappingBoundaries(int _mapMin, int _mapMax){
  pMapMin = _mapMin;
  pMapMax = _mapMax;
}

byte ValveChannel::operate_manual(){
    readPressure();
    readPotentiometer();
    readButton(DEBOUNCE_DELAY);
    emergency_watchdog(safetyStop);

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
  if (inFlate&&(millis() - inFlate_time>TIMEOUT)) stop();
  if (deFlate&&(millis() - deFlate_time>TIMEOUT)) stop();

  return 0;
}

byte ValveChannel::trigger(int p_set, int trig){
    readPressure();
    //readPotentiometer();
    //readButton(DEBOUNCE_DELAY);
    emergency_watchdog(safetyStop);
    int p_set_mapped_to_raw_p = mapToRawP(p_set); // we calculate internally with the raw input value range from adc

    if (millis()-lastFill>=0){

        lastFill=millis();
        if (old_p_set!=p_set_mapped_to_raw_p&&(pressure < p_set_mapped_to_raw_p && trig == 1)) {
            deFlate = false;
            stop();
            // Serial.print("in, ");
            // Serial.print("p_set: ");
            // Serial.print(p_set);
            // Serial.print(", p_set: ");
            // Serial.print(p_set_mapped_to_raw_p);
            Serial.print(", pressure: ");
            Serial.println(pressure);
            inflate();
            inFlate = true;
            inFlate_time = millis();
            old_p_set=p_set_mapped_to_raw_p;
        }
        else if (pressure > (p_set_mapped_to_raw_p-calcInflationInertia(pressure)) && inFlate){

            // Serial.print("at ");
            // Serial.print(pressure);
            // Serial.print(" inflation ");
            stop();

            inFlate = false;
        }
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
        else if (pressure < (p_set_mapped_to_raw_p+deflationInertia) && deFlate){
          // Serial.print("at ");
          // Serial.print(pressure);
          // Serial.print(" deflation ");
            stop();
            deFlate = false;
        }
    }
    //timeout if inflation never reaches top
    if (inFlate&&(millis() - inFlate_time>TIMEOUT)) {
        Serial.print ("timeout ");
        inFlate=false;
        stop();
    }
    //timeout if inflation never reaches bottom
    if (deFlate&&(millis() - deFlate_time>TIMEOUT)) {
        Serial.print ("timeout ");
        deFlate=false;
        stop();
    }

    /*REFILL i pressure fals lower then calculated offset to set pressure
    might probably collide with timeout inflation control
    */
    if ((!inFlate&&!deFlate)&&(pressure < (p_set_mapped_to_raw_p-calcRefillOffset(pressure)) && trig == 0)) {
       Serial.print("Refill offset: ");
       Serial.println(calcRefillOffset(pressure));
      // Serial.println(" deflation ");
        deFlate = false;
        inflate();
        inFlate = true;
        inFlate_time = millis();
    }

  return 0;
}
int ValveChannel::mapToRawP(int p){ // maps pressure setting from GUI land to internal adc raw value
  return int(map(p,pMapMin,pMapMax,minPressure,maxPressure));
}
int ValveChannel::calcRefillOffset(int p){
  return (p-minPressure)/10;
}

int ValveChannel::calcInflationInertia(int p){
  return (p-minPressure)/20;
}

int ValveChannel::get_MappedPressure(){ // maps pressuref from internal adc raw value to GUI land pressure range
    return int(map(pressure,minPressure,maxPressure,pMapMin,pMapMax));
}
int ValveChannel::get_Pressure(){
    return pressure;
}

int ValveChannel::get_MappedPoti(){
    return int(map(potiValMapped,minPressure,maxPressure,pMapMin,pMapMax));
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
      //int adc = ads.readADC_SingleEnded(adcPort);
      //pressure = filter(adc, 0.3, pressure);
      pressure = ads.readADC_SingleEnded(adcPort);
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
      Serial.println("stop called");
      digitalWrite(infValvePin, LOW);
      digitalWrite(defValvePin, LOW);
  }

  void ValveChannel::emergency_watchdog(int maxP){
    if (readPressure()>maxP) stop();
  }


float ValveChannel::filter(float rawValue, float weight, float lastValue){
  //weighted average (IIR) filter (also accepts integers)
    return  weight * rawValue + (1.0 - weight) * lastValue;
}
