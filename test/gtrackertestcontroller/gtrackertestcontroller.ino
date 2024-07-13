#include <Wire.h>
#include <RTCZero.h> // Include RTC library - make sure it's installed!
#include <DateTime.h>
#include "gtracker.h"

// Comment the next line out before shipping...
#define DEBUG

//#define SERIAL_SERIAL SERIAL_SERIALUSB //Used for Sparkfun boards
#define DEBUG_SERIAL Serial    //Used for Seeeduino XIOA and Adafruit QT Py board

#define REPORT_SERIAL Serial1

#define MINUTES_TO_WAIT 7

#define MAX_TRIES 500

#define BUFF_SIZE 340

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

  DEBUG_SERIAL.print("nextMinutes = ");
  DEBUG_SERIAL.println(nextMinutes);

#ifdef DEBUG
  if (nextMinutes > 60) {
    DEBUG_SERIAL.print("Error! nextMinutes can not be greater than 60! - ");
    DEBUG_SERIAL.println(nextMinutes);
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

  DEBUG_SERIAL.print("Alarm set to ");
  DEBUG_SERIAL.print(alarmHours);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(alarmMinutes);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.println(alarmSeconds);
}

void alarmMatch(void) {
  get_results = true;
  rtc.disableAlarm();
}

void setup(void) {

  char msg[] = "abc"; 

  DEBUG_SERIAL.begin(115200);

  while (!DEBUG_SERIAL) {
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  }

  DEBUG_SERIAL.println("Controller code...");

  REPORT_SERIAL.begin(9600);

  rtc.begin();

  rtc.setTime(0, 0, 0); // Then set the time
  rtc.setDate(0, 0, 0); // And the date
  DEBUG_SERIAL.println("RTC Started!");

  DEBUG_SERIAL.println();
  DEBUG_SERIAL.print(rtc.getYear());
  DEBUG_SERIAL.print('/');
  DEBUG_SERIAL.print(rtc.getMonth());
  DEBUG_SERIAL.print('/');
  DEBUG_SERIAL.print(rtc.getDay());
  DEBUG_SERIAL.print(" ");
  DEBUG_SERIAL.print(rtc.getHours());
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(rtc.getMinutes());
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(rtc.getSeconds());
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println();

  rtc.attachInterrupt(alarmMatch); // callback while alarm is match
  setNextAlarm(MINUTES_TO_WAIT);
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day
}

void loop(void) {

  unsigned int dataLen;
  uint8_t *wkptr;
  int i;
  int tries;
  int xferDone;
  int xferGotData;
  int xferOverrun;
  int xferNoResponse;
  uint8_t buff[BUFF_SIZE];

  if (get_results == true) {

    wkptr = (uint8_t *)&buff;
    i = 0;
    tries = 0;
    xferDone = false;
    xferGotData = false;
    xferOverrun = false;
    xferNoResponse = false;

    DEBUG_SERIAL.println("Alarm!!!");

    while (REPORT_SERIAL.available()) {
      REPORT_SERIAL.read();
      // Clear the read buffer.
    }

    REPORT_SERIAL.write('?');

    DEBUG_SERIAL.println("? sent...");

    i = 0;

    while (1) {
      if (REPORT_SERIAL.available()) {
        buff[i] = REPORT_SERIAL.read();
        xferGotData = true;

        if (buff[i] == '\n') {
          buff[i + 1] = '\0';
          xferDone = true;
          DEBUG_SERIAL.println("Got xferDone");
          break;
        }

        i++;

        if (i >= BUFF_SIZE) {
          xferOverrun = true;
          break;
        }
      } else {  // Data not available

        ++tries;

        if (tries > MAX_TRIES) {
          xferNoResponse = true;
          break;
        }

        delay(10);
      }
    }

    if (xferOverrun == true) {

      DEBUG_SERIAL.println("Overrun on message receive!!!");

      while (1) {
        yield();
      }
    }

    if (xferGotData == true && xferNoResponse == true) {

      DEBUG_SERIAL.println("Timeout receiving message!!!");

      while (1) {
        yield();
      }
    }

    if (xferDone == true) {

      DEBUG_SERIAL.println("G_data received!!!");
      DEBUG_SERIAL.println();
      DEBUG_SERIAL.println("Done...");
      DEBUG_SERIAL.println((char *)&buff);
      DEBUG_SERIAL.println();

    } else {

      DEBUG_SERIAL.println("No response...");
      get_results = false;
      setNextAlarm(MINUTES_TO_WAIT);
      rtc.enableAlarm(rtc.MATCH_HHMMSS); // match hours minutes seconds

    }
  }
}


