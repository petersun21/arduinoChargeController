#define SERIAL_BAUD 9600

/*******************************************
	In this code we have used "[]" to surround our command codes
	As a bit of a proof of concept for how to use the XC4411 board
*********************************************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    8  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC    9
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void setupTFT() {
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
  tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
}

int chargeStart;
int chargeEnd;
int InvStart;
int InvEnd;

void setupTimes() {
  chargeStart = 1200;
  chargeEnd   = 360;
  InvStart    = 360;
  invEnd      = 1200;
}

int invPins[5];
int invState[5];
int inverterWatts[4];
int hfeLowSwitch;  //LOW -- off- HIGH - on
int minutesAfterMidnight;
int LastInternetTime;

int currentPin;
int voltagePin;
int startTime;

void turnOn(int pinNdx) {
  pinMode(invPins[pinNdx], OUTPUT);
  pinMode(hfeLowSwitch, OUTPUT);
  digitalWrite((hfeLowSwitch, HIGH);
  digitalWrite（invPins[pinNdx], HIGH);
  invState[pinNdx] = HIGH;
  delay(300);
  digitalWrite(invPins[pinNdx], LOW);
  digitalWrite((hfeLowSwitch, LOW);
}

void turnOff(int pinNdx) {
  pinMode(invPins[pinNdx], OUTPUT);
  pinMode(hfeLowSwitch, OUTPUT);
  digitalWrite((hfeLowSwitch, LOW);
  digitalWrite（invPins[pinNdx], HIGH);
  invState[pinNdx] = LOW;
  delay(300);
  digitalWrite(invPins[pinNdx], LOW);
}

int setValue(String msg, int index, int value)
{
  int nl = msg.length();
  int v = msg.substring(index).toInt();
  pinMode(LED_BUILTIN , OUTPUT);
  digitalWrite(LED_BUILTIN , HIGH);
  delay(500);
  if(v > 0) {
    delay(1500);
  } else {
    v = value;
  }
  digitalWrite(LED_BUILTIN, LOW);
  return v;
}

void setupRelays() {
  hfeLowSwitch = 6;
  for(int i = 0; i < 5; i++) {
    invPins[i] = i;
    invState[i] = LOW;
    hfeLowSwitch = LOW;
    if(i < 2) {
      inverterWatts[i] = 290;
    } else if(i > 1 && i < 4) {
      inverterWatts[i] = 380;
    }
  } 
}

setupAnalog() {
  currentPin = A0;
  voltagePin = A1;
}



void setup()
{
  startTime = millis()/60000;
  setupRelays();
  setupAnalog();
  setupTimes();
  LastInternetTime = -1;
  setupTFT();
  Serial.begin(SERIAL_BAUD); //same as ESP baud
  delay(2000); //get all information on serial

}

int analogRead() {
  float current = 0;
  float voltage = 0;
  float watts   = 0;
  
  for(int i = 0; i < 1200; i++) {
     float c = 5.0 * analogRead(currentPin) / (float) 1024;
     float v = 5.0 * analogRead(voltagePin) / (float) 1024;
     watts += c*v;
     current += c;
     voltage += v;
  }
  watts -= current * voltage / (1200 * 1200);
  current = current * 5.0/200.0;
  return current * 248 * watts / abs(watts);
}

int invStart = 360;
int invEnd   = 1200;
int chargeStart = 1200;
int chargeEnd   = 360;
watts = 0;

void loop()
{
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil(']'); //read it
    if (message.indexOf('[') >= 0) {
      int pos = message.indexOf('[');
      switch (message.charAt(pos + 1)) {
        case '0':
          minutesAfterMidnight =  setValue(message, pos + 3, minutesAfterMidnight);
          LastInternetTime = millis()/1000/60;
          break;
        case '1':
          invStart =  setValue(message, pos + 3, invStart1);
          break;
        case '3':
         invEnd = setValue(message, pos + 3, invEnd);
          break;
        case '4':
          chargeStart = setValue(message, pos + 3, chargeStart);
          break;
        case '5':
          chargeEnd = setValue(message, pos + 3, chargeEnd);
          break;        
        default:
         break;
      }
    }
  }
  unsigned long currTime = millis()/60000;
  if(currTime > starttime + 2)  {
    if(invStart1 < invEnd) {
      if( invStart1 <= minutesAfterMidnight && minutesAfterMidnight < invEnd) {
        turnOn(inverter1pin);
      } else {
        turnOff(inverter1pin);
      }
    } else {
      if(invStart1 < minutesAfterMidnight || invEnd > minutesAfterMidnight) {
         turnOn(inverter1pin);
      } else {
        turnOff(inverter1pin);     
      }
    }
    if(invStart2 < invEnd) {
      if( invStart2 <= minutesAfterMidnight && minutesAfterMidnight < invEnd) {
        turnOn(inverter2pin);
      } else {
        turnOff(inverter2pin);
      }
    } else {
      if(invStart2 < minutesAfterMidnight || invEnd > minutesAfterMidnight) {
         turnOn(inverter2pin);
      } else {
        turnOff(inverter2pin);     
      }
    }
    if(chargeStart < chargeEnd) {
      if( chargeStart <= minutesAfterMidnight && minutesAfterMidnight < chargeEnd) {
        turnOn(chargepin);
      } else {
        turnOff(chargepin);
      }
    } else {
      if(chargeStart < minutesAfterMidnight || chargeEnd > minutesAfterMidnight) {
         turnOn(chargepin);
      } else {
        turnOff(chargepin);     
      }
    }
  }
}
