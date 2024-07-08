// Basic demo for accelerometer readings from Adafruit LIS331HH

#include <stdio.h>
//#include <time.h>
#include <Wire.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h> // Include RTC library - make sure it's installed!
#include <DateTime.h>
#include <Adafruit_NeoPixel.h>
#include "gtracker.h"

#define DEBUG

#define MINUTES_TO_WAIT 5

float max_X;
float max_Y;
float max_Z;

int reportResults  = false;
int reportNow      = false;
int reportDone     = false;

Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

RTCZero rtc; // Create an RTC object

void setNextAlarm(int nextMinutes) {

  byte alarmHours;
  byte alarmMinutes;
  byte alarmSeconds;

#ifdef DEBUG
  Serial.print("nextMinutes = ");
  Serial.println(nextMinutes);

  if (nextMinutes > 60) {
    Serial.print("Error! nextMinutes can not be greater than 60! - ");
    Serial.println(nextMinutes);
    return;
  }
#endif

  alarmHours = rtc.getHours();
  alarmMinutes = rtc.getMinutes();
  alarmSeconds = rtc.getSeconds();

  if ((alarmMinutes + nextMinutes) >= 60) {
    alarmMinutes = (alarmMinutes + nextMinutes) - 60;
    rtc.setAlarmMinutes(alarmMinutes);

    if ((alarmHours + 1) >= 24) {
      alarmHours = 0;
      rtc.setAlarmHours(alarmHours);
    } else {
      alarmHours += 1;
      rtc.setAlarmHours(alarmHours);
    }
  } else {
    rtc.setAlarmHours(alarmHours);
    alarmMinutes += nextMinutes;
    rtc.setAlarmMinutes(alarmMinutes);
  }

  rtc.setAlarmSeconds(alarmSeconds);
//  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day

#ifdef DEBUG
  Serial.print("Alarm set to ");
  Serial.print(alarmHours);
  Serial.print(':');
  Serial.print(alarmMinutes);
  Serial.print(':');
  Serial.println(alarmSeconds);
#endif
}

void alarmMatch(void) {
  if (Serial1.read == '\n') {
    if (reportDone == false) {
      if (reportResults == true) {
        reportNow = true;
      }
    }
  }
}

void serial1event(voie) {
//  report_now = true;
//}

void setup(void) {

  max_X = 0;
  max_Y = 0;
  max_Z = 0;

  int i;

#ifdef DEBUG 
  Serial.begin(115200);
  Serial.println("gtracker code");
#endif

  Serial1.begin(9600);

  rtc.begin();

  if (!lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address

    while (1) {    

      // set the first pixel #0 to red
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      // and write the data
      pixels.show();
  
      delay(250);
    
      // turn off the pixel
      pixels.clear();
      pixels.show();
  
      delay(250);
      yield();

    }
  }

  Serial.println("H3LIS331 found!");

  lis.setRange(H3LIS331_RANGE_100_G);   // 100, 200, or 400 G!
  //lis.setRange(H3LIS331_RANGE_200_G);   // 100, 200, or 400 G!
  //lis.setRange(H3LIS331_RANGE_400_G);   // 100, 200, or 400 G!

  Serial.print("Range set to: ");

  switch (lis.getRange()) {
    case H3LIS331_RANGE_100_G: 
      Serial.println("100 g"); 
      break;
    case H3LIS331_RANGE_200_G: 
      Serial.println("200 g"); 
      break;
    case H3LIS331_RANGE_400_G: 
      Serial.println("400 g"); 
      break;
  }

  lis.setDataRate(LIS331_DATARATE_1000_HZ);

  Serial.print("Data rate set to: ");

  switch (lis.getDataRate()) {
    case LIS331_DATARATE_POWERDOWN: 
      Serial.println("Powered Down"); 
      break;
    case LIS331_DATARATE_50_HZ: 
      Serial.println("50 Hz"); 
      break;
    case LIS331_DATARATE_100_HZ: 
      Serial.println("100 Hz"); 
      break;
    case LIS331_DATARATE_400_HZ: 
      Serial.println("400 Hz"); 
      break;
    case LIS331_DATARATE_1000_HZ: 
      Serial.println("1000 Hz"); 
      break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: 
      Serial.println("0.5 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: 
      Serial.println("1 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: 
      Serial.println("2 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: 
      Serial.println("5 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: 
        Serial.println("10 Hz Low Power"); 
        break;
  }

  //!!! notice The year is limited to 2000-2099
  rtc.begin();

  rtc.setTime(0, 0, 0); // Then set the time
  rtc.setDate(0, 0, 0); // And the date
  Serial.println("RTC Started!");

  Serial.println("adjusted time!");
  Serial.println();
  Serial.print(rtc.getYear(), DEC);
  Serial.print('/');
  Serial.print(rtc.getMonth(), DEC);
  Serial.print('/');
  Serial.print(rtc.getDay(), DEC);
  Serial.print(" ");
  Serial.print(rtc.getHours(), DEC);
  Serial.print(':');
  Serial.print(rtc.getMinutes(), DEC);
  Serial.print(':');
  Serial.print(rtc.getSeconds(), DEC);
  Serial.println();
  Serial.println();

  rtc.attachInterrupt(alarmMatch); // callback while alarm is match
  setNextAlarm(MINUTES_TO_WAIT);
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day
}

void loop() {

  /* Get a new sensor event, normalized */
  sensors_event_t event;

  float temp_x;
  float temp_y;
  float temp_z;

  char msg[100];

  if (reportDone == false) {
    if (reportResults == false) {
      lis.getEvent(&event);

      if (max_X < (temp_x = event.acceleration.x)) {
        max_X = temp_x;
      }

      if (max_Y < (temp_y = event.acceleration.y)) {
        max_Y = temp_y;
      }

      if (max_Z < (temp_z = event.acceleration.z)) {
        max_Z = temp_z;
      }
    } else { // reportResults == true
      if (reportNow == true) {
        max_X = max_X / SENSORS_GRAVITY_STANDARD;
        max_Y = max_Y / SENSORS_GRAVITY_STANDARD;
        max_Z = max_Z / SENSORS_GRAVITY_STANDARD;

        sprintf((char *)&msg, "max_X = %.2f max_Y = %.2f max_Z = %.2f\n", max_X, max_Y, max_Z );
        Serial1.write((char *)&msg);
#ifdef DEBUG
        Serial.write(char *)&msg);
        Serial.println();
        Serial.println("Done...");
        Serial.println();
#endif
        reportDone = true
      }
    }
  }
}
