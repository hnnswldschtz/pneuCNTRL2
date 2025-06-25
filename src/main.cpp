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

#define ADS_L false
#define HOW_MANY_CHANNELS 4
#define DEBUG false



// ++++++++++++++++++++++ SET HERE MODE OF OPERATION ++++++++++++++++++++++

#define MANUAL_MODE false // set to true to use manual mode, false to use sequencer mode
#define RANDOMIZE_SEQUENCE_START false // set to true to randomize the sequence start, false to start with first sequence step
#define SET_MODE true // set to 1 to use the set1 sequence, 0 to use the sequence array
#define TRAINING_MODE false // set to 1 to use the training mode, 0 to use the normal mode

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




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
ProportionalChannel ch8P;


int knobVal[4];
int old_knobVal[4];
int butVal[4];
int old_butVal[4];
//boolean knobTurned[4];
int page = 0;
int trig = 0;
int count = -1;
int set_index = 0; // used for set sequence indexing
int set_selector = 0; // used for set sequence selection

unsigned long triggerMetro;
boolean button = false;
boolean startup = true;
unsigned long lastButtonPressTime = 0;

unsigned long ledOnTime = 0;
boolean showIt = false;
boolean lightsOut = false;
unsigned long brightFadeTime = 0;
byte brightness;
boolean fading  = false;



#if SET_MODE
  int set1[] = {0, 4, 2, 3, 2, 1, 2, 4, 2, 1, 4, 0, 3, 0, 1, 0, 3, 0, 1, 3, 4};
  int set2[] = {0, 4, 3, 4, 1, 3, 1, 0, 2, 0, 3, 1, 2, 3, 4, 0, 1, 4, 2, 0, 2};
  int set3[] = {0, 2, 4, 3, 4, 2, 1, 2, 3, 4, 1, 0, 4, 3, 1, 0, 2, 3, 0, 1, 0};
  int* setList[] = { set1, set2, set3 };
  int setLengthList[] = { int((sizeof set1)/sizeof(*set1)), int((sizeof set2)/sizeof(*set2)), int((sizeof set3)/sizeof(*set3)) };
#else
  int set1[] = {0, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 0};
  int set2[] = {5, 6, 7, 8, 9, 8, 7, 6, 5, 6, 7, 8, 9, 8, 7, 6, 5, 6, 7, 8, 9, 8, 7, 6, 5, 6, 7, 8, 9, 8, 7, 6, 5};
  int* setList[] = { set1, set2};
  int setLengthList[] = { int((sizeof set1)/sizeof(*set1)), int((sizeof set2)/sizeof(*set2)) };
#endif

//3channel, double frequency
#if HOW_MANY_CHANNELS == 3
DATA_P dataPoint_1 = {40,  0,   0,  0,  0,  0, 0, 0}; // empty data point for ch5P
DATA_P dataPoint_2 = {40,  0,   0,  0,  0,  0, 99, 0}; // empty data point for ch5P
DATA_P dataPoint_3 = {40,  0,   0,  0,  99,  0, 99, 0}; // empty data point for ch5P
DATA_P dataPoint_4 = {40,  0,   0,  0,  99, 99, 99, 0}; // empty data point for ch5P
 
#elif HOW_MANY_CHANNELS == 4
// 4ch, double freuency 
DATA_P dataPoint_1 = {40,  0,   0,  0,  0,  0, 0, 0}; // empty data point for ch5P
DATA_P dataPoint_2 = {40,  0,   0,  0,  0,  0, 0, 90}; // empty data point for ch5P
DATA_P dataPoint_3 = {40,  0,   0,  0,  127,  0, 0, 90}; // empty data point for ch5P
DATA_P dataPoint_4 = {40,  0,   0,  0,  0, 127, 0, 90}; // empty data point for ch5P
DATA_P dataPoint_5 = {40,  0,   0,  0,  127, 127, 127, 90}; // empty data point for ch5P

// 4ch, increase area
DATA_P dataPoint_6 = {40,  0,   0,  0,  0,  0, 0, 0}; // empty data point for ch5P
DATA_P dataPoint_7 = {40,  0,   0,  0,  127,  0, 0, 0}; // empty data point for ch5P
DATA_P dataPoint_8 = {40,  0,   0,  0,  127,  127, 0, 0}; // empty data point for ch5P
DATA_P dataPoint_9 = {40,  0,   0,  0,  127, 127, 127, 0}; // empty data point for ch5P
DATA_P dataPoint_10 = {40,  0,   0,  0,  127, 127, 127, 90}; // empty data point for ch5P


//DATA_P sequence [] = {dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5 }; // double frequency
//DATA_P sequence [] = {dataPoint_6,dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10 }; //increase area 

//5x double frequency, 5x increase area
/*DATA_P sequence [] = {
  dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
  dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
  dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
  dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
  dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
  dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10,
  dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10,
  dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10,
  dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10,
  dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10
};
*/
//pingpong frequence sequence
// DATA_P sequence [] = {
//   dataPoint_1,dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5,
//   dataPoint_4,dataPoint_3, dataPoint_2, dataPoint_1, dataPoint_2,
//   dataPoint_3,dataPoint_4, dataPoint_5, dataPoint_4, dataPoint_3,
//   dataPoint_2,dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4,
//   dataPoint_5,dataPoint_4, dataPoint_3, dataPoint_2, dataPoint_2
// };

//pingpong sequence with increase area
// DATA_P sequence [] = {
//   dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10,
//   dataPoint_9, dataPoint_8, dataPoint_7, dataPoint_6, dataPoint_7,
//   dataPoint_8, dataPoint_9, dataPoint_10, dataPoint_9, dataPoint_8,
//   dataPoint_7, dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9,
//   dataPoint_10, dataPoint_9, dataPoint_8, dataPoint_7, dataPoint_6
// };

// sequence needed for set and rnd mode, 
DATA_P sequence [] = {dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4, dataPoint_5, dataPoint_6, dataPoint_7, dataPoint_8, dataPoint_9, dataPoint_10 };
#else
//3channel
DATA_P sequence [] = {dataPoint_1, dataPoint_2, dataPoint_3, dataPoint_4};
#endif


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
const int MEDIUM_NYLON_SURFACE_STRUCTURE_SAFETY_PRESSURE_LIMIT = 25000; //where safety kicks in
const int SPINACKER_STRUCTURE_SAFETY_PRESSURE_LIMIT = 19000;//where safety kicks in

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


  delay(1000); //war 2000
  if (!PV_dac.begin()) {
      Serial.println("Failed to find MCP4728 chip");
  }

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

    //VALVE CHANNELS
    //2025 3 CHANNELS Haptic Setup 
      //arguments:  min_pressure, max_pressure, safety_stop_pressure, timeout
    ch1.begin(15500,BIGBAG_SAFETY_PRESSURE_LIMIT-250,BIGBAG_SAFETY_PRESSURE_LIMIT,TIMEOUT); // lower and upper pressure vals are experimentally derived from Psens_adc wset to ads.setGain(GAIN_TWO);

    ch2.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT, TIMEOUT); //Haptic ch 1

    ch3.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT, TIMEOUT);  //Haptic ch 2

    ch4.begin(15500,BAGUETTE_SAFETY_PRESSURE_LIMIT-250,BAGUETTE_SAFETY_PRESSURE_LIMIT, TIMEOUT);  //Haptic ch 3
    //delay(10000);
    /*set hysteresis boundarys according to used air chamber size and flexibility
    the bigger the air chamber, the lower the values
    */
    ch1.setInertia(20,10);
    ch2.setInertia(20,10);
    ch3.setInertia(20,10);
    ch4.setInertia(20,10);




  #if ADS_L
    ch5P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_3);
    ch6P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_2);
    ch7P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_0);
    ch7P.setAdc(&PV_adc,ADS1115_REG_CONFIG_MUX_SINGLE_1);
  #else
    ch5P.setAdc(&PV_adc,3);
    ch6P.setAdc(&PV_adc,2);
    ch7P.setAdc(&PV_adc,0);
    ch8P.setAdc(&PV_adc,1);
  #endif

  ch5P.setDac(&PV_dac, MCP4728_CHANNEL_D);
  ch6P.setDac(&PV_dac, MCP4728_CHANNEL_C);
  ch7P.setDac(&PV_dac, MCP4728_CHANNEL_A);
  ch8P.setDac(&PV_dac, MCP4728_CHANNEL_B);


  /* ------------ begin and set pressure range and limits of the connected inflatable
  takes three arguemnts, Lower end of pressure, upper end, safety limit and pressure range in Bar of valve (default = 1).
  pressure value is experimentally derived and revers to voltage levels from PV_adc wset to ads.setGain(GAIN_TWO);
  */
  ch5P.begin(0,30000,30000); // BIGBAG CALIBRATED lower and upper pressure vals are experimentally derived from Psens_adc wset to ads.setGain(GAIN_TWO) (max is 32768);
  ch6P.begin(0,30000,30000);
  ch7P.begin(0,30000,30000);  // has fourth argument, to specify the higher pressure Range of the Valve in Bar
  ch8P.begin(0,30000,30000,2);  // has fourth argument, to specify the higher pressure Range of the Valve in Bar


  //add all values from each set in setlist and check if they are sane
  //set1 sanity check
for (int c = 0; c < int(sizeof(setList)/sizeof(*setList)); c++) {
  int setLength = setLengthList[c];
  int *set = setList[c];
  int sum = 0;
  for (int i = 0; i < setLength; i++) {
    sum += set[i];
  }
  Serial.print("Sum of set");
  Serial.print(c + 1);
  Serial.print(": ");
  Serial.println(sum);
  if (sum % 10 == 0) {
    Serial.println("Set is sane");
  } else {
    Serial.println("Set is not sane, please check your values");
  }
}



  //set the pressure range of the valves in GUI land
  ch5P.setGuiMappingRange(0,100);
  ch6P.setGuiMappingRange(0,100);
  ch7P.setGuiMappingRange(0,100);
  ch8P.setGuiMappingRange(0,100);

  //set the pressure range of the valves in GUI land
  for (int i=0;i<4;i++){
    vArray[i]->setGuiMappingRange(0,100);
  }

  lcd.clear();
  lcd.print("Ready");
  delay(500);
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
        // if (digitalRead(BUTTON_1) == LOW) Serial.println("button1 down");
        // if ((Button_switcher.getFlag() && !button) || startup) { //||Button_1.get_buttonNow()
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
            // When button is pressed:
            
            startup = false; //makes sure pessures are defined if not risk of burst!!!
            trig = 1;
            button = true;
            lightsOut = false;
          
            

            /*--------------DEBUG MSG---------------*/
            // Serial.print("count: ");
            // Serial.println(count);
          
            if (RANDOMIZE_SEQUENCE_START) { //randomize sequence start
  
              int oldCount = count;
              while (count == oldCount) { //randomize sequence start
                #if HOW_MANY_CHANNELS == 4
                  count=random(0,5); // randomize sequence start
                #elif HOW_MANY_CHANNELS == 3
                  count=random(0,4); // randomize sequence start
                #endif
                Serial.print("count: ");
                Serial.println(count);
                Serial.print("oldCount: ");
                Serial.println(oldCount);
              } 
            }

            else if (SET_MODE) {
              int *setListPtr = setList[set_selector];
              count = setListPtr[set_index]; //set to first sequence step
              // count = setList[set_selector][set_index]; //set to first sequence step
            }

            else {
              if (count >= SEQ_LNGTH) count = 0;
            }
            unsigned long timeSinceLastButton = millis() - lastButtonPressTime;
     
            ch5P.goToPressure(sequence[count].ch5_val);
            ch6P.goToPressure(sequence[count].ch6_val);
            ch7P.goToPressure(sequence[count].ch7_val);
            ch8P.goToPressure(sequence[count].ch8_val);    

            //SERIAL OUTPUT for experimental data collection
            
            char strBuf[10]; // buffer for sprintf

            sprintf(strBuf, "%2d", set_index);
            Serial.print(strBuf);
            Serial.print(" | "); 
            
            sprintf(strBuf, "%5d", int(timeSinceLastButton));
            Serial.print("time (ms): ");
            Serial.print(strBuf);
            Serial.print(" | "); 
            
            sprintf(strBuf, "%3d", count);
            Serial.print(strBuf);
            Serial.print(" | ");

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
            Serial.print(sequence[count].ch6_val);
            Serial.print(" ");
            Serial.print(sequence[count].ch7_val);
            Serial.print(" ");
            Serial.println(sequence[count].ch8_val);
  
            //increment set_index for count or next sequence step
            if (SET_MODE) {
              set_index++; //set to first sequence step
              if (set_index >= setLengthList[set_selector]) {
                set_index = 0; //reset to first sequence step
                set_selector++; //increment set counter
                if (set_selector >= int(sizeof(setList)/sizeof(*setList))) { //reset to first set
                  set_selector = 0;
                }
              }
            }
            else if(RANDOMIZE_SEQUENCE_START) {
              count++;
              if (count >= SEQ_LNGTH) count = 0;
            }
            lastButtonPressTime = millis();
        }
        else if ((digitalRead(BUTTON_SWITCH) == HIGH && digitalRead(BUTTON_1) == HIGH)  && button && (millis()-ledOnTime >200)) { //&& !Button_1.get_buttonNow()
            button = false;
            lastLast = millis();
            showIt = true;
            //Button_switcher.clearFlag();
        }
        //operate the valves
        
        if ( millis() - triggerMetro > 100 ){
          //---------Valve channels-------------
          ch1.trigger(sequence[count].ch1_val, trig);
          ch2.trigger(sequence[count].ch2_val, trig);
          ch3.trigger(sequence[count].ch3_val, trig);
          ch4.trigger(sequence[count].ch4_val, trig);
          //---------Proportional channels-------------
          ch5P.operate();
          ch6P.operate();
          ch7P.operate();
          ch8P.operate();

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

// void setFlagHandler(){
//   if (!Button_switcher.getFlag()) Button_switcher.setFlag(); //
// }


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
