#include "Wire.h"
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

static const int RXPin = 6, TXPin = 5; // e.g. ensure pin 6 on arduino is linked to TX on GPS 
static const uint32_t GPSBaud = 9600;
static const int MPU_ADDR = 0x68;
static const String fileName = "data.csv";
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 3, d7 = 2;
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
File file;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup()
{  
  Serial.begin(9600);

  // set analog pins to digital
  pinMode(A0, INPUT); // A0
  pinMode(A1, INPUT); // A1
  pinMode(A2, INPUT); // A2

  // setup LCD screen
  lcd.begin(16, 2); 
  lcd.setCursor(0,0);     

  // setup SD card
  if (!SD.begin(4)) {
    lcd.print("SD Card failed!");
    Serial.println("SD Card failed!");
    while (1);
  }
  lcd.print("SD Card done.");
  Serial.println("SD Card done.");
  
  // setup GPS
  ss.begin(GPSBaud);

  // setup accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // setup file to write to
  file = SD.open(fileName, FILE_WRITE);
}

void loop()
{ 
  lcd.setCursor(0, 0);
  lcd.print("Select mode.");

  
  if(digitalRead(14) == HIGH) {
    lcd.clear();
    lcd.print("Selected record.");

    while(true) {
      while (ss.available() > 0)
        if (gps.encode(ss.read()))
          getInfo();
    
      if (millis() > 5000 && gps.charsProcessed() < 10)
        doNotAvailable();
    }
  } else if(digitalRead(15) == HIGH) {
    lcd.clear();
    lcd.print("Selected reset.");
    removeFile(fileName);
    while(1);
  } else if(digitalRead(16) == HIGH) {
    lcd.clear();
    lcd.print("Selected upload.");
    readFromFile(fileName);
    while(1);
  }
}

void doNotAvailable() {
  Serial.println(F("No GPS detected: check wiring."));
  lcd.print("No GPS detected.");
  
  while(true);
}

void getInfo()
{
  lcd.setCursor(0,0);
  lcd.print("Retrieving info.");
  
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
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());

    char lat[6]; dtostrf(gps.location.lat(), 4, 6, lat);
    char lng[6]; dtostrf(gps.location.lng(), 4, 6, lng);

    char dataString[50]; sprintf(dataString, "%s,%s,%lu,%d,%d,%d", lat, lng, (unsigned long) now(), accelerometer_x, accelerometer_y, accelerometer_z);
    Serial.println(dataString);
    writeToFile(dataString);
  }
  
  delay(1000);
  lcd.clear();
}

void writeToFile(String row) {
  if(file) {
    file.println(row);
    lcd.setCursor(0,1);
    lcd.println("Written to file.");
  } else {
    Serial.println("Error opening file.");
    lcd.setCursor(0,0);
    lcd.print("Error opening file.");
  }
}

void removeFile(String fileName) {
   lcd.clear();
   
   // filename must include extension
   if(SD.exists(fileName)) {
    Serial.println("Removing file.");
    SD.remove(fileName);
    Serial.println("Successfully removed file.");
  } else {
    Serial.println("File doesn't exist");
    lcd.print("File doesn't exist");
  }
}

void readFromFile(String fileName) {
  lcd.clear();
  
  File file = SD.open(fileName);
  if (file) {
    Serial.println("Reading from file.");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Error opening file.");
    lcd.print("File doesn't exist");
  }
}
