// Basic demo for accelerometer readings from Adafruit LIS331HH

#include <Wire.h>
#include <Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h> // Include RTC library - make sure it's installed!
#include <DateTime.h>
#include "gtracker.h"

// Used for software SPI
//#define LIS331HH_SCK 13
//#define LIS331HH_MISO 12
//#define LIS331HH_MOSI 11
// Used for hardware & software SPI
//#define LIS331HH_CS 10

//#define GDATA_COUNT 1750

//#define SERIAL_PORT SERIAL_PORTUSB //Used for Sparkfun boards
#define SERIAL_PORT Serial    //Used for Seeeduino XIOA and Adafruit QT Py board

#define MINUTES_TO_WAIT 15

typedef struct G_data_struct {
  float max_X;
  float max_Y;
  float max_Z;
} G_data;

G_data gdata;

int report_results  = false;
int report_now      = false;
int report_done     = false;

//Adafruit_LIS331HH lis = Adafruit_LIS331HH();

RTCZero rtc; // Create an RTC object

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
//  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day

  SERIAL_PORT.print("Alarm set to ");
  SERIAL_PORT.print(alarmHours);
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(alarmMinutes);
  SERIAL_PORT.print(':');
  SERIAL_PORT.println(alarmSeconds);
}
 
void alarmMatch(void) {
  report_results = true;
  rtc.disableAlarm();
}

void reportRequested(void) {
  report_now = true; 
}

void setup(void) {

  gdata.max_X = 0;
  gdata.max_Y = 0;
  gdata.max_Z = 0;

  int i;

  SERIAL_PORT.begin(115200);

  while (!SERIAL_PORT) {
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  }

  RTCZero rtc; // Create an RTC object.println("LIS331HH test!");

/*
  if (!lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    SERIAL_PORT.println("Couldnt start");
    while (1)
      yield();
  }

  SERIAL_PORT.println("LIS331HH found!");
  lis.setRange(LIS331HH_RANGE_24_G);   // 6, 12, or 24 G

  SERIAL_PORT.print("Range set to: ");
  switch (lis.getRange()) {
    case LIS331HH_RANGE_6_G:
      SERIAL_PORT.println("6 g");
      break;
    case LIS331HH_RANGE_12_G:
      SERIAL_PORT.println("12 g");
      break;
    case LIS331HH_RANGE_24_G:
      SERIAL_PORT.println("24 g");
      break;
  }

  //lis.setDataRate(LIS331_DATARATE_50_HZ);
  SERIAL_PORT.print("Data rate set to: ");
  switch (lis.getDataRate()) {

    case LIS331_DATARATE_POWERDOWN:
      SERIAL_PORT.println("Powered Down");
      break;
    case LIS331_DATARATE_50_HZ:
      SERIAL_PORT.println("50 Hz");
      break;
    case LIS331_DATARATE_100_HZ:
      SERIAL_PORT.println("100 Hz");
      break;
    case LIS331_DATARATE_400_HZ:
      SERIAL_PORT.println("400 Hz");
      break;
    case LIS331_DATARATE_1000_HZ:
      SERIAL_PORT.println("1000 Hz");
      break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ:   
      SERIAL_PORT.println("0.5 Hz Low Power");
      break;
    case LIS331_DATARATE_LOWPOWER_1_HZ:
      SERIAL_PORT.println("1 Hz Low Power");
      break;
    case LIS331_DATARATE_LOWPOWER_2_HZ:
      SERIAL_PORT.println("2 Hz Low Power");
      break;
    case LIS331_DATARATE_LOWPOWER_5_HZ:
      SERIAL_PORT.println("5 Hz Low Power");
      break;
    case LIS331_DATARATE_LOWPOWER_10_HZ:
      SERIAL_PORT.println("10 Hz Low Power");
      break;

  }
*/
  //!!! notice The year is limited to 2000-2099
  rtc.begin();

  rtc.setTime(0, 0, 0); // Then set the time
  rtc.setDate(0, 0, 0); // And the date
  SERIAL_PORT.println("RTC Started!");

  SERIAL_PORT.println("adjusted time!");
  SERIAL_PORT.println();
  SERIAL_PORT.print(rtc.getYear(), DEC);
  SERIAL_PORT.print('/');
  SERIAL_PORT.print(rtc.getMonth(), DEC);
  SERIAL_PORT.print('/');
  SERIAL_PORT.print(rtc.getDay(), DEC);
  SERIAL_PORT.print(" ");
  SERIAL_PORT.print(rtc.getHours(), DEC);
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(rtc.getMinutes(), DEC);
  SERIAL_PORT.print(':');
  SERIAL_PORT.print(rtc.getSeconds(), DEC);
  SERIAL_PORT.println();
  SERIAL_PORT.println();

  if (rtc.getHours() == 23) {
    rtc.setAlarmHours(0);
  } else {
    rtc.setAlarmHours(rtc.getHours() + 1);
  }

  rtc.setAlarmMinutes(rtc.getMinutes());
  rtc.setAlarmSeconds(rtc.getSeconds());

  rtc.attachInterrupt(alarmMatch); // callback while alarm is match
  setNextAlarm(MINUTES_TO_WAIT);
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // match Every Day
}

void loop(void) {

  /* Get a new sensor event, normalized */
  //sensors_event_t event;

  float temp_x;
  float temp_y;
  float temp_z;

  if ( report_results = true ) {
    report();
  } else if (report_done = false ) {
  /*
      lis.getEvent(&event);

      if (G_data.max_X < (temp_x = event.acceleration.x)) {
        G_data.max_X = temp_x;
      }

      if (G_data.max_Y < (temp_y = event.acceleration.y)) {
        G_data.max_Y = temp_y;
      }

      if (G_data.max_Z < (temp_z = event.acceleration.z)) {
        G_data.max_Z = temp_z;
      }
  */
  }
}

void report(void) {
  SERIAL_PORT.println( "Alarm Match!" );
  SERIAL_PORT.print( "max_X = " );
  SERIAL_PORT.print( gdata.max_X );
  SERIAL_PORT.print( " " ); 
  SERIAL_PORT.print( "max_Y = ");
  SERIAL_PORT.print( gdata.max_Y );
  SERIAL_PORT.print( " " );
  SERIAL_PORT.print( "max_Z = " );
  SERIAL_PORT.print( gdata.max_Z );
  SERIAL_PORT.println();
  SERIAL_PORT.println( "Done..." );
  SERIAL_PORT.println();

  Wire.end();
  Wire.begin(GTRACKER_ADDR);
  Wire.onRequest( reportRequested );
 
  while (1) {
    if (report_now == true) {
      Wire.write((const uint8_t *)&gdata, (size_t)sizeof(gdata));
      noInterrupts();
      report_now = false;
      interrupts();
    }

    yield();
  }

}

