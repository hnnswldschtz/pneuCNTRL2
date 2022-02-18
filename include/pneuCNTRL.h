#ifndef PNEU
#define PNEU

#include <Arduino.h>
#include <Adafruit_ADS1015.h>
#include <LiquidCrystal_I2C.h>


void display(int);
void display_seq_4ch(int);
void display_seq(int);
void sendDataOverSerial();


struct DATA_P {
  int ch1_val;
  int ch2_val;
  int ch3_val;
  int ch4_val;
};

#define valve_A1 4
#define valve_A2 8
#define valve_B3 5
#define valve_B4 10
#define valve_C5 6
#define valve_C6 11
#define valve_D7 7
#define valve_D8 12

const int SENSE_PIN_1 = 0;
const int SENSE_PIN_2 = 1;
const int SENSE_PIN_3 = 2;
const int SENSE_PIN_4 = 3;

class ValveChannel{

  public:
    ValveChannel(int, int, int, int, int );
    int begin(int, int, int);
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
    bool get_buttonNow(unsigned long d=30);
    void setInertia(int, int);
    void setMappingBoundaries(int ,int);



  private:
    Adafruit_ADS1115 ads;
    int infValvePin;
    int defValvePin;
    byte adcPort;
    int potPin;
    int buttonPin;
    int pressure;
    int old_p_set;

    int minPressure;
    int maxPressure;
    int safetyStop;
    int inflationInertia;
    int deflationInertia;
    int potiValMapped;
    int potiVal;
    int pMapMin;
    int pMapMax;

    long lastFill;
    bool inFlate;
    bool deFlate;
    long inFlate_time;
    long deFlate_time;
    long lastDebounceTime;
    bool lastFlickerState;
    bool buttonState;
    bool lastButtonState;
    bool buttonPressed;


    void emergency_watchdog(int);
    void inflate();
    void deflate();
    void stop();
    void readButton(unsigned long);
    int readPressure();
    int readPotentiometer();
    float filter(float, float, float);
    int value_mapper (int, int, int);

  };

  class ProportionalChannel{

    public:
      ProportionalChannel(int, int, int, int, int );
      int begin(int, int, int);
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
      bool get_buttonNow(unsigned long d=30);
      void setInertia(int, int);
      void setMappingBoundaries(int ,int);



    private:
      Adafruit_ADS1115 ads;
      int infValvePin;
      int defValvePin;
      byte adcPort;
      int potPin;
      int buttonPin;
      int pressure;
      int old_p_set;

      int minPressure;
      int maxPressure;
      int safetyStop;
      int inflationInertia;
      int deflationInertia;
      int potiValMapped;
      int potiVal;
      int pMapMin;
      int pMapMax;

      long lastFill;
      bool inFlate;
      bool deFlate;
      long inFlate_time;
      long deFlate_time;
      long lastDebounceTime;
      bool lastFlickerState;
      bool buttonState;
      bool lastButtonState;
      bool buttonPressed;


      void emergency_watchdog(int);
      void inflate();
      void deflate();
      void stop();
      void readButton(unsigned long);
      int readPressure();
      int readPotentiometer();
      float filter(float, float, float);
      int value_mapper (int, int, int);

    };
#endif
