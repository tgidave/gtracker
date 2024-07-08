#include <Wire.h>
#include <RTCZero.h> // Include RTC library - make sure it's installed!
#include <DateTime.h>
#include "gtracker.h"

// Comment the next line out before shipping...
#define DEBUG

//#define SERIAL_PORT SERIAL_PORTUSB //Used for Sparkfun boards
#define SERIAL_PORT Serial    //Used for Seeeduino XIOA and Adafruit QT Py board

#define MINUTES_TO_WAIT 10

RTCZero rtc;

typedef struct G_data_struct {
  float max_X;
  float max_Y;
  float max_Z;
} G_data;

G_data gdata;

int get_results = false;

void setNextAlarm(int nextMinutes) {

  byte alarmHours;
  byte alarmMinutes;
  byte alarmSeconds;

  SERIAL_PORT.print("nextMinutes = ");
  SERIAL_PORT.println(nextMinutes);

#ifdef DEBUG
  if (nextMinutes > 60) {
    SERIAL_PORT.print("Error! nextMinutes can not be greater than 60! - ");
    SERIAL_PORT.println(nextMinutes);
    return;
  }
#endif

  alarmHours = rtc.getHours();
  alarmMinutes = rtc.getMinutes();
  alarmSeconds = rtc.getSeconds();

  rtc.setAlarmHours(alarmHours);

  if ((alarmMinutes + nextMinutes) >= 60) {
    alarmMinutes = (alarmMinutes + nextMinutes) - 60;
    rtc.setAlarmMinutes(alarmMinutes);

    if ((alarmHours + 1) >= 24) {
      alarmHours = 0;
      rtc.setAlarmHours(alarmMinutes);
    } else {
      alarmHours += 1;
      rtc.setAlarmHours(alarmHours);
    }
  } else {
    alarmMinutes += nextMinutes;
    rtc.setAlarmMinutes(alarmMinutes);
  }

  rtc.setAlarmSeconds(alarmSeconds);

  SERIAL_PORT.print("Alarm set to ");
  SERIAL_PORT.print(alarmHours);
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(alarmMinutes);
  SERIAL_PORT.print(':');
  SERIAL_PORT.println(alarmSeconds);
}

void alarmMatch(void) {
  get_results = true;
  rtc.disableAlarm();
}

void setup(void) {

  SERIAL_PORT.begin(115200);

  while (!SERIAL_PORT)
  {
    ;
  }

  SERIAL_PORT.println("Controller code...");

  rtc.begin();

  rtc.setTime(0, 0, 0); // Then set the time
  rtc.setDate(0, 0, 0); // And the date
  SERIAL_PORT.println("RTC Started!");

  SERIAL_PORT.println();
  SERIAL_PORT.print(rtc.getYear());
  SERIAL_PORT.print('/');
  SERIAL_PORT.print(rtc.getMonth());
  SERIAL_PORT.print('/');
  SERIAL_PORT.print(rtc.getDay());
  SERIAL_PORT.print(" ");
  SERIAL_PORT.print(rtc.getHours());
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(rtc.getMinutes());
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(rtc.getSeconds());
  SERIAL_PORT.println();
  SERIAL_PORT.println();

  rtc.attachInterrupt(alarmMatch); // callback while alarm is match
  setNextAlarm(MINUTES_TO_WAIT);
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day

  Wire.begin();
}

void loop(void) {
  unsigned int dataLen;
  uint8_t *wkptr;
  int i;

  if (get_results == true) {
    
    SERIAL_PORT.println("Alarm!!!");

    //Check to see if the peripheral is there...
    Wire.beginTransmission(GTRACKER_ADDR);

    SERIAL_PORT.println("begin done");

    if(Wire.endTransmission(GTRACKER_ADDR) == 0) {

      SERIAL_PORT.println("end done");

      wkptr = (uint8_t *)&gdata;
      i = 0;

      SERIAL_PORT.println("sending requectFrom");

      dataLen = Wire.requestFrom(GTRACKER_ADDR, sizeof(gdata));

      SERIAL_PORT.println(dataLen);

#ifdef DEBUG
      if (dataLen > sizeof(gdata)) {
        SERIAL_PORT.print("Bad return from Wire.requestFrom() = ");
        SERIAL_PORT.println(dataLen);
        dataLen = sizeof(gdata);
      }
  #endif

      if (dataLen > 0) {

        while (Wire.available()) {
          wkptr[i] = Wire.read();    // Receive a byte as character
          ++i;

          if (i >= dataLen) {
            SERIAL_PORT.println("Peripheral sending too much data!!!");
            break;
          }
        }

        SERIAL_PORT.println("G_data received!!!");
        SERIAL_PORT.print("max_X = ");
        SERIAL_PORT.print(gdata.max_X);
        SERIAL_PORT.print(" ");
        SERIAL_PORT.print("max_Y = ");
        SERIAL_PORT.print(gdata.max_Y);
        SERIAL_PORT.print(" ");
        SERIAL_PORT.print("max_Z = ");
        SERIAL_PORT.print(gdata.max_Z);
        SERIAL_PORT.println();
        SERIAL_PORT.println("Done...");
        SERIAL_PORT.println();

      }
    } else {
      SERIAL_PORT.println("Peripheral isn't ready...");
    }
    
    get_results = false;
    setNextAlarm(MINUTES_TO_WAIT);
    rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day
  }
}


