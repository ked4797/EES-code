#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <RV3028C7.h>

// oximeter setup
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;
uint32_t tsLastReport = 0;

// accelerometer setup
Adafruit_LSM6DSOX lsm;

// BLE setup
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

RV3028C7 rtc;

// display variables
#define LSM_CS 12
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11

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
#define button_pin 13

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


float p = 3.1415926;
int scroll = 0;
int timeout = 0;
String data;
bool backlightOff = false;
bool beatDetected = false;
unsigned long timePressed;
bool prevPressed = false;
int detection = 0;
int heartRate = 0;
int SpO2;
String heartRateString;
String SpO2String;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  tft.init(240,240);
  tft.fillScreen(ST77XX_BLACK);
  while (rtc.begin() == false){
    Serial.println("Failed to detect RV-3028-C7!");
  }
  if (!pox.begin()){
    Serial.println("Oximeter failed");
    for(;;);
  } else {
    Serial.println("Oximeter success");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  pox.shutdown();
  delay(500);
  pox.resume();
  if (!lsm.begin_I2C()) {
    Serial.println("Failed to find LSM6DSOX chip");
  } else {
    Serial.println("LSM6DSOX found!");
  }
  lsm.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm.setAccelDataRate(LSM6DS_RATE_26_HZ);
  lsm.setGyroDataRate(LSM6DS_RATE_26_HZ);
  lsm.configInt1(false, false, false, true);
  lsm.enablePedometer(true);
  pinMode(backlight_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}
 
void loop() {
 
  if (digitalRead(button_pin) == 0 && prevPressed == false){
    timePressed = millis();
    prevPressed = true;
  }

   if (timeout <= 20000 && !backlightOff && digitalRead(button_pin) != 0) { 
    timeout++;      
    Serial.println(timeout);   
    delay(1); 
  }

  if(digitalRead(button_pin) == 1 && prevPressed == true){
    if (millis() - timePressed > 2000){
       digitalWrite(backlight_pin,HIGH);
       tft.fillScreen(ST77XX_BLACK);
       bluetooth();
       while ( !bleuart.available() && digitalRead(button_pin) == 1)
       {
        delay(1);
       }
       while ( bleuart.available() && digitalRead(button_pin) == 1)
        {
          delay(1000);
          pox.update();
          data = "Steps taken: " + bluetoothstepcount() + "\nHeart rate: " + bluetoothheart() + "\nOxygen level: " + bluetoothoxygen() + "\n";
          bleuart.println(data);
        } 
       delay(5);
       timeout = 0;
    }
    prevPressed = false;
  }
  
  Serial.println(digitalRead(button_pin));
  

  if (digitalRead(button_pin) == 0){
    digitalWrite(backlight_pin, HIGH);
    if (scroll == 0){
     tft.fillScreen(ST77XX_BLACK);
     showTime();
     Serial.println("Time displayed");
     scroll = 1;
     pox.shutdown();
    }
    else if (scroll == 1){
     tft.fillScreen(ST77XX_BLACK);
     stepcount();
     scroll = 2;
     pox.shutdown();
    }
    else if (scroll == 2)
    {
    tft.fillScreen(ST77XX_BLACK); 
    pox.resume();
    while (!beatDetected){
      pox.update();
      heartRate = int(pox.getHeartRate());
      SpO2 = int(pox.getSpO2());
      Serial.print("Heart rate: ");
      Serial.println(heartRate);
      Serial.println(SpO2);
      delay(1);
      if (String(heartRate) != "0"){
        beatDetected = true;
      }
    }
    drawtext(String(heartRate), ST77XX_WHITE, 6);
    drawtext(String(SpO2), ST77XX_WHITE, 11);
    scroll = 0;
    }
    timeout = 0;
    delay(500);
    backlightOff = false;
  }

  if(timeout >= 20000) {
   digitalWrite(backlight_pin, LOW);
   Serial.println("Backlight off");
   timeout = 0;
   scroll = 0;
   backlightOff = true;
   pox.shutdown();
 }
}



void oximeterreadings() {
  while(heartRate < 1){
      pox.update();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
          Serial.print("Heart rate:");
          Serial.print(pox.getHeartRate());
          Serial.print("bpm / SpO2:");
          Serial.print(pox.getSpO2());
          Serial.println("%");
          heartRate = pox.getHeartRate();
          SpO2 = pox.getSpO2();
          tsLastReport = millis();
      }
      delay(10);
    }
  Serial.println(heartRate);
  Serial.println(SpO2);
  drawtext(String(heartRate), ST77XX_WHITE, 6);
  drawtext(String(SpO2), ST77XX_WHITE, 11);
}



// function for stepcount
void stepcount() {
 int stepcount = lsm.readPedometer();
 String stepcountString = "Steps taken: "+ String(stepcount);
 drawtext(stepcountString, ST77XX_WHITE, 10);
}



// function for stepcount for bluetooth
String bluetoothstepcount() {
 int stepcount = lsm.readPedometer();
 String stepcountString = String(stepcount);
 return stepcountString;
 }



String bluetoothheart() {
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      int heartRate = int(pox.getHeartRate());
      String heartRateString = String(heartRate);
      tsLastReport = millis();
      return heartRateString;
  }
}

String bluetoothoxygen() { 
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      int SpO2 = int(pox.getSpO2());
      String SpO2String = String(SpO2);
      tsLastReport = millis();
      return SpO2String;
  }
}

void showTime(){
  //String time = .rtc.getCurrentDateTime();
  //String h= time.substrate(13, 14); 
  //String m = time.substrate(16,17)
 //tft.println(h + ":" +m);
 //drawtext(currentTime,ST77XX_WHITE, 10);
  
  drawtext(String(rtc.getCurrentDateTime()), ST77XX_WHITE, 3);
  delay(1000);
}

void bluetooth() {
 
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(false);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Bluefruit52");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");
}


void drawtext(String text, uint16_t color, int line) {
  tft.setCursor(0, line*10);
  tft.setTextColor(color);
  tft.setTextSize(3);
  tft.setTextWrap(true);
  tft.println(text);
}

void startAdv()
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void onBeatDetected()
{
    Serial.println("Beat!");
}
