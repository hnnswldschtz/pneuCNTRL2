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
//#include <ADS1115_lite.h>

// either manual mode or sequence mode
#define MANUAL_MODE false
#define ADS_L false

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
Adafruit_NeoPixel switch_neo_pixels(NEO_SWITCH_NUM_PIXELS, NEO_SWITCH_PIN, NEO_GRB + NEO_KHZ800);

#if ADS_L
  ADS1115_lite Psens_adc(0x49);
  ADS1115_lite PV_adc(0x48);
#else
  Adafruit_ADS1115 Psens_adc; // instance of ads1115 Psens_adc  out of all scopes
  Adafruit_ADS1115 PV_adc;
#endif
Adafruit_MCP4728 PV_dac;


Button Button_switcher;


// class ADCtest{
//
// private:
//
//   Adafruit_ADS1115 * adc;
//
// public:
//   ADCtest(Adafruit_ADS1115 * _adc){
//   adc = _adc;
//   adc->setGain(GAIN_TWO);
//   adc->begin();
//   }
//   int readADC(){
//     int pressure = adc->readADC_SingleEnded(1);
//     return pressure;
//   }
//
// };


/* ValveChannel Class Constructor,
Takes 6 Arguments Inflate Valve pin, deflate valve pin, adafruit ads1115 adc,
ads1115 input ch, potentiometer pin, button pin
returns ?
*/

// --------- all four buttons are acivated in manual mode
// #if MANUAL_MODE==true
//     ValveChannel ch1(valve_A1, valve_A2, &Psens_adc, SENSE_PIN_1, POTI_1, BUTTON_1); //instance of channel
// // --------- button 1 is for switching sets in sequencer change init to button_2 to enable interrupt on button_1
// #elif MANUAL_MODE==false
//     ValveChannel ch1(valve_A1, valve_A2, &Psens_adc, SENSE_PIN_1, POTI_1, BUTTON_2); //instance of channel
// #endif
//
// ValveChannel ch2(valve_B3, valve_B4, &Psens_adc, SENSE_PIN_2, POTI_2, BUTTON_2);
// ValveChannel ch3(valve_C5, valve_C6, &Psens_adc, SENSE_PIN_3, POTI_3, BUTTON_3);
// ValveChannel ch4(valve_D7, valve_D8, &Psens_adc, SENSE_PIN_4, POTI_4, BUTTON_4);


#if MANUAL_MODE==true
  ValveChannel ch1(valve_A1, valve_A2, POTI_1, BUTTON_1); //instance of channel
// --------- button 1 is for switching sets in sequencer change init to button_2 to enable interrupt on button_1
#elif MANUAL_MODE==false
  ValveChannel ch1(valve_A1, valve_A2, POTI_1, BUTTON_2); //instance of channel
#endif

ValveChannel ch2(valve_B3, valve_B4, POTI_2, BUTTON_2);
ValveChannel ch3(valve_C5, valve_C6, POTI_3, BUTTON_3);
ValveChannel ch4(valve_D7, valve_D8, POTI_4, BUTTON_4);

ValveChannel * vArray[]= {&ch1, &ch2, &ch3, &ch4};

ProportionalChannel ch5P;
ProportionalChannel ch6P;
ProportionalChannel ch7P;


int knobVal[4];
int old_knobVal[4];
int butVal[4];
int old_butVal[4];
//boolean knobTurned[4];
int page = 0;
int trig = 0;
int count = -1;
unsigned long triggerMetro;
boolean button = false;
boolean startup = true;

unsigned long ledOnTime = 0;
boolean showIt = false;
boolean lightsOut = false;
unsigned long brightFadeTime = 0;
byte brightness;
boolean fading  = false;

/*
DATA_P dataPoint_1 = {100, 100, 50, 50};
DATA_P dataPoint_2 = {10, 5, 60, 60};
DATA_P dataPoint_3 = {20, 20, 70, 70};
DATA_P dataPoint_4 = {15, 20, 80, 80};
DATA_P dataPoint_5 = {10, 30, 90, 90};
DATA_P dataPoint_6 = {20, 30, 100, 100};
DATA_P dataPoint_7 = {10, 10, 5, 00};
DATA_P dataPoint_8 = {3, 3, 6, 10};
DATA_P dataPoint_9 = {20, 20, 15, 20};
DATA_P dataPoint_10 = {20, 20, 20, 30};
DATA_P dataPoint_11 = {20, 20, 30, 40};
DATA_P sequence [] = {dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5, dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10, dataPoint_11};
*/


/* MARCH 2022:
how does your bankaccount feel?
connected:
ch 1: Haptic Baguett (core)
ch 2: Flute
ch 3: Haptic Pattern 1
ch 4: Haptic Pattern 2
ch 5P: BIG BAG
ch 6P: test Flute/notn
*/



SEQ_CH ch1_seq = {32, 57, 99, 18,  10,  6 };//ch 1: Haptic Baguett (core)
SEQ_CH ch2_seq = {32, 57, 99, 18,  9,  0 };//ch 2: Flute
SEQ_CH ch3_seq = { 0, 20, 99,  0,  0,  0 };//ch 3: Haptic Pattern 1
SEQ_CH ch4_seq = {70, 99, 99, 37, 25,  0 };//ch 4: Haptic Pattern 2
SEQ_CH ch5_seq = {32, 57, 99, 18,  6,  0 };//ch 5P: BIG BAG
SEQ_CH ch6_seq = {32, 57, 99, 18,  9,  0 };//ch 6P: test Flute/notn


// DATA_P dataPoint_1 = {32, 32,  0,  70, 32, 32};
// DATA_P dataPoint_2 = {57, 57,  20, 99, 57, 57};
// DATA_P dataPoint_3 = {99, 99,  99, 99, 99, 99};
// DATA_P dataPoint_4 = {18, 18,   0, 37, 18, 18}; // was 34/73
// DATA_P dataPoint_5 = {12,  9,   0, 25,  6,  9}; // set to nine to reach 12
// DATA_P dataPoint_6 = { 8,  0,   0,  0,  0,  0};

DATA_P dataPoint_1 = {85, 32,  0,  70, 32, 32};
DATA_P dataPoint_2 = {85, 57,  20, 99, 57, 57};
DATA_P dataPoint_3 = {85, 99,  99, 99, 99, 99};
DATA_P dataPoint_4 = {85, 18,   0, 37, 18, 18}; // was 34/73
DATA_P dataPoint_5 = {85,  10,   0, 25,  6,  9}; // set to nine to reach 12
DATA_P dataPoint_6 = {85,  0,   0,  0,  0,  0};

DATA_P sequence [] = {dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5, dataPoint_6};
//DATA_P sequence[4];

#define SEQ_LNGTH int((sizeof sequence)/sizeof(*sequence))  // calc sequence length

/*
BE CAREFULL!!!
A WRONG SETTING CAUSES YOUR INFLATABLES TO EXPLODE!!!
TESTET WITH 4 Bar Pressure input from Compressor
*/
const long TIMEOUT = 60000;

const int BIGBAG_SAFETY_PRESSURE_LIMIT = 17500; //where safety kicks in
const int FLUTE_SAFETY_PRESSURE_LIMIT = 17500;//18700; //where safety kicks in
const int BAGUETTE_SAFETY_PRESSURE_LIMIT = 21000; //where safety kicks in
const int MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT = 25000;
const int SPINACKER_STRUCTURE_SAFETY_PRESSURE_LIMIT = 19000;
//ADCtest testADC(&ads);
int prsssr = 0;
unsigned long lastLast = 0;

void setup() {

    switch_neo_pixels.begin();
    Serial.begin(115200);
    delay(1500);
    Serial.println("Hello, I'm here and ready to accept instructions");
    lcd.init();
    lcd.backlight();// Turn on the backlight

    #if ADS_L
      Psens_adc.setGain(ADS1115_REG_CONFIG_PGA_2_048V);
      Psens_adc.setSampleRate(ADS1115_REG_CONFIG_DR_860SPS);
      PV_adc.setGain(ADS1115_REG_CONFIG_PGA_4_096V);
      PV_adc.setSampleRate(ADS1115_REG_CONFIG_DR_860SPS);
    #else
      Psens_adc.setGain(GAIN_TWO); // 2,048V Max. set to two or higher because of spte sensors 1bar ~ 0,5V
      Psens_adc.begin(0x49);

      PV_adc.setGain(GAIN_ONE); // 4,096V max. set to one because of spte sensors 1bar ~ 5V / 2,5V
      PV_adc.begin(0x48);
    #endif



  delay(2000);
  if (!PV_dac.begin()) {
      Serial.println("Failed to find MCP4728 chip");
  }

  // if (!PV_adc.testConnection()) {
  //   Serial.println("PropChanAdc Connection failed"); //oh man...something is wrong
  //   return;
  // }
  // if (!Psens_adc.testConnection()) {
  //   Serial.println("ValvePSens Connection failed"); //oh man...something is wrong
  //   return;
  // }

    pinMode(BUTTON_SWITCH, INPUT_PULLUP);
    //attachInterrupt(BUTTON_SWITCH, setFlagHandler , FALLING);

    //----------- activate BUTTON_1 interrupt when running auto sequencer mode
#if MANUAL_MODE==false
    pinMode(BUTTON_1, INPUT_PULLUP);
    //attachInterrupt(BUTTON_1, setFlagHandler , FALLING);
#endif
    //---------------
  //  pinMode(SWITCH_LED, OUTPUT);
  #if ADS_L
    ch1.setAdc(&Psens_adc, ADS1115_REG_CONFIG_MUX_SINGLE_0);
    ch2.setAdc(&Psens_adc, ADS1115_REG_CONFIG_MUX_SINGLE_1);
    ch3.setAdc(&Psens_adc, ADS1115_REG_CONFIG_MUX_SINGLE_2);
    ch4.setAdc(&Psens_adc, ADS1115_REG_CONFIG_MUX_SINGLE_3);
  #else
    ch1.setAdc(&Psens_adc, SENSE_PIN_1);
    ch2.setAdc(&Psens_adc, SENSE_PIN_2);
    ch3.setAdc(&Psens_adc, SENSE_PIN_3);
    ch4.setAdc(&Psens_adc, SENSE_PIN_4);
  #endif



    ch1.begin(15500,BIGBAG_SAFETY_PRESSURE_LIMIT-250,BIGBAG_SAFETY_PRESSURE_LIMIT,TIMEOUT); // lower and upper pressure vals are experimentally derived from Psens_adc wset to ads.setGain(GAIN_TWO);

    ch2.begin(15500,FLUTE_SAFETY_PRESSURE_LIMIT-250,FLUTE_SAFETY_PRESSURE_LIMIT, TIMEOUT);//(15500,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT-250,MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT);

    ch3.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT, TIMEOUT);//(15500,LENKRAD_SAFETY_PRESSURE_LIMIT-250,LENKRAD_SAFETY_PRESSURE_LIMIT);

    ch4.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT, TIMEOUT);
    //delay(10000);
    /*set hysteresis boundarys according to used air chamber size and flexibility
    the bigger the air chamber, the lower the values
    */
    ch1.setInertia(20,10);
    ch2.setInertia(20,10);
    ch3.setInertia(20,10);
    ch4.setInertia(20,10);


    /*//value range for displayed in GUI */
    // ch1.setGuiMappingRange(0,100);
    // ch2.setGuiMappingRange(0,100);
    // ch3.setGuiMappingRange(0,100);
    // ch4.setGuiMappingRange(0,100);

  //test valves

  //ch5P.setAdc(&PV_adc,3); //ADS1115_REG_CONFIG_MUX_SINGLE_3);
  //ch6P.setAdc(&PV_adc,2); //ADS1115_REG_CONFIG_MUX_SINGLE_2);
  //ch7P.setAdc(&PV_adc,0); //ADS1115_REG_CONFIG_MUX_SINGLE_0);



  #if ADS_L
    ch5P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_3);
    ch6P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_2);
    ch7P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_0);
  #else
    ch5P.setAdc(&PV_adc,3);
    ch6P.setAdc(&PV_adc,2);
    ch7P.setAdc(&PV_adc,0);
  #endif

  ch5P.setDac(&PV_dac, MCP4728_CHANNEL_D);
  ch6P.setDac(&PV_dac, MCP4728_CHANNEL_C);
  ch7P.setDac(&PV_dac, MCP4728_CHANNEL_A);


  /* ------------ begin and set pressure range and limits of the connected inflateble
  takes three arguemnts, Lower end of pressure, upper end, safety limit and pressure range in Bar of valve (default = 1).
  pressure value is experimentally derived and revers to voltage levels from PV_adc wset to ads.setGain(GAIN_TWO);
  */
  ch5P.begin(0,10000,10250); // BIGBAG CALIBRATED lower and upper pressure vals are experimentally derived from Psens_adc wset to ads.setGain(GAIN_TWO) (max is 32768);
  ch6P.begin(0,10250,10500);
  ch7P.begin(0,10250,10500,2);  // has fourth argument, to specify the higher pressure Range of the Valve in Bar
}

void loop() {
    //switch_neo_pixels.clear();
  //Serial.println(testADC.readADC());

    if (MANUAL_MODE) { /*________manual mode__________*/
      // set and apply pressure manually
        ch1.operate_manual();
        ch2.operate_manual();
        ch3.operate_manual();
        ch4.operate_manual();

        //read all potis and knobs of first 4 channels
        for (int i = 0;i<4;i++){
          knobVal[i] = vArray[i]->get_Poti();
          butVal[i]=vArray[i]->get_button();
        }

        /*page switcher. Got to page where last change happend*/
        for (int i=0;i<4;i++){
            if ((abs(knobVal[i]-old_knobVal[i])>10)||(butVal[i] != old_butVal[i])) page = i;
            old_knobVal[i] = knobVal[i];
            old_butVal[i] = butVal[i];
        }

        display(page);
        sendDataOverSerial(false); //argument true sets data for serial plotter

    }
    else
    { /*________sequencer mode__________*/
        /*steps through sequence by pressing button 1*/
//        if (digitalRead(BUTTON_1) == LOW) Serial.println("button1 down");
        //if ((Button_switcher.getFlag() && !button) || startup) { //||Button_1.get_buttonNow()
        if (((digitalRead(BUTTON_SWITCH) == LOW || digitalRead(BUTTON_1) == LOW) && !button) || startup) { //||Button_1.get_buttonNow()
            //digitalWrite(SWITCH_LED, HIGH);
            brightness = 255;
          for (int ledNr = 0; ledNr<NEO_SWITCH_NUM_PIXELS; ledNr++){

            switch_neo_pixels.setBrightness(brightness);
            switch_neo_pixels.setPixelColor(ledNr, switch_neo_pixels.Color(0, 255,255));
            switch_neo_pixels.show();
               // Send the updated pixel colors to the hardware.
          }
            //Serial.println("LIGHTS ON");
            ledOnTime = millis();
            startup = false; //makes sure pessures are defined if not risk of burst!!!
            trig = 1;
            button = true;
            lightsOut = false;
            count++;
            if (count >= SEQ_LNGTH) count = 0;

            /*--------------DEBUG MSG---------------*/
            // Serial.print("count: ");
            // Serial.println(count);


            ch5P.goToPressure(sequence[count].ch5_val);
            //ch7P.goToPressure(sequence[count].ch2_val);
            ch7P.goToPressure(0);
            //ch6P.goToPressure(0);
            ch6P.goToPressure(0);

            Serial.print(sequence[count].ch1_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch2_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch3_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch4_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch5_val);
            Serial.print(" ");
            Serial.println(sequence[count].ch6_val);
        }
        else if ((digitalRead(BUTTON_SWITCH) == HIGH && digitalRead(BUTTON_1) == HIGH)  && button && (millis()-ledOnTime >200)) { //&& !Button_1.get_buttonNow()
            button = false;
            lastLast = millis();
            showIt = true;


            //Button_switcher.clearFlag();
        }
        //---------Valve channels-------------
        if ( millis() - triggerMetro > 100 ){
          ch1.trigger(sequence[count].ch1_val, trig);
          ch2.trigger(sequence[count].ch2_val, trig);
          ch3.trigger(sequence[count].ch3_val, trig);
          ch4.trigger(sequence[count].ch4_val, trig);
          ch5P.operate();
          ch6P.operate();
          ch7P.operate();

          triggerMetro = millis();
          trig = 0;
          if (!fading) display_seq_4ch(count);
        }
    }


//-------------- button illumination -----------
    if (millis()-ledOnTime > 2500 && !lightsOut) {
    if (millis()-brightFadeTime > 2){
      fading = true;
        switch_neo_pixels.setBrightness(--brightness);
        switch_neo_pixels.show();   // Send the updated pixel colors to the hardware.
        // Serial.println(brightness);
        if (brightness == 0) {
          lightsOut = true;
          // Serial.println("lightsOut");
          fading = false;
        }
        brightFadeTime = millis();
      }
    }//digitalWrite(SWITCH_LED, LOW); // switch of LED

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


void sendDataOverSerial(boolean plotter){
static unsigned long lastSend = 0;

if (millis()-lastSend > 80){
    lastSend = millis();
    if (!plotter){
        Serial.print("p: ");
        Serial.print("\t");
      }
    Serial.print(ch1.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch2.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch3.get_MappedPressure());
    Serial.print("\t");
    Serial.print(ch4.get_MappedPressure());

    if (!plotter){
      Serial.print("\npr:");
      Serial.print("\t");
      Serial.print(ch1.get_Pressure());
      Serial.print("\t");
      Serial.print(ch2.get_Pressure());
      Serial.print("\t");
      Serial.print(ch3.get_Pressure());
      Serial.print("\t");
      Serial.print(ch4.get_Pressure());
      Serial.print("\nv: ");
      Serial.print("\t");
      Serial.print(ch1.get_MappedPoti());
      Serial.print("\t");
      Serial.print(ch2.get_MappedPoti());
      Serial.print("\t");
      Serial.print(ch3.get_MappedPoti());
      Serial.print("\t");
      Serial.print(ch4.get_MappedPoti());
      Serial.print("\n");
    }
    Serial.print("\n");
  }
}

void setFlagHandler(){
  if (!Button_switcher.getFlag()) Button_switcher.setFlag(); //
}


// spte sensors on ch1 - ch4:  1 bar ~ 0,5V
// veab valves on ch5 - ch 6:  1 bar ~ 10v -> spannungsteiler ->5V
// veab valve on ch7:          1 bar ~ 5v -> spannungsteiler -> 2.5V

// The ADC input range (or gain) can be changed via the following
// functions, but be careful never to exceed VDD +0.3V max, or to
// exceed the upper and lower limits if you adjust the input range!
// Setting these values incorrectly may destroy your ADC!
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
