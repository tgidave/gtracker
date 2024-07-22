///////////////////////////////////////////////////////////////////////////////
//
// gtracker.ino
//
// This code will track G forces.
//
// The gtracker program will track G forces that occur for the first user
// defined period of time and then send the results to another device through
// the serial port to be reported back to the user.
//
// Author                                                       
//                                                                               
// Uncle Dave                                                  
//                                                                               
//  License                                                                                                                                    
//  Unknown (Talk to Cy)                                                        
//                                                                               
//  HISTORY                                                             
//                                                                               
//  v1.0   - First release
//
///////////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <DateTime.h>
#include <Wire.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h> // Include RTC library - make sure it's installed!
#include <Adafruit_NeoPixel.h>
#include "gtracker.h"

#define DEBUG
#define DEBUG_SERIAL Serial

#define REPORT_SERIAL       Serial1
//#define REPORT_SERIAL_EVENT Serial1event

#define MINUTES_TO_WAIT 5

float max_X;
float max_Y;
float max_Z;

int reportResults = false;
int reportNow     = false;
int reportDone    = false;
bool firstTime    = true;

Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

RTCZero rtc; // Create an RTC object

void setNextAlarm(int nextMinutes) {

  byte alarmHours;
  byte alarmMinutes;
  byte alarmSeconds;

#ifdef DEBUG
  DEBUG_SERIAL.print("nextMinutes = ");
  DEBUG_SERIAL.println(nextMinutes);

  if (nextMinutes > 60) {
    DEBUG_SERIAL.print("Error! nextMinutes can not be greater than 60! - ");
    DEBUG_SERIAL.println(nextMinutes);
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

#ifdef DEBUG
  DEBUG_SERIAL.print("Alarm set to ");
  DEBUG_SERIAL.print(alarmHours);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(alarmMinutes);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.println(alarmSeconds);
#endif
}

void alarmMatch(void) {

  reportResults = true;
// The next line is for testing purposes only - normally commented out.
//  reportNow = true;
}

void setup(void) {

  max_X = 0;
  max_Y = 0;
  max_Z = 0;

  int i;

  int available;

#ifdef DEBUG 
  DEBUG_SERIAL.begin(115200);

//  while (!DEBUG_SERIAL) {
//    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
//  }

  DEBUG_SERIAL.println("gtracker code");
#endif

  REPORT_SERIAL.begin(9600);

//  available = REPORT_SERIAL.availableForWrite();
//  DEBUG_SERIAL.print("availableForWrite = ");
//  DEBUG_SERIAL.println(available);

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

  DEBUG_SERIAL.println("H3LIS331 found!");

  //lis.setRange(H3LIS331_RANGE_100_G);   // 100, 200, or 400 G!
  //lis.setRange(H3LIS331_RANGE_200_G);   // 100, 200, or 400 G!
  lis.setRange(H3LIS331_RANGE_400_G);   // 100, 200, or 400 G!

  DEBUG_SERIAL.print("Range set to: ");

  switch (lis.getRange()) {
    case H3LIS331_RANGE_100_G: 
      DEBUG_SERIAL.println("100 g"); 
      break;
    case H3LIS331_RANGE_200_G: 
      DEBUG_SERIAL.println("200 g"); 
      break;
    case H3LIS331_RANGE_400_G: 
      DEBUG_SERIAL.println("400 g"); 
      break;
  }

  lis.setDataRate(LIS331_DATARATE_1000_HZ);

  DEBUG_SERIAL.print("Data rate set to: ");

  switch (lis.getDataRate()) {
    case LIS331_DATARATE_POWERDOWN: 
      DEBUG_SERIAL.println("Powered Down"); 
      break;
    case LIS331_DATARATE_50_HZ: 
      DEBUG_SERIAL.println("50 Hz"); 
      break;
    case LIS331_DATARATE_100_HZ: 
      DEBUG_SERIAL.println("100 Hz"); 
      break;
    case LIS331_DATARATE_400_HZ: 
      DEBUG_SERIAL.println("400 Hz"); 
      break;
    case LIS331_DATARATE_1000_HZ: 
      DEBUG_SERIAL.println("1000 Hz"); 
      break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: 
      DEBUG_SERIAL.println("0.5 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: 
      DEBUG_SERIAL.println("1 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: 
      DEBUG_SERIAL.println("2 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: 
      DEBUG_SERIAL.println("5 Hz Low Power"); 
      break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: 
        DEBUG_SERIAL.println("10 Hz Low Power"); 
        break;
  }

  //!!! notice The year is limited to 2000-2099
  rtc.begin();

  rtc.setTime(0, 0, 0); // Then set the time
  rtc.setDate(0, 0, 0); // And the date
  DEBUG_SERIAL.println("RTC Started!");

  DEBUG_SERIAL.println("adjusted time!");
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.print(rtc.getYear(), DEC);
  DEBUG_SERIAL.print('/');
  DEBUG_SERIAL.print(rtc.getMonth(), DEC);
  DEBUG_SERIAL.print('/');
  DEBUG_SERIAL.print(rtc.getDay(), DEC);
  DEBUG_SERIAL.print(" ");
  DEBUG_SERIAL.print(rtc.getHours(), DEC);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(rtc.getMinutes(), DEC);
  DEBUG_SERIAL.print(':');
  DEBUG_SERIAL.print(rtc.getSeconds(), DEC);
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println();

  rtc.attachInterrupt(alarmMatch); // callback while alarm is match
  setNextAlarm(MINUTES_TO_WAIT);
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day
}

void loop() {

  /* Get a new sensor event, normalized */
  sensors_event_t event;

  int i;
  uint8_t *wkptr;
//  bool firstTime;
  float temp_x;
  float temp_y;
  float temp_z;

  uint8_t msg[340];
  
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
//      DEBUG_SERIAL.println("Alarm...");
      // Clear the read buffer.
      if (firstTime == true) {

        DEBUG_SERIAL.println("reportResults is true");

        while(REPORT_SERIAL.available()) {
          REPORT_SERIAL.read();
        }

        firstTime = false;
      }

      if (REPORT_SERIAL.available()) {
        if (REPORT_SERIAL.read() == '?') {

  #ifdef DEBUG
          DEBUG_SERIAL.println("Sending...");
  #endif
          max_X = max_X / SENSORS_GRAVITY_STANDARD;
          max_Y = max_Y / SENSORS_GRAVITY_STANDARD;
          max_Z = max_Z / SENSORS_GRAVITY_STANDARD;
          sprintf((char *)&msg, "max_X = %.2f max_Y = %.2f max_Z = %.2f\n", max_X, max_Y, max_Z );
          REPORT_SERIAL.write((char *)&msg);
  #ifdef DEBUG
          DEBUG_SERIAL.write((char *)&msg);
          DEBUG_SERIAL.println();
          DEBUG_SERIAL.println("Done...");
          DEBUG_SERIAL.println();
  #endif
          reportDone = true;
        }
      }
    }
  }
}
