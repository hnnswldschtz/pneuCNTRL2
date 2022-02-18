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

// eiter manual mode or sequence mode
#define MANUAL_MODE true

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display


/* ValveChannel Class Constructor,
Takes 5 Arguments Inflate Valve pin, deflate valve pin,
ads1115 input ch, potentiometer pin, button pin
returns ?
*/
ValveChannel ch1(valve_A1,valve_A2,SENSE_PIN_1,0,58); //instance of channel
ValveChannel ch2(valve_B3,valve_B4,SENSE_PIN_2,1,59);
ValveChannel ch3(valve_C5,valve_C6,SENSE_PIN_3,2,18);
ValveChannel ch4(valve_D7,valve_D8,SENSE_PIN_4,3,19);

ProportionalChannel ch5(valve_B3,valve_B4,SENSE_PIN_2,1,59);
ProportionalChannel ch6(valve_C5,valve_C6,SENSE_PIN_3,2,18);
ProportionalChannel ch7(valve_D7,valve_D8,SENSE_PIN_4,3,19);

ValveChannel chArray[] = {ch1,ch2,ch3,ch4};


int knobVal[4];
int old_knobVal[4];
int butVal[4];
int old_butVal[4];
//boolean knobTurned[4];
int page = 0;
int trig = 0;
int count = -1;
boolean button = false;


DATA_P dataPoint_1 = {100,100,50,50};
DATA_P dataPoint_2 = {10,5,60,60};
DATA_P dataPoint_3 = {20,20,70,70};
DATA_P dataPoint_4 = {15,20,80,80};
DATA_P dataPoint_5 = {10,30,90,90};
DATA_P dataPoint_6 = {20,30,100,100};
DATA_P dataPoint_7 = {10,10,5,00};
DATA_P dataPoint_8 = {3,3,6,10};
DATA_P dataPoint_9 = {20,20,15,20};
DATA_P dataPoint_10 = {20,20,20,30};
DATA_P dataPoint_11 = {20,20,30,40};

DATA_P sequence [] = {dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5, dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10, dataPoint_11};
//DATA_P sequence[4];

#define SEQ_LNGTH int((sizeof sequence)/sizeof(*sequence))  // calc sequence length

/*
BE CAREFULL!!!
A WRONG SETTING CAUSES YOUR INFLATABLES TO EXPLODE!!!
TESTET WITH 4 Bar Pressure input from Compressor
*/

const int BAGUETTE_SAFETY_PRESSURE_LIMIT = 19000; //where safety kicks in
const int LENKRAD_SAFETY_PRESSURE_LIMIT = 18000; //where safety kicks in
const int MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT = 25000;
const int SPINACKER_STRUCTURE_SAFETY_PRESSURE_LIMIT = 19000;

void setup() {

    Serial.begin(9600);
    Serial.println("im here");
    lcd.begin();
    lcd.backlight();// Turn on the backlight
    ch1.begin(15500,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT-250,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT); // lower and upper pressure vals are experimentally derived from adc wset to ads.setGain(GAIN_TWO);
    ch2.begin(15500,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT-250,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT);
    ch3.begin(15500,LENKRAD_SAFETY_PRESSURE_LIMIT-250,LENKRAD_SAFETY_PRESSURE_LIMIT);
    ch4.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT);

    /*set hysteresis boundarys according to used air chamber size and flexibility
    the bigger the air chamber, the lower the values
    */
    ch1.setInertia(20,10);
    ch2.setInertia(20,10);
    ch3.setInertia(20,10);
    ch4.setInertia(20,10);


    /*//value range for displayed in GUI */
    ch1.setMappingBoundaries(0,100);
    ch2.setMappingBoundaries(0,100);
    ch3.setMappingBoundaries(0,100);
    ch4.setMappingBoundaries(0,100);

  //test valves



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


    if (MANUAL_MODE) { /*________manual mode__________*/
      // set and apply pressure manually
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


        /*page switcher. Got to page where last change happend*/
        for (int i=0;i<4;i++){
            if ((abs(knobVal[i]-old_knobVal[i])>10)||(butVal[i] != old_butVal[i])) page = i;
            old_knobVal[i] = knobVal[i];
            old_butVal[i] = butVal[i];
        }

        display(page);
      //  sendDataOverSerial();

    } else { /*________sequencer mode__________*/
        /*steps through sequence by pressing button 1*/
        if (ch1.get_buttonNow() && !button) {
            trig = 1;
            button = true;
            count++;
            if (count >= SEQ_LNGTH) count = 0;
            Serial.print("count: ");
            Serial.println(count);
            Serial.print(sequence[count].ch1_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch2_val);
            Serial.print(" ");
            Serial.println(sequence[count].ch3_val);
            Serial.print(" ");
            Serial.println(sequence[count].ch4_val);
        }
        if (!ch1.get_buttonNow() && button) {
            button = false;
        }

        ch1.trigger(sequence[count].ch1_val, trig);
        ch2.trigger(sequence[count].ch2_val, trig);
        ch3.trigger(sequence[count].ch3_val, trig);
        ch4.trigger(sequence[count].ch4_val, trig);

      // ch4.trigger();
        trig = 0;

        display_seq_4ch(count);
    }

}




/*
HELPER Functions
Display
serial


*/

/* display fundtion for manual mode*/
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
        if (ch1.get_state()) lcd.print(".");
        else lcd.print(" ");

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
        if (ch2.get_state()) lcd.print(".");
        else lcd.print(" ");
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
        if (ch3.get_state()) lcd.print(".");
        else lcd.print(" ");

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
        if (ch4.get_state()) lcd.print(".");
        else lcd.print(" ");
    }

}

/* display fundtion for sequencer  mode*/
void display_seq_4ch(int count){


    lcd.setCursor(0, 0);
    // lcd.print("v");
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(sequence[count].ch1_val);


    lcd.setCursor(0, 1);
    // lcd.print("p");
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print(ch1.get_MappedPressure());
    if (ch1.get_state()) lcd.print(".");
    else lcd.print(" ");

    lcd.setCursor(3, 0);
    lcd.print("     ");
    lcd.setCursor(4, 0);
    lcd.print(sequence[count].ch2_val);

    lcd.setCursor(3, 1);
    lcd.print("     ");
    lcd.setCursor(4, 1);
    lcd.print(ch2.get_MappedPressure());
    if (ch2.get_state()) lcd.print(".");
    else lcd.print(" ");

    lcd.setCursor(7, 0);
    lcd.print("     ");
    lcd.setCursor(8, 0);
    lcd.print(sequence[count].ch3_val);

    lcd.setCursor(7, 1);
    lcd.print("     ");
    lcd.setCursor(8, 1);
    lcd.print(ch3.get_MappedPressure());
    if (ch3.get_state()) lcd.print(".");
    else lcd.print(" ");

    lcd.setCursor(11, 0);
    lcd.print("     ");
    lcd.setCursor(12, 0);
    lcd.print(sequence[count].ch4_val);

    lcd.setCursor(11, 1);
    lcd.print("     ");
    lcd.setCursor(12, 1);
    lcd.print(ch4.get_MappedPressure());
    if (ch4.get_state()) lcd.print(".");
    else lcd.print(" ");
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
    Serial.println(ch4.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch3.get_MappedPoti());
    Serial.print("\t");

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
