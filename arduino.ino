#define SERIAL_BAUD 9600

/*******************************************
	In this code we have used "[]" to surround our command codes
	As a bit of a proof of concept for how to use the XC4411 board
*********************************************/

int current;
int vol;

int watts;

int inverter1pin;
int inverter2pin;
int chargepin;


int invStart1;
int invStart2;
int invEnd;
int chargeStart;
int chargeEnd;
int minutesAfterMidnight;
int starttime;


void turnOff(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
void turnOn(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}


int setValue(String msg, int index, int value)
{
  int nl = msg.length();
  int v = msg.substring(index).toInt();
  pinMode(LED_BUILTIN , OUTPUT);
  digitalWrite(LED_BUILTIN , HIGH);
  delay(1000);
  if(v > 0) {
    delay(3000);
  } else {
    v = value;
  }
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  return v;
}

void setup()
{

  current = A0;
  vol     = A1;

  watts       = 0;

  inverter1pin  = 4;
  inverter2pin  = 5;
  chargepin     = 6;


  invStart1 = 360;
  invStart2 = 540;
  invEnd   = 1200;
  chargeStart = 1200;
  chargeEnd   = 360;
  minutesAfterMidnight = 0;
  starttime = millis()/1000;

  Serial.begin(SERIAL_BAUD); //same as ESP baud
  delay(2000); //get all information on serial
  pinMode(inverter1pin, OUTPUT); 
  pinMode(inverter2pin, OUTPUT); 
  pinMode(chargepin, OUTPUT);
  turnOff(inverter1pin);
  turnOff(inverter2pin);
  turnOff(chargepin);
}

void loop()
{
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil(']'); //read it
    if (message.indexOf('[') >= 0) {
      int pos = message.indexOf('[');
      switch (message.charAt(pos + 1)) {
        case '0':
          minutesAfterMidnight =  setValue(message, pos + 3, minutesAfterMidnight);
          break;
        case '1':
          invStart1 =  setValue(message, pos + 3, invStart1);
          break;
        case '2':
          invStart2 = setValue(message, pos + 3,invStart2);
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

  if(millis()/1000 > starttime + 120)  {
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
