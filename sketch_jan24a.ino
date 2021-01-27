#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

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
#include <RV3028C7.h>

RV3028C7 rtc;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

 //object called now 

void setup() {
  rtc.begin();
  tft.begin(240, 240);   
  Serial.begin(9600);

}

void testdrawtext(String text, int line) { // , uint16_t color
  tft.setCursor(line ,10);
  //tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.println(text);
}
void loop() {
  DateTime now;
  now = rtc.now();
  
  int h = now.hour(); 
 int m = now.minute(); 
 int s = now.second(); 
 tft.setTextColor(ST77XX_WHITE)

 tft.setCursor(5,10)
 if (h>10){
   tft.print("0");
 }
 tft.print(h)
 tft.testdrawtext(":", 7);
 
tft.setCursor(8,10) ; 
  if (m>10){
    tft.print("0");
 }
 tft.testdrawtext(":", 10);
 tft.print(m);
 
tft.setCursor(11,10);
 if (s>10){
    tft.print("0");
 }
 tft.testdrawtext(s, 5);
}
