#include "PneuCNTRL.h"


PneuChannel::PneuChannel(int inf_pin, int def_pin, int adc_port, int pot_pin, int button_pin){
    #define DEBOUNCE_DELAY 30UL
    #define TIMEOUT 8000UL

    //Serial.begin(9600);
    infValvePin = inf_pin;
    defValvePin = def_pin;
    adcPort = adc_port;
    potPin = pot_pin;
    buttonPin = button_pin;
    Adafruit_ADS1115 ads; // instantiate out of all scopes
}

int PneuChannel::begin(int min_pressure, int max_pressure, int safety_stop){
    pinMode(infValvePin,OUTPUT);
    pinMode(defValvePin,OUTPUT);
    pinMode(buttonPin,INPUT_PULLUP);

    ads.setGain(GAIN_TWO);
    ads.begin();
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

void PneuChannel::setInertia(int infIn=1000, int defIn=500){
  // change hysteresis boundarys according to inflation and defltion behaviour
  inflationInertia = infIn;
  deflationInertia = defIn;

}

byte PneuChannel::operate_manual(){
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

byte PneuChannel::trigger(int p_set, int trig){
    readPressure();
    readPotentiometer();
    readButton(DEBOUNCE_DELAY);
    emergency_watchdog(safetyStop);

    if (millis()-lastFill>=0){

        lastFill=millis();
        if (pressure < p_set && trig) {
            inflate();
            inFlate = true;
            inFlate_time = millis();
        }
        else if (pressure > (p_set-inflationInertia) && inFlate){
            stop();
            inFlate = false;
        }
        else if (pressure > p_set && trig) {
            deflate();
            deFlate = true;
            deFlate_time = millis();
        }
        else if (pressure < (p_set+deflationInertia) && deFlate){
            stop();
            deFlate = false;
        }
    }
  if (inFlate&&(millis() - inFlate_time>TIMEOUT)) stop();
  if (deFlate&&(millis() - deFlate_time>TIMEOUT)) stop();

  return 0;
}


int PneuChannel::readPressure(){
    //int adc = ads.readADC_SingleEnded(adcPort);
    //pressure = filter(adc, 0.3, pressure);
    pressure = ads.readADC_SingleEnded(adcPort);

    return pressure;
}

int PneuChannel::readPotentiometer(){
    potiVal = filter(analogRead(potPin), 0.3, potiVal); //check for analog input 1...4
    potiValMapped = int(map(potiVal,0,1023,maxPressure,minPressure));///potiToPressureFactor;
    return potiValMapped;
}

void PneuChannel::readButton(unsigned long debounce_delay){
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

void PneuChannel::inflate(){
    digitalWrite(infValvePin, HIGH);
    digitalWrite(defValvePin, LOW);
}

void PneuChannel::deflate(){
    digitalWrite(infValvePin, LOW);
    digitalWrite(defValvePin, HIGH);
}

void PneuChannel::stop(){
    digitalWrite(infValvePin, LOW);
    digitalWrite(defValvePin, LOW);
}

void PneuChannel::emergency_watchdog(int maxP){
  if (readPressure()>maxP) {
    stop();
  }
}

int PneuChannel::get_MappedPressure(int pMapMin, int pMapMax){
    return int(map(pressure,minPressure,maxPressure,pMapMin,pMapMax));
}
int PneuChannel::get_Pressure(){
    return pressure;
}

int PneuChannel::get_MappedPoti(int pMapMin, int pMapMax){
    return int(map(potiValMapped,minPressure,maxPressure,pMapMin,pMapMax));
}

int PneuChannel::get_Poti(){
    return potiValMapped;
}

bool PneuChannel::get_button(){
  return buttonPressed;
}
//weighted average (IIR) filter (also accepts integers)
float PneuChannel::filter(float rawValue, float weight, float lastValue){
    return  weight * rawValue + (1.0 - weight) * lastValue;
}
