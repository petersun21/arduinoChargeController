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

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen (ST7735_BLACK);
}

int chargeStart;
int chargeEnd;
int invStart;
int invEnd;

int invPins[5];
bool invState[5];
int inverterWatts[4];
int hfeLowSwitch;  //LOW -- off- HIGH - on
int minutesAfterMidnight;
int LastInternetTime;

int currentPin;
int voltagePin;
int startTime;
float  watts;
float cur;

void setupTimes() {
  chargeStart = 1200;
  chargeEnd   = 360;
  invStart    = 360;
  invEnd      = 1200;
}

void turnOn(int pinNdx) {
  pinMode(invPins[pinNdx], OUTPUT);
  pinMode(hfeLowSwitch, OUTPUT);
  digitalWrite(hfeLowSwitch, HIGH);
  digitalWrite(invPins[pinNdx], HIGH);
  invState[pinNdx] = HIGH;
  delay(300);
  digitalWrite(invPins[pinNdx], LOW);
  digitalWrite(hfeLowSwitch, LOW);
}

void turnOff(int pinNdx) {
  pinMode(invPins[pinNdx], OUTPUT);
  pinMode(hfeLowSwitch, OUTPUT);
  digitalWrite(hfeLowSwitch, LOW);
  digitalWrite(invPins[pinNdx], HIGH);
  invState[pinNdx] = false;
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
  if (v > 0) {
    delay(1500);
  } else {
    v = value;
  }
  digitalWrite(LED_BUILTIN, LOW);
  return v;
}

void setupRelays() {
  hfeLowSwitch = 6;
  for (int i = 0; i < 5; i++) {
    invPins[i] = i;
    turnOff(i);
    if (i < 2) {
      inverterWatts[i] = 290;
    } else if (i > 1 && i < 4) {
      inverterWatts[i] = 380;
    }
  }
}

void setupAnalog() {
  currentPin = A0;
  voltagePin = A1;
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}



void setup()
{
  startTime = millis() / 1000;
  setupRelays();
  setupAnalog();
  setupTimes();
  LastInternetTime = -1;
  setupTFT();
  Serial.begin(SERIAL_BAUD); //same as ESP baud
  delay(2000); //get all information on serial
  Serial.println("done setup");
  minutesAfterMidnight = 0;
}

int sort_desc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

float readWatts() {
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  float current = 0;
  float voltage = 0;
  watts = 0.0;
  float cum   = 0.0;

  float maxc = -5;
  float minc = +5;
 
  for (int i = 0; i < 1200; i++) {
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);

    unsigned long mis = micros();
    float c = 0.0;
    float v = 0.0;
  
    for(int j = 0; j < 4; j++) {
       c  += (float) (analogRead(A0) ;
       v +=  (float) analogRead(A1);
    }
   
    c /= 4;
    v /= 4;
    
    v = 5 * v /  1023.0;
    c = 5 * c /  1023.0;

    if(c < minc) {
      minc = c;
    }
    if(c > maxc) {
      maxc = c;
    }

   
    watts += c * v;
    current += c;
    cum     += c * c;
    voltage += v;
    unsigned long t = micros();
 
    while (t - mis < 1667 && t - mis > 0) {
      t = micros();
    }
    mis = micros();
  }

  tft.setTextSize(1.5);
  maxc = maxc * 200 / 3.0;
  minc = minc * 200 / 3.0;
  display_rec(50, 120, 71, 15,"minc: " + String(minc));
  display_rec(50, 135, 71, 15,"maxc: " + String(maxc));
  watts = watts / 1200;
  current = current / 1200;
  voltage = voltage / 1200;
  cum = cum / 1200;
  watts -= current * voltage;
  current = sqrt(cum - current * current);
  
  current = current * 200.0 / 3;
  watts =  current * 248 * watts / abs(watts);
  return current;
}

void display_rec(int x, int y, int x_inc, int y_inc, String txt) {
  tft.fillRect (x, y, x_inc - 1, y_inc - 1, ST7735_BLACK);
  tft.setCursor(x, y);
  tft.println(txt);
}

void tft_display() {


  int x = 5;
  int y = 25;

  tft.setTextSize(1.5);
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextWrap(true);
  for (int i = 0; i < 4; i++) {
    if (invState[i] == true) {
      display_rec(x, y + i * 25, 40, 25, "ON ");
    } else {
      display_rec(x, y + i * 25, 40, 25, "OFF");
    }
  }

  tft.setTextColor(ST77XX_RED);

  if (invState[5] == true) {
    display_rec(5, 125, 45, 25, "ON ");
  } else {
    display_rec(5, 125, 45, 25, "OFF");
  }
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1.5);
  display_rec(35, 25, 82, 20, "Watts: " + String(watts, 2));
  tft.setTextSize(1.5);
  display_rec(50, 45, 71, 15, "Curr:  " + String(cur, 1));
  display_rec(50, 60, 71, 15, "Mid:   " + String(minutesAfterMidnight / 60) + ":" + String(minutesAfterMidnight % 60));
  display_rec(50, 75, 71, 15, "LInt:  " + String(LastInternetTime));
  display_rec(50, 90, 71, 15,"InvSt: " +  timeString(invStart));
  display_rec(50, 105, 71, 15,"InvEd: " + timeString(invEnd));
}

String timeString(int t) {
  return String(t / 60) + ":" + String(t % 60);
}

void esp8266() {
  String message = Serial.readStringUntil(']'); //read it
  if (message.indexOf('[') >= 0) {
    int pos = message.indexOf('[');
    switch (message.charAt(pos + 1)) {
      case '0':
        minutesAfterMidnight =  setValue(message, pos + 3, minutesAfterMidnight);
        LastInternetTime = millis() / 60000;
        break;
      case '1':
        invStart =  setValue(message, pos + 3, invStart);
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

bool  checkInverterUsage( unsigned long currTime) {
  if ((currTime / 60 <= startTime / 60 + 2) || (LastInternetTime <= 0)) return false;
  minutesAfterMidnight = (minutesAfterMidnight + currTime / 60 - LastInternetTime) % 1440;
  if (invStart < invEnd) {
    if ( invStart <= minutesAfterMidnight && minutesAfterMidnight < invEnd) {
      return true;
    }
  } else if (invStart < minutesAfterMidnight || invEnd > minutesAfterMidnight) {
    return true;
  }

  return false;
}

bool checkChargingUsage(unsigned long currTime) {
  if ((currTime / 60 <= startTime / 60 + 2) || (LastInternetTime <= 0)) return false;
  if (chargeStart < chargeEnd) {
    if ( chargeStart <= minutesAfterMidnight && minutesAfterMidnight < chargeEnd) {
      return true;
    }
  } else {
    if (chargeStart < minutesAfterMidnight || chargeEnd > minutesAfterMidnight) {
      return true;
    }
  }
  return false;
}


void loop()
{
  while (Serial.available() > 0) {
    esp8266();
  }
  
  unsigned long currTime = millis() / 1000;
  bool useInverters = checkInverterUsage(currTime);
  bool charging     = checkChargingUsage(currTime);

  cur = readWatts();
  if (useInverters) {
    if (watts > 290) {
      float remainWatts = watts;
      bool toLook = true;
      while (remainWatts > 290 && toLook) {
        toLook = false;
        for (int i = 0; i < 4; i++) {
          if (invState[i] == false) {
            turnOn(i);
            remainWatts -=  inverterWatts[i];
            toLook = true;
            break;
          }
        }
      }
      //look for opportunity to invert
    } else if (watts < -100) {
      float remainWatts = watts;
      bool toLook = true;
      while (remainWatts < -100 && toLook) {
        toLook = false;
        for (int i = 0; i < 4; i++) {
          if (invState[i] == true) {
            turnOff(i);
            remainWatts +=  inverterWatts[i];
            toLook = true;
            break;
          }
        }
      }
    } else if (watts < -100) {
      for (int i = 0; i < 4; i++) {
        turnOff(i);
      }
    }
  }
  tft_display();
}
