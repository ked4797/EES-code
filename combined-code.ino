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
 
}
 
void loop() {
 
  while (timeout < 21) {
  
  timeout = timeout + 1;
  delay(1000);
  
 }
 
  Serial.println(digitalRead(4));
  delay(20);
  
  duration = pulseIn(pin, LOW);
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

void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
    delay(0);
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

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}
