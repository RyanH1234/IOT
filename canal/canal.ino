#include "Wire.h"
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

static const int RXPin = 6, TXPin = 5; // e.g. ensure pin 4 on arduino is linked to TX on GPS 
static const uint32_t GPSBaud = 9600;
static const int MPU_ADDR = 0x68;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data

void setup()
{
  Serial.begin(9600);

  // setup SD card
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("SD Card initialization done.");
  
  // setup GPS
  ss.begin(GPSBaud);

  // setup accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop()
{   
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      getInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
    doNotAvailable();
}

void doNotAvailable() {
  Serial.println(F("No GPS detected: check wiring."));
  while(true);
}

void getInfo()
{
  // tell MPU-6050 we want to read
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3*2, true);
  
  accelerometer_x = Wire.read()<<8 | Wire.read();
  accelerometer_y = Wire.read()<<8 | Wire.read();
  accelerometer_z = Wire.read()<<8 | Wire.read();

  bool gpsLocationValid = gps.location.isValid();
  bool gpsDateValid = gps.date.isValid();
  bool gpsTimeValid = gps.time.isValid();
  
  if (gpsLocationValid && gpsDateValid && gpsTimeValid) {
    float lat = gps.location.lat();
    float lng = gps.location.lng();

    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
    long epoch_utc = now();

    String dataString = String(
      String(lat, 6) + "," + String(lng, 6)
      + "," + String(epoch_utc)
      + "," + String(accelerometer_x) + "," + String(accelerometer_y) + "," + String(accelerometer_z)
    );

    Serial.println(dataString);
    writeToFile("data.csv", dataString);
  }

  delay(1000);
}

void removeFile(String fileName) {
   // filename must include extension
   if(SD.exists(fileName)) {
    Serial.println("Removing file.");
    SD.remove(fileName);
    Serial.println("Successfully removed file.");
  }
}

void writeToFile(String fileName, String row) {
  File file = SD.open(fileName, FILE_WRITE);
  if(file) {
    Serial.println("Writing to file.");
    file.println(row);
    file.close();
    Serial.println("Finished writing to file.");
  } else {
    Serial.println("Error opening file.");
  }
}

void readFromFile(String fileName) {
  File file = SD.open(fileName);
  if (file) {
    Serial.println("Reading from file.");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Error opening file.");
  }
}
