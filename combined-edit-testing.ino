#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

Adafruit_LSM6DSOX lsm;

// For SPI mode, we need a CS pin
#define LSM_CS 12
// For software-SPI mode we need SCK/MOSI/MISO pins
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11
 
#define REPORTING_PERIOD_MS     1000

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         7
#endif

#define backlight_pin 12

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;
int scroll = 0;
int timeout = 0;
 
PulseOximeter pox;
uint32_t tsLastReport = 0;
 
void onBeatDetected()
{
    Serial.println("Beat!");
}

int pin = 4; //For button
unsigned long duration;
 
void setup()
{
    Serial.begin(9600);
    tft.init(240, 240);

    Serial.println(F("Initialized"));

  // basically all of the display code only runs through once and is therefore in setup
  
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // large block of text
  tft.fillScreen(ST77XX_BLACK);
  delay(1000);

  Serial.println("done");
  delay(1000);
  
   Serial.print("Initializing pulse oximeter..");

   // Initialize the PulseOximeter instance
   // Failures are generally due to an improper I2C wiring, missing power supply
   // or wrong target chip
   if (!pox.begin()) {
       Serial.println("FAILED");
       for(;;);
   } else {
       Serial.println("SUCCESS");
   }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

   // Register a callback for the beat detection
   pox.setOnBeatDetectedCallback(onBeatDetected);

   Serial.begin(9600);
   pinMode(pin, INPUT_PULLUP);
 
 if (!lsm.begin_I2C()) {
    // if (!lsm.begin_SPI(LSM_CS)) {
    // if (!lsm.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DS33 chip");
    while (1) {
      delay(10);
    }
  }

 // gyro-accelerometer setup code
  Serial.println("LSM6DS33 Found!");

  // Set to 2G range and 26 Hz update rate
  lsm.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm.setAccelDataRate(LSM6DS_RATE_26_HZ);
  lsm.setGyroDataRate(LSM6DS_RATE_26_HZ);

  // step detect output on INT1
  lsm.configInt1(false, false, false, true);

  // turn it on!
  lsm.enablePedometer(true);

  pinMode(backlight_pin, OUTPUT);
 
}
 
void loop() {
 
  while (timeout < 21 && digitalRead(pin) != 0) {
  
  timeout = timeout + 1;
  delay(1000);
  
 }

 if(timeout >= 20) {
    
   digitalWrite(backlight_pin,LOW);
   timeout = 0;
   delay(5000);
 }
 
  Serial.println(digitalRead(4));
  delay(20);
  
  Serial.println(duration); //in microseconds
  
    
    if(scroll==0 && digitalRead(4)==0) {
     
     tft.fillScreen(ST77XX_BLACK);
     //Function for time goes here
     scroll = 1;
     timeout = 0;
     
    } else if(scroll==1 && digitalRead(4)==0) {
     
     tft.fillScreen(ST77XX_BLACK);
     stepcount();
     scroll = 2;
     timeout = 0;
     
    } else if(scroll==2 && digitalRead(4)==0) {
     
     //Fill screen is already included in function
     oximeterreadings();
     scroll = 0;
     timeout = 0;
      
    } else if(duration > 2000000) {
     
     tft.fillScreen(ST77XX_BLACK);
     //Function for Bluetooth goes here
     timeout = 0;
    
    }
 
   if(timeout >= 20) {
    
   digitalWrite(backlight_pin,LOW);
   timeout = 0;
   delay(5000);
  
  }

}
void oximeterreadings() {
 
// Make sure to call update as fast as possible
pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      tft.fillScreen(ST77XX_BLACK);
      int heartRate = int(pox.getHeartRate());
      String heartRateChar = String(heartRate);
      String heartRateString = "Heart rate: " + heartRateChar;
      testdrawtext(heartRateString, ST77XX_WHITE, 0);
      Serial.print("Heart rate:");
      Serial.print(pox.getHeartRate());
      int SpO2 = int(pox.getSpO2());
      String SpO2Char = String(SpO2);
      String SpO2String = "Oxygen level: " + SpO2Char;
      testdrawtext(SpO2String, ST77XX_WHITE, 2);
      Serial.print("bpm / SpO2:");
      Serial.print(pox.getSpO2());
      Serial.println("%");
  
      tsLastReport = millis();
  }
}

// function for stepcount
void stepcount() {
 int stepcount = lsm.readPedometer();
 String stepcountString = String(stepcount);
 String steps = "Steps taken: " + stepcountString;
 testdrawtext(steps, ST77XX_WHITE, 0);
}

void bluetooth() {
  
}

void testdrawtext(String text, uint16_t color, int line) {
  tft.setCursor(0, line*10);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.println(text);
}
