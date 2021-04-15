#ifndef PNEU
#define PNEU

#include <Arduino.h>
#include <Adafruit_ADS1015.h>


class PneuChannel{
  public:
    PneuChannel(int, int, int, int, int );
    byte operate_manual();
    byte trigger(int,int);
    int begin(int, int, int);
    void inflate();
    void deflate();
    void stop();
    int get_Pressure();
    int get_MappedPressure(int, int);
    int get_Poti();
    int get_MappedPoti(int, int);
    bool get_button();
    void setInertia(int, int);


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

    float filter(float, float, float);
    int readPressure();
    int readPotentiometer();
    void readButton(unsigned long);
    void emergency_watchdog(int);

  };
#endif
