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
#ifndef PNEU
#define PNEU

#include <Arduino.h>
#include <Wire.h>
//#include <Adafruit_ADS1015.h>
#include <Adafruit_ADS1X15.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MCP4728.h>
#include "Channels.h"
#include <Adafruit_NeoPixel.h>
//#include <LiquidCrystalFast.h>
#include <ADS1115_lite.h>
#include "Button.h"




void display(int);
void display_seq_4ch(int);
void display_seq(int);
void sendDataOverSerial(boolean);
void setFlagHandler();


struct DATA_P {
  int ch1_val;
  int ch2_val;
  int ch3_val;
  int ch4_val;
  int ch5_val;
  int ch6_val;
  int ch7_val;
};

struct SEQ_CH {
  int s1_val;
  int s2_val;
  int s3_val;
  int s4_val;
  int s5_val;
  int s6_val;
  int s7_val;
};

const int PCB_PORT1  =  3;
const int PCB_PORT2  =  4;
const int PCB_PORT3  =  5;
const int PCB_PORT4  =  6;
const int PCB_PORT5  =  20;
const int PCB_PORT6  =  21;
const int PCB_PORT7  =  22;
const int PCB_PORT8  =  23;


#define valve_A1 PCB_PORT4
#define valve_A2 PCB_PORT5
#define valve_B3 PCB_PORT3
#define valve_B4 PCB_PORT6
#define valve_C5 PCB_PORT2
#define valve_C6 PCB_PORT7
#define valve_D7 PCB_PORT1
#define valve_D8 PCB_PORT8


#define POTI_1 A0
#define POTI_2 A1
#define POTI_3 A2
#define POTI_4 A3

#define BUTTON_1 24
#define BUTTON_2 25
#define BUTTON_3 26
#define BUTTON_4 27
#define BUTTON_SWITCH 7
/*LED OF SWITCH*/
//#define SWITCH_LED 2

const int SENSE_PIN_1 = 0;
const int SENSE_PIN_2 = 1;
const int SENSE_PIN_3 = 2;
const int SENSE_PIN_4 = 3;

// NeoPixel setup
#define NEO_SWITCH_PIN 2
#define NEO_SWITCH_NUM_PIXELS 5 // how many leds in switchbox?

 

#endif
