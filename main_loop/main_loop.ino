#include <Arduino.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
 
#define SET            A0
#define PLUS           A1
#define MINUS          A2

#define BUTTON1        11
#define IN1            8
#define IN2            7
#define LED1           12

#define BUTTON2        9
#define IN3            6
#define IN4            5
#define LED2           10


RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

enum PumpID {
    PUMP1,
    PUMP2
};

// Settings
// scheduling
const unsigned long onByButtonTimer = 10000; // 10 sec in milliseconds
const unsigned long offByButtonTimer = 10000; // 10sec in milliseconds

const int P1turnOnHH = 1;
const int P1turnOffHH = 2;
const bool P1turnOnMonths[12] = {false, false, false, true, true, true, true, true, true, false, false, false};
const int P1skipDaysPerMonth[12]= {0, 0, 0, 5, 1, 0, 0, 0, 1, 0, 0, 0};

const int P2turnOnHH = 1;
const int P2turnOffHH = 2;
const bool P2turnOnMonths[12] = {false, false, false, true, true, true, true, true, true, false, false, false};
const int P2skipDaysPerMonth[12]= {0, 0, 0, 5, 2, 1, 1, 1, 2, 0, 0, 0};

// delay for settings button (set date)
const int  setModeTime=2000;


// Initialize state
// pumps
bool P1isOn = false;
unsigned long P1buttonPressRecentUntil = 0;
unsigned long P1turnedOffAt = 0;

bool P2isOn = false;
unsigned long P2buttonPressRecentUntil = 0;
unsigned long P2turnedOffAt = 0;

// date
char buffer[10];
unsigned long time=0;
unsigned long timeSet=0;
float temp=0;
//DateTime currentTime;


void setup() {
    Serial.begin(9600);
    Wire.begin();
    rtc.begin();
    lcd.begin();
    // Initialize the RTC module
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(SET, INPUT);
    pinMode(PLUS, INPUT);
    pinMode(MINUS, INPUT);
    
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode (BUTTON1, INPUT_PULLUP);
    pinMode (BUTTON2, INPUT_PULLUP);
    
    digitalWrite (IN1,LOW);
    digitalWrite (IN2,LOW);
    digitalWrite (IN3,LOW);
    digitalWrite (IN4,LOW);

}

DateTime now() {
    return rtc.now();
}


bool IsPumpOn(PumpID pump) {
    if (pump == PUMP1) {
        return P1isOn;
    } else if (pump == PUMP2) {
        return P2isOn;
    }
    return false;
}

bool IsItTimeToBeOn(PumpID pump) {
    int turnOnHH, turnOffHH;
    const int* skipDaysPerMonth;
    const bool* turnOnMonths;

    unsigned long turnedOffAt;

    if (pump == PUMP1) {
        turnOnHH = P1turnOnHH;
        turnOffHH = P1turnOffHH;
        turnOnMonths = P1turnOnMonths;
        skipDaysPerMonth = P1skipDaysPerMonth;
        turnedOffAt = P1turnedOffAt;
    } else if (pump == PUMP2) {
        turnOnHH = P2turnOnHH;
        turnOffHH = P2turnOffHH;
        turnOnMonths = P2turnOnMonths;
        skipDaysPerMonth = P2skipDaysPerMonth;
        turnedOffAt = P2turnedOffAt;
    } else {
        return false;
    }

  int hh = now().hour();
  int mm = now().month();
  if (turnOnMonths[mm-1] == true ){
    if (hh >= turnOnHH && hh < turnOffHH) {
      unsigned long skippedDays = (now().unixtime() - turnedOffAt) / 86400;
      return skippedDays >= skipDaysPerMonth[mm-1];
    }
  }
    
  return false;
}

bool IsButtonPressed(PumpID pump) {
      if (pump == PUMP1) {
       if (digitalRead (BUTTON1) == 0) {
      return true;
        } 
    return false;
      }
    if (pump == PUMP2) {
       if (digitalRead (BUTTON2) == 0) {
      return true;
        } 
    return false;
      }
}

bool WasButtonPressedRecently(PumpID pump) {
    unsigned long recencyThreshold = (pump == PUMP1) ? P1buttonPressRecentUntil : P2buttonPressRecentUntil;
    return now().unixtime() <= recencyThreshold;
}

void TurnOnPump(PumpID pump, bool triggeredByButton) {
    if (pump == PUMP1) {
        digitalWrite(IN1, HIGH);  // Imposta il pin IN1 HIGH 
        digitalWrite(IN2, LOW);   // Imposta il pin IN2 LOW 
        delay(2000);
        digitalWrite(IN1, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN2, LOW);   // Imposta il pin IN2 LOW
        P1isOn = true;
    } else if (pump == PUMP2) {
        // TODO: Add code to turn on P2
        digitalWrite(IN3, HIGH);  // Imposta il pin IN1 HIGH 
        digitalWrite(IN4, LOW);   // Imposta il pin IN2 LOW 
        delay(2000);
        digitalWrite(IN3, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN4, LOW);   // Imposta il pin IN2 LOW
        P2isOn = true;

    }

    if (triggeredByButton) {
        if (pump == PUMP1) {
            P1buttonPressRecentUntil = now().unixtime() + onByButtonTimer;  
        } else if (pump == PUMP2) {
            P2buttonPressRecentUntil = now().unixtime() + onByButtonTimer;
        }
    }
}

void TurnOffPump(PumpID pump, bool triggeredByButton) {
    if (pump == PUMP1) {
        // TODO: Add code to turn off P1
        digitalWrite(IN1, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN2, HIGH);   // Imposta il pin IN2 HIGH 
        delay(2000);
        digitalWrite(IN1, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN2, LOW);   // Imposta il pin IN2 LOW
        P1isOn = false;
        P1turnedOffAt = now().unixtime();
        Serial.print("TurnOffPump");
        Serial.println(pump);
    } else if (pump == PUMP2) {
        // TODO: Add code to turn off P2
        digitalWrite(IN3, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN4, HIGH);   // Imposta il pin IN2 HIGH 
        delay(2000);
        digitalWrite(IN3, LOW);   // Imposta il pin IN1 LOW 
        digitalWrite(IN4, LOW);   // Imposta il pin IN2 LOW 
        P2isOn = false;
        P2turnedOffAt = now().unixtime();
        Serial.print("TurnOffPump");
        Serial.println(pump);
    }

    if (triggeredByButton) {    
        if (pump == PUMP1) {
            P1buttonPressRecentUntil = now().unixtime() + offByButtonTimer;
        } else if (pump == PUMP2) {
           P2buttonPressRecentUntil = now().unixtime() + offByButtonTimer;   
        }
    }
}


void setMode(DateTime now_) {
    boolean setMode = true;
    int setModeLevel = 0;
 
    int _day = now_.day();
    int _month = now_.month();
    int _year = now_.year();
    int _hour = now_.hour();
    int _min = now_.minute();
    int _sec = now_.second();
 
    lcd.clear();
    lcd.setCursor(0,0);
    sprintf(buffer,  "%s: %02d", "Giorno", _day);
    delay( 1000 );
    timeSet = millis();
 
    while ( setMode ) {
      if ( analogRead(SET) > 1000 || analogRead(PLUS) > 1000 || analogRead(MINUS) > 1000 ) { timeSet = millis(); }
 
      lcd.setCursor(0,0);
 
      // Set Day
      if ( setModeLevel == 0 ) {
        if ( analogRead(PLUS) > 1000 && _day < 31) { _day++; }
        if ( analogRead(MINUS) > 1000 && _day > 1) { _day--; }
 
        sprintf(buffer,  "%s: %02d", "Giorno", _day);
      }
      // Set Month
      if ( setModeLevel == 1 ) {
        if ( analogRead(PLUS) > 1000 && _month < 12) { _month++; }
        if ( analogRead(MINUS) > 1000 && _month > 1) { _month--; }
 
        sprintf(buffer,  "%s: %02d", "Mese", _month);
      }
      // Set Year
      if ( setModeLevel == 2 ) {
        if ( analogRead(PLUS) > 1000 && _year < 9999) { _year++; }
        if ( analogRead(MINUS) > 1000 && _year > 1900) { _year--; }
 
        sprintf(buffer,  "%s: %02d", "Anno", _year);
      }
      // Set Hour
      if ( setModeLevel == 3 ) {
        if ( analogRead(PLUS) > 1000 && _hour < 24) { _hour++; }
        if ( analogRead(MINUS) > 1000 && _hour > 1) { _hour--; }
 
        sprintf(buffer,  "%s: %02d", "Ora", _hour);
      }
      // Set Minute
      if ( setModeLevel == 4 ) {
        if ( analogRead(PLUS) > 1000 && _min < 60) { _min++; }
        if ( analogRead(MINUS) > 1000 && _min > 1) { _min--; }
 
        sprintf(buffer,  "%s: %02d", "Minuti", _min);
      }
      // Set Second
      if ( setModeLevel == 5 ) {
        if ( analogRead(PLUS) > 1000 && _sec < 60) { _sec++; }
        if ( analogRead(MINUS) > 1000 && _sec > 0) { _sec--; }
 
        sprintf(buffer,  "%s: %02d", "Secondi", _sec);
      }
 
      lcd.print( buffer );
      if ( analogRead(SET) > 1000 ) { lcd.clear(); setModeLevel++;  }
      if ( setModeLevel > 5 ) { setModeLevel=0; }
 
      if (timeSet > 0 && (setModeTime*2) < (millis() - timeSet) ) {
         rtc.adjust(DateTime(_year, _month, _day, _hour, _min, _sec));
         setMode = false;
      }
      delay(200);
    }
}


void refreshScreen() {
  
    DateTime now_ = now();
    lcd.clear();

    sprintf(buffer,  "%02d/%02d/%d", now_.day(), now_.month(), now_.year());
    lcd.setCursor(0,0);
    lcd.print(buffer);
    
    char buffer[10] = "";
 
    sprintf(buffer,  "%02d:%02d:%02d", now_.hour(), now_.minute(), now_.second());
   
    lcd.setCursor(0,1);
    lcd.print(buffer);   
   
}

bool isItTimeToSetTime() {
    if (time > 0 && setModeTime < (millis() - time) ) { 
        return true;
    } else {
        return false;
    }  
}


void loop() {

    refreshScreen();
   
    if (analogRead (SET) < 1000) { time = millis();}

    if (isItTimeToSetTime()) {
        setMode(now()); // note that the "now" would be wrong when this is called
    }

    PumpID pumps[] = {PUMP1, PUMP2};
    for (PumpID pump : pumps) {
        bool pumpIsOn = IsPumpOn(pump);
        bool itsTimeToBeOn = IsItTimeToBeOn(pump);
        bool buttonIsPressed = false; //according Puglierin request
        bool buttonPressedRecently = WasButtonPressedRecently(pump);
        
        if (!pumpIsOn) {
            if (buttonIsPressed) {
                TurnOnPump(pump, true);
            } else if (itsTimeToBeOn && !buttonPressedRecently) {
                TurnOnPump(pump, false);
            }
        } else {
            if (buttonIsPressed) {
                TurnOffPump(pump, true);
            } else if (!itsTimeToBeOn && !buttonPressedRecently) {
                TurnOffPump(pump, false);
            }
        }
    }

    
    // time sort of updates once a second (not really...)
    delay (1000);
   // while(1); //infinite for loop blocking
}
