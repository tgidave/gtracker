
// Basic demo for accelerometer readings from Adafruit H3LIS331

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define H3LIS331_SCK 13
#define H3LIS331_MISO 12
#define H3LIS331_MOSI 11
// Used for hardware & software SPI
#define H3LIS331_CS 10

//#define SERIAL_PORT SerialUSB
#define SERIAL_PORT Serial

Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

void setup(void) {
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  SERIAL_PORT.println("H3LIS331 test!");

//  if (!lis.begin_SPI(H3LIS331_CS)) {
//  if (!lis.begin_SPI(H3LIS331_CS, H3LIS331_SCK, H3LIS331_MISO, H3LIS331_MOSI)) {
 if (! lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    SERIAL_PORT.println("Couldnt start");
    while (1) yield();
  }
  SERIAL_PORT.println("H3LIS331 found!");

  lis.setRange(H3LIS331_RANGE_100_G);   // 100, 200, or 400 G!

  SERIAL_PORT.print("Range set to: ");
  switch (lis.getRange()) {
    case H3LIS331_RANGE_100_G: SERIAL_PORT.println("100 g"); break;
    case H3LIS331_RANGE_200_G: SERIAL_PORT.println("200 g"); break;
    case H3LIS331_RANGE_400_G: SERIAL_PORT.println("400 g"); break;
  }

  lis.setDataRate(LIS331_DATARATE_1000_HZ);

  SERIAL_PORT.print("Data rate set to: ");

  switch (lis.getDataRate()) {
    case LIS331_DATARATE_POWERDOWN: SERIAL_PORT.println("Powered Down"); break;
    case LIS331_DATARATE_50_HZ: SERIAL_PORT.println("50 Hz"); break;
    case LIS331_DATARATE_100_HZ: SERIAL_PORT.println("100 Hz"); break;
    case LIS331_DATARATE_400_HZ: SERIAL_PORT.println("400 Hz"); break;
    case LIS331_DATARATE_1000_HZ: SERIAL_PORT.println("1000 Hz"); break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: SERIAL_PORT.println("0.5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: SERIAL_PORT.println("1 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: SERIAL_PORT.println("2 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: SERIAL_PORT.println("5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: SERIAL_PORT.println("10 Hz Low Power"); break;

  }
}

void loop() {
  /* Get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  SERIAL_PORT.print("\t\tX: "); SERIAL_PORT.print(event.acceleration.x);
  SERIAL_PORT.print(" \tY: "); SERIAL_PORT.print(event.acceleration.y);
  SERIAL_PORT.print(" \tZ: "); SERIAL_PORT.print(event.acceleration.z);
  SERIAL_PORT.println(" m/s^2 ");

  /* Alternately, given the range of the H3LIS331, display the results measured in g */
  // SERIAL_PORT.print("\t\tX:"); SERIAL_PORT.print(event.acceleration.x / SENSORS_GRAVITY_STANDARD);
  // SERIAL_PORT.print(" \tY: "); SERIAL_PORT.print(event.acceleration.y / SENSORS_GRAVITY_STANDARD);
  // SERIAL_PORT.print(" \tZ: "); SERIAL_PORT.print(event.acceleration.z / SENSORS_GRAVITY_STANDARD);
  // SERIAL_PORT.println(" g");

  delay(1000);
}