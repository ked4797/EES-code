/*
  RV-3028-C7 Set Date Time Example
  Configures date and time to the RTC module via the serial console, and then
  prints the current date and time in ISO 8601 format every second.
  Copyright (c) 2020 Macro Yau
  https://github.com/MacroYau/RV-3028-C7-Arduino-Library
*/

#include <RV3028C7.h>

RV3028C7 rtc;

void setup() {
  Serial.begin(9600);
  Serial.println("RV-3028-C7 Set Date Time Example");
  Serial.println();

  Wire.begin();

  while (rtc.begin() == false) {
    Serial.println("Failed to detect RV-3028-C7!");
    delay(5000);
  }
}

void loop() {
  Serial.println(rtc.getCurrentDateTime());
  delay(1000);
}  
