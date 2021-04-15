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
#include "Adafruit_MPRLS.h"
#define MPRLS_SENS false
// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
#if MPRLS_SENS == true
  Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);
#endif

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

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

/* PneuChannel Class Constructor,
Takes 5 Arguments Inflate Valve pin, deflate valve pin,
ads1115 input ch, potentiomer pin, button pin
returns ?
*/
PneuChannel ch1(valve_A1,valve_A2,SENSE_PIN_1,0,58); //instance of channel
PneuChannel ch2(valve_B3,valve_B4,SENSE_PIN_2,1,59);
PneuChannel ch3(valve_C5,valve_C6,SENSE_PIN_3,2,18);
PneuChannel ch4(valve_D7,valve_D8,SENSE_PIN_4,3,19);

PneuChannel chArray[] = {ch1,ch2,ch3,ch4};

const int safety_pressure_limit = 19000; //where safety kicks in

int knobVal[4];
int old_knobVal[4];
int butVal[4];
int old_butVal[4];
//boolean knobTurned[4];
int page=0;

void display(int);

void setup() {
    Serial.begin(9600);
    Serial.println("im here");
    ch1.begin(15500,18750,safety_pressure_limit);
    ch2.begin(15500,18750,safety_pressure_limit);
    ch3.begin(15500,18750,safety_pressure_limit);
    ch4.begin(15500,18750,safety_pressure_limit);
//set hysteresis boundarys according to used air chamber size and flexibility
    ch1.setInertia(1000,500);
    ch2.setInertia(1000,500);
    ch3.setInertia(20,10);
    ch4.setInertia(1000,500);

  //test valves

    lcd.begin();
    lcd.backlight();// Turn on the backlight

#if MPRLS_SENS == true
      Serial.println("MPRLS Simple Test");
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found MPRLS sensor");
#endif

}

void loop() {
//   static long lastFrame;
//       if (millis()-lastFrame>500){
//          lastFrame=millis();
//   float pressure_hPa = mpr.readPressure();
// Serial.print("Pressure (hPa): "); Serial.println(pressure_hPa);
// Serial.print("Pressure (PSI): "); Serial.println(pressure_hPa / 68.947572932);
// }
  #if MPRLS_SENS == true
    float pressure_hPa = mpr.readPressure();
  #endif
    ch1.operate_manual();
    knobVal[0] = ch1.get_Poti();
    butVal[0]=ch1.get_button();

    ch2.operate_manual();
    knobVal[1] = ch2.get_Poti();
    butVal[1]=ch2.get_button();

    ch3.operate_manual();
    knobVal[2] = ch3.get_Poti();
    butVal[2]=ch3.get_button();

    ch4.operate_manual();
    knobVal[3] = ch4.get_Poti();
    butVal[3]=ch4.get_button();

    for (int i=0;i<4;i++){
        if ((abs(knobVal[i]-old_knobVal[i])>10)||(butVal[i] != old_butVal[i])) {
            //knobTurned[i] = true;
            page = i;
        }
        old_knobVal[i] = knobVal[i];
        old_butVal[i] = butVal[i];
    }

    display(page);


    if (ch1.get_MappedPressure(1000,1510)>=1000) Serial.print(ch1.get_MappedPressure(1000,1510));
    Serial.print("\t");
      if (ch2.get_MappedPressure(1000,1510)>=1000) Serial.print(ch2.get_MappedPressure(1000,1510));
    Serial.print("\t");
      if (ch3.get_MappedPressure(1000,1510)>=1000) Serial.print(ch3.get_MappedPressure(1000,1510));
    Serial.print("\t");
    Serial.print(ch3.get_MappedPoti(1000,1510));
    // Serial.print(ch3.get_Pressure());
    // Serial.print("\t");
  #if MPRLS_SENS == true
    Serial.print("\t");
    Serial.println(pressure_hPa);
  #endif
  Serial.print("\n");
    //Serial.print("\t");
    //Serial.println(ch4.get_MappedPressure(0,5000));
}

void display(int page){
//  static long lastFrame;
    // if (millis()-lastFrame>100){
    //     lastFrame=millis();
    //     lcd.setCursor(3, 0);
    //     lcd.print("     ");
    //
    //     lcd.setCursor(11, 0);
    //     lcd.print("     ");
    //   }
        if (page <2){
            lcd.setCursor(0, 0);
            lcd.print("1 v");
            lcd.print("     ");
            lcd.setCursor(3, 0);
            lcd.print(ch1.get_MappedPoti(1000,1510));

            lcd.setCursor(8, 0);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 0);
            lcd.print(ch1.get_MappedPressure(1000,1510));

            lcd.setCursor(0, 1);
            lcd.print("2 v");
            lcd.print("     ");
            lcd.setCursor(3, 1);
            lcd.print(ch2.get_MappedPoti(1000,1510));

            lcd.setCursor(8, 1);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 1);
            lcd.print(ch2.get_MappedPressure(1000,1510));
        }
        else{
            lcd.setCursor(0, 0);
            lcd.print("3 v");
            lcd.print("     ");
            lcd.setCursor(3, 0);
            lcd.print(ch3.get_MappedPoti(1000,1510));

            lcd.setCursor(8, 0);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 0);
            lcd.print(ch3.get_MappedPressure(1000,1510));

            lcd.setCursor(0, 1);
            lcd.print("4 v");
            lcd.print("     ");
            lcd.setCursor(3, 1);
            lcd.print(ch4.get_MappedPoti(1000,1510));

            lcd.setCursor(8, 1);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 1);
            lcd.print(ch4.get_MappedPressure(1000,1510));
        }

}




// The ADC input range (or gain) can be changed via the following
// functions, but be careful never to exceed VDD +0.3V max, or to
// exceed the upper and lower limits if you adjust the input range!
// Setting these values incorrectly may destroy your ADC!
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
//ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
