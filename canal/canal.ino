#include "Wire.h"
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

static const int MPU_ADDR = 0x68;
const char* fileName = "DATA.CSV";

TinyGPSPlus gps;
SoftwareSerial ss(6, 5); // e.g. ensure pin 6 on arduino is linked to TX on GPS 
LiquidCrystal lcd(7, 8, 9, 4, 3, 2); // rs, en, d4, d5, d6, d7


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
  if (!SD.begin(10)) {
    lcd.print("SD Card failed!");
    Serial.println(F("SD Card failed!"));
    while (1);
  }
  lcd.print("SD Card done.");
  Serial.println(F("SD Card done."));
  
  // setup GPS
  ss.begin(9600);

  // setup accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);  
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
    removeFile();
    while(1);
  } else if(digitalRead(16) == HIGH) {
    lcd.clear();
    lcd.print("Selected read.");
    readFromDisk();
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
    
  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());

    char lat[12]; dtostrf(gps.location.lat(), 4, 6, lat);
    char lng[12]; dtostrf(gps.location.lng(), 4, 6, lng);

    char dataString[50]; 
    
    sprintf(
      dataString, 
      "%s,%s,%lu,%d,%d,%d", 
      lat, 
      lng, 
      (unsigned long) now(), 
      Wire.read()<<8 | Wire.read(), // accelerometer x
      Wire.read()<<8 | Wire.read(), // accelerometer y
      Wire.read()<<8 | Wire.read() // accelerometer z
    );

    File file = SD.open(fileName, FILE_WRITE);
    Serial.println(dataString);
    file.println(dataString);
    file.close();
  }
  
  lcd.clear();
  delay(1000);
  return;
}

void removeFile() {
   lcd.clear();
   
   // filename must include extension
   if(SD.exists(fileName)) {
    Serial.println(F("Removing file."));
    
    if(SD.remove(fileName)) {
      lcd.print("Removed file.");
      Serial.println(F("Successfully removed file."));
    } else {
      lcd.print("Unsuccessful");
      Serial.println(F("Unsuccessfully removed file."));
    }
  } else {
    Serial.println(F("File doesn't exist"));
    lcd.print("File DNE");
  }
}

void readFromDisk() {
  lcd.clear();

  Serial.println(SD.exists(fileName));

  if(SD.exists(fileName)) {
    Serial.println(F("Reading from file."));
    File file = SD.open(fileName);
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println(F("File doesn't exist"));
    lcd.print("File DNE");    
  }
}
