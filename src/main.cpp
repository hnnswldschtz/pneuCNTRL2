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

#define MANUAL_MODE false


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
int page = 0;
int trig = 0;
int count = -1;
boolean button = false;



const int sequence_lenght = 4;

struct DATA_P {
  int ch1;
  int ch2;
  int ch3;
};

DATA_P dataPoint_1 = {1050,1300,1200};
DATA_P dataPoint_2 = {1200,1500,1400};
DATA_P dataPoint_3 = {1501,1100,1050};
DATA_P dataPoint_4 = {1110,1050,1190};

DATA_P sequence [] = {dataPoint_1,dataPoint_2,dataPoint_3,dataPoint_4,};
//DATA_P sequence[4];

#define SEQ_LNGTH int((sizeof sequence)/sizeof(*sequence))  // calc keyboard length

void setup() {
    Serial.begin(9600);
    Serial.println("im here");
    ch1.begin(15500,18750,safety_pressure_limit);
    ch2.begin(15500,18750,safety_pressure_limit);
    ch3.begin(15500,18750,safety_pressure_limit);
    ch4.begin(15500,18750,safety_pressure_limit);

    /*set hysteresis boundarys according to used air chamber size and flexibility
    the bigger the air chamber, the lower the values
    */
    ch1.setInertia(1000,500);
    ch2.setInertia(1000,500);
    ch3.setInertia(20,10);
    ch4.setInertia(1000,500);

    ch1.setMappingBoundaries(1000,1510);
    ch2.setMappingBoundaries(1000,1510);
    ch3.setMappingBoundaries(1000,1510);
    ch4.setMappingBoundaries(1000,1510);

  //test valves

    lcd.begin();
    lcd.backlight();// Turn on the backlight

    #if MPRLS_SENS == true
        Serial.println("MPRLS Simple Test");
        if (! mpr.begin()) {
            Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
            while (1) {
              delay(10); //deadlock!!
            }
        }
        Serial.println("Found MPRLS sensor");
    #endif

}

void loop() {


    if (MANUAL_MODE) { // set and apply pressure manually
        ch1.operate_manual();
        ch2.operate_manual();
        ch3.operate_manual();
        ch4.operate_manual();


        knobVal[0] = ch1.get_Poti();
        butVal[0]=ch1.get_button();
        knobVal[1] = ch2.get_Poti();
        butVal[1]=ch2.get_button();
        knobVal[2] = ch3.get_Poti();
        butVal[2]=ch3.get_button();
        knobVal[3] = ch4.get_Poti();
        butVal[3]=ch4.get_button();



        for (int i=0;i<4;i++){
            if ((abs(knobVal[i]-old_knobVal[i])>10)||(butVal[i] != old_butVal[i])) page = i;
            old_knobVal[i] = knobVal[i];
            old_butVal[i] = butVal[i];
        }

        display(page);
      //  sendDataOverSerial();

    } else { // sequenceer mode

        if (ch1.get_buttonNow() && !button) {
            trig = 1;
            button = true;
            count++;
            if (count >= SEQ_LNGTH){
                count=0;
            }
            Serial.print(sequence[count].ch1);
            Serial.print(" ");
            Serial.print(sequence[count].ch2);
            Serial.print(" ");
            Serial.println(sequence[count].ch3);
        }
        if (!ch1.get_buttonNow() && button) {
            button = false;
        }
      // ch4.trigger();
        ch1.trigger(sequence[count].ch1, trig);
        ch2.trigger(sequence[count].ch2, trig);
        ch3.trigger(sequence[count].ch3, trig);
        trig = 0;

        display_seq_3ch(count);
    }

}




/*
HELPER Functions
Display
serial


*/


void display(int page){

    if (page <2){
        lcd.setCursor(0, 0);
        lcd.print("1 v");
        lcd.print("     ");
        lcd.setCursor(3, 0);
        lcd.print(ch1.get_MappedPoti());

        lcd.setCursor(8, 0);
        lcd.print(" p");
        lcd.print("      ");
        lcd.setCursor(10, 0);
        lcd.print(ch1.get_MappedPressure());

        lcd.setCursor(0, 1);
        lcd.print("2 v");
        lcd.print("     ");
        lcd.setCursor(3, 1);
        lcd.print(ch2.get_MappedPoti());

        lcd.setCursor(8, 1);
        lcd.print(" p");
        lcd.print("      ");
        lcd.setCursor(10, 1);
        lcd.print(ch2.get_MappedPressure());
    }
    else{
        lcd.setCursor(0, 0);
        lcd.print("3 v");
        lcd.print("     ");
        lcd.setCursor(3, 0);
        lcd.print(ch3.get_MappedPoti());

        lcd.setCursor(8, 0);
        lcd.print(" p");
        lcd.print("      ");
        lcd.setCursor(10, 0);
        lcd.print(ch3.get_MappedPressure());

        lcd.setCursor(0, 1);
        lcd.print("4 v");
        lcd.print("     ");
        lcd.setCursor(3, 1);
        lcd.print(ch4.get_MappedPoti());

        lcd.setCursor(8, 1);
        lcd.print(" p");
        lcd.print("      ");
        lcd.setCursor(10, 1);
        lcd.print(ch4.get_MappedPressure());
    }

}

void display_seq_3ch(int count){


    lcd.setCursor(0, 0);
    lcd.print("v");
    lcd.print("     ");
    lcd.setCursor(2, 0);
    lcd.print(sequence[count].ch1);

    lcd.setCursor(0, 1);
    lcd.print("p");
    lcd.print("     ");
    lcd.setCursor(2, 1);
    lcd.print(ch1.get_MappedPressure());

    lcd.setCursor(6, 0);
    lcd.print("     ");
    lcd.setCursor(7, 0);
    lcd.print(sequence[count].ch2);

    lcd.setCursor(6, 1);
    lcd.print("     ");
    lcd.setCursor(7, 1);
    lcd.print(ch2.get_MappedPressure());

    lcd.setCursor(11, 0);
    lcd.print("     ");
    lcd.setCursor(12, 0);
    lcd.print(sequence[count].ch3);

    lcd.setCursor(11, 1);
    lcd.print("     ");
    lcd.setCursor(12, 1);
    lcd.print(ch3.get_MappedPressure());
}


    void display_seq(int count){

        // if (true){
        //     lcd.setCursor(0, 0);
        //     lcd.print("1 v");
        //     lcd.print("     ");
        //     lcd.setCursor(3, 0);
        //     lcd.print(sequence[count].ch1);
        //
        //     lcd.setCursor(8, 0);
        //     lcd.print(" p");
        //     lcd.print("      ");
        //     lcd.setCursor(10, 0);
        //     lcd.print(ch1.get_MappedPressure());
        //
        //     lcd.setCursor(0, 1);
        //     lcd.print("2 v");
        //     lcd.print("     ");
        //     lcd.setCursor(3, 1);
        //     lcd.print(sequence[count].ch2);
        //
        //     lcd.setCursor(8, 1);
        //     lcd.print(" p");
        //     lcd.print("      ");
        //     lcd.setCursor(10, 1);
        //     lcd.print(ch2.get_MappedPressure());
        // }
        // else{
            lcd.setCursor(0, 0);
            lcd.print("3 v");
            lcd.print("     ");
            lcd.setCursor(3, 0);
            lcd.print(sequence[count].ch3);

            lcd.setCursor(8, 0);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 0);
            lcd.print(ch3.get_MappedPressure());

            lcd.setCursor(0, 1);
            lcd.print("4 v");
            lcd.print("     ");
            lcd.setCursor(3, 1);
            lcd.print(ch4.get_MappedPoti());

            lcd.setCursor(8, 1);
            lcd.print(" p");
            lcd.print("      ");
            lcd.setCursor(10, 1);
            lcd.print(ch4.get_MappedPressure());
        // }

}




void sendDataOverSerial(){

    #if MPRLS_SENS == true
        float pressure_hPa = mpr.readPressure();
    #endif

    Serial.print(ch1.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch2.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch3.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch3.get_MappedPoti());
    //Serial.print("\t");
    //Serial.println(ch4.get_MappedPressure(1000,1510));

    #if MPRLS_SENS == true
        Serial.print("\t");
        Serial.println(pressure_hPa);
    #endif
    Serial.print("\n");
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
