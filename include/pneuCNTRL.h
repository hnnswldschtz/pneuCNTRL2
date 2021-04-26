#ifndef PNEU
#define PNEU

#include <Arduino.h>
#include <Adafruit_ADS1015.h>

void display(int);
void display_seq_3ch(int);
void display_seq(int);
void sendDataOverSerial();

class PneuChannel{

  public:
    PneuChannel(int, int, int, int, int );
    int begin(int, int, int);
    byte operate_manual();
    byte trigger(int,int);


    int get_Pressure();
    int get_MappedPressure();
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
