/*
pneuCNTRlbox Firmware.
connect either a potentiometer to A2
as well as to pin8 and 11 interonnected by a 4k7 ohms resistor


*/


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <LiquidCrystal_I2C.h>
#include "PneuCNTRL.h"



#define valve_A1 4
#define valve_A2 8
#define valve_B3 5
#define valve_B4 10
#define valve_C5 6
#define valve_C6 11
#define valve_D7 7
#define valve_D8 12

#define SENSE_PIN_1 0
#define SENSE_PIN_2 1
#define SENSE_PIN_3 2
#define SENSE_PIN_4 3

int valve[]={4,8,5,10,6,11,7,12};
Adafruit_ADS1115 ads; // 16-bit adc


LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

/* PneuChannel Class Constructor,
Takes 5 Arguments Inflate Valve pin, deflate valve pin,
ads1115 input ch, potentiomer pin, button pin
returns ?
*/
PneuChannel ch1(valve_A1,valve_A2,SENSE_PIN_1,0,58); //instance of channel
PneuChannel ch2(valve_B3,valve_B4,SENSE_PIN_2,0,58);
PneuChannel ch3(valve_C5,valve_C6,SENSE_PIN_3,0,58);
PneuChannel ch4(valve_D7,valve_D8,SENSE_PIN_4,0,58);

PneuChannel chArray[]={ch1,ch2,ch3,ch4};

const int pad_1_Fill_Valve=valve_A1;
const int pad_1_Ex_Valve=valve_A2;
const int pad_2_Fill_Valve=valve_B3;
const int pad_2_Ex_Valve=valve_B4;
const int tank_Fill_Valve=valve_C5;
const int tank_Ex_Valve=valve_C5;

int max_pressure=22000; //where safety kicks in
int16_t adc[4];
byte adcPort[]={0,1,2,3};


int baguettePressure = 0;        // value read from the pot
int pattern1_pressure = 0;
int pattern2_pressure = 0;
int pressure[4];     // value read from the pot
int potiVal[4];
int old_potiVal[4];
int potiToPressure[4];
boolean knobTurned[4];
int page=0;
int potiToPressureFactor=3;
int potentiometerMax=21000;
int potentiometerMin=16000;
int potiPin[]={0,1,2,3};
int buttonPin[]={58,59,18,19};
long lastFillTime=0;
long lastFill[4];
int count=0;

boolean fill[]={false,false,false,false};
boolean release[]={false,false,false,false};
boolean buttonPressed[]={false,false,false,false};
boolean buttonState[]={false,false,false,false};
boolean lastButtonState[]={false,false,false,false};

float filter(float , float , float );

void setup() {
  Serial.begin(9600);

ch1.setPressureRange(10000,20000,22000);
ch1.setPressureRange(10000,20000,22000);
ch1.setPressureRange(10000,20000,22000);
ch1.setPressureRange(10000,20000,22000);


  pinMode(valve_A1,OUTPUT);
  pinMode(valve_A2,OUTPUT);
  pinMode(valve_B3,OUTPUT);
  pinMode(valve_B4,OUTPUT);
  pinMode(valve_C5,OUTPUT);
  pinMode(valve_C6,OUTPUT);
  pinMode(valve_D7,OUTPUT);
  pinMode(valve_D8,OUTPUT);



  for (int i =0;i<4;i++) pinMode(buttonPin[i], INPUT_PULLUP);

  for (int i =0;i<8;i++){
    digitalWrite(valve[i], HIGH);
    delay(100);
    digitalWrite(valve[i], LOW);
    delay(100);
  }

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads.begin();
  lcd.begin();
// Turn on the blacklight and print a message.
lcd.backlight();
}

void loop() {

//readout and process sensor input and interface elements
  //ch1.read_Pressure();
  //ch1.deflate();
  //ch1.inflate();
  for (int i=0;i<4;i++){
    chArray[i].operate();

    adc[i] = 1;//ads.readADC_SingleEnded(adcPort[i]);
    pressure[i] = filter(adc[i], 0.3, pressure[i]);
    potiVal[i] = filter(analogRead(i), 0.3, potiVal[i]); //check for analog input 1...4
    potiToPressure[i]=int(map(potiVal[i],0,1023,potentiometerMax,potentiometerMin));///potiToPressureFactor;
    if (potiVal[i]!= old_potiVal[i]) {
      knobTurned[i]=true;
      page=i;
    }
    old_potiVal[i] = potiVal[i];
    if (pressure[i] >= max_pressure) digitalWrite(valve[i*2], LOW); //global safety!!!
    buttonState[i]=digitalRead(buttonPin[i]);
    if (buttonState[i]==LOW && lastButtonState[i]==HIGH)  {
    buttonPressed[i]=true;
  }else   buttonPressed[i]=false;
  lastButtonState[i]=buttonState[i];
    if (millis()-lastFill[i]>1){
     lastFill[i]=millis();
    Serial.print("HI");
     if (pressure[i] < potiToPressure[i]&&buttonPressed[i]) {
       digitalWrite(valve[i*2], HIGH);
       digitalWrite(valve[i*2+1], LOW);
       fill[i]=true;
       }
     else if (pressure[i] > potiToPressure[i]&&fill[i]){
     digitalWrite(valve[i*2], LOW);
     digitalWrite(valve[i*2+1], LOW);
         fill[i]=false;
     }
     else if (pressure[i] > potiToPressure[i]&&buttonPressed[i]) {
       digitalWrite(valve[i*2], LOW);
       digitalWrite(valve[i*2+1], HIGH);
       release[i]=true;
       }
     else if (pressure[i] < potiToPressure[i]&&release[i]){
     digitalWrite(valve[i*2], LOW);
     digitalWrite(valve[i*2+1], LOW);
         release[i]=false;
     }
    }


  }

  //max_pressure=potiToPressure;

if (page <2){
  lcd.setCursor(0, 0);
  lcd.print("1 v");
  lcd.print(potiToPressure[0]);

  lcd.setCursor(8, 0);
  lcd.print(" p");
  lcd.print(pressure[0]);

  lcd.setCursor(0, 1);
  lcd.print("2 v");
  lcd.print(potiToPressure[1]);

  lcd.setCursor(8, 1);
  lcd.print(" p");
  lcd.print(pressure[1]);
}
else{
  lcd.setCursor(0, 0);
  lcd.print("3 v");
  lcd.print(potiToPressure[2]);

  lcd.setCursor(8, 0);
  lcd.print(" p");
  lcd.print(pressure[2]);

  lcd.setCursor(0, 1);
  lcd.print("4 v");
  lcd.print(potiToPressure[3]);

  lcd.setCursor(8, 1);
  lcd.print(" p");
  lcd.print(pressure[3]);
}


  delay(1);
  //lcd.clear();

// if (digitalRead(buttonPin[0])==LOW)  {
//     digitalWrite(pad_1_Fill_Valve, HIGH);
//     digitalWrite(pad_1_Ex_Valve, LOW);
//     }
//   else if (digitalRead(buttonPin[1])==LOW){
//   digitalWrite(pad_1_Ex_Valve, HIGH);
//   digitalWrite(pad_1_Fill_Valve, LOW);
//   }
//   else {
//     digitalWrite(pad_1_Ex_Valve, LOW);
//     digitalWrite(pad_1_Fill_Valve, LOW);
//   }
// if (digitalRead(buttonPin[0])==LOW)  {
// fill[0]=true;
//     }
//   else if (digitalRead(buttonPin[1])==LOW){
// fill[0]=true;
//   }



}


// int setPressure(int pin, int p){
//
// }

//weighted average (IIR) filter (also accepts integers)
float filter(float rawValue, float weight, float lastValue)
{
  float result = weight * rawValue + (1.0 - weight) * lastValue;
  return result;
}


// Performs an ADC reading on the internal GND channel in order
// to clear any voltage that might be leftover on the ADC.
// Only works on AVR boards and silently fails on others.
// void analogReset()
// {
// #if defined(ADMUX)
// #if defined(ADCSRB) && defined(MUX5)
//     // Code for the ATmega2560 and ATmega32U4
//     ADCSRB |= (1 << MUX5);
// #endif
//     ADMUX = 0x1F;
//
//     // Start the conversion and wait for it to finish.
//     ADCSRA |= (1 << ADSC);
//     loop_until_bit_is_clear(ADCSRA, ADSC);
// #endif
// }
