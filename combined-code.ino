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

RV3028C7 rtc;


float p = 3.1415926;
int scroll = 0;
int timeout = 0;
String data;
unsigned long duration;
bool backlightOff = false;
  
void setup()
{
  Serial.begin(9600);
  tft.init(240, 240);
  
  uint16_t time = millis();
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

 if (!lsm.begin_I2C()) {
    Serial.println("Failed to find LSM6DS33 chip");
    while (1) {
      delay(10);
    }
  }
  
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
  pinMode(button_pin, INPUT_PULLUP);
}





 
void loop() {
 
  if (timeout <= 20000 && !backlightOff && digitalRead(button_pin) != 0) {
    timeout++;
    Serial.println("+1");
    delay(1);
  }
  
  
  Serial.println(digitalRead(4));
  delay(20);

  duration = pulseIn(pin, LOW);
  Serial.println(duration); 

  Serial.println(digitalRead(button_pin));

  if (digitalRead(button_pin) == 0){
    if (scroll == 0){
     digitalWrite(backlight_pin,HIGH);
     tft.fillScreen(ST77XX_BLACK);
     //Function for time goes here
     Serial.println("Time displayed");
     scroll = 1;
    }
    else if (scroll == 1){
     digitalWrite(backlight_pin,HIGH);
     tft.fillScreen(ST77XX_BLACK);
     stepcount();
     scroll = 2;
    }
    else if (scroll == 2)
    {
     digitalWrite(backlight_pin,HIGH);
     //Fill screen is already included in function
     oximeterreadings();
     scroll = 0;
    }
    timeout = 0;
    delay(500);
    backlightOff = false;
  }
   
  if(duration >= 20000) {
   digitalWrite(backlight_pin,HIGH);
   tft.fillScreen(ST77XX_BLACK);
   bluetooth();
   delay(5);
   timeout = 0;
  } 

  if(timeout >= 20000) {
   digitalWrite(backlight_pin, LOW);
   Serial.println("Backlight off");
   timeout = 0;
   backlightOff = true;
 }
}




void oximeterreadings() {
 
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      tft.fillScreen(ST77XX_BLACK);
      int heartRate = int(pox.getHeartRate());
      String heartRateString = "Heart rate: " + String(heartRate);
      testdrawtext(heartRateString, ST77XX_WHITE, 6);
      Serial.print("Heart rate:");
      Serial.print(pox.getHeartRate());
      int SpO2 = int(pox.getSpO2());
      String SpO2String = "Oxygen level:" + String(SpO2);
      testdrawtext(SpO2String, ST77XX_WHITE, 11);
      Serial.print("bpm / SpO2:");
      Serial.print(pox.getSpO2());
      Serial.println("%");
      tsLastReport = millis();
  }
}

void time(){
  
  //String time = .rtc.getCurrentDateTime();
  //String h= time.substrate(13, 14); 
  //String m = time.substrate(16,17)
 //tft.println(h + ":" +m);
 //testdrawtext(currentTime,ST77XX_WHITE, 10);
  
  tft.println(rtc.getCurrentDateTime());
  delay(1000);

}

// function for stepcount
void stepcount() {
 int stepcount = lsm.readPedometer();
 String stepcountString = "Steps taken: "+ String(stepcount);
 testdrawtext(stepcountString, ST77XX_WHITE, 10);
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

void bluetooth() {
 
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

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
 
 // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    pox.update();
    data = "Steps taken: " + bluetoothstepcount() + "\nHeart rate: " + bluetoothheart() + "\nOxygen level: " + bluetoothoxygen() + "\n";
    bleuart.println(data);
  } 
}


void testdrawtext(String text, uint16_t color, int line) {
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
