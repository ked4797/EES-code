#include <Adafruit_LSM6DSOX.h>

Adafruit_LSM6DSOX lsm;

// For SPI mode, we need a CS pin
#define LSM_CS 12
// For software-SPI mode we need SCK/MOSI/MISO pins
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11


void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit LSM6DS pedometer test!");

  if (!lsm.begin_I2C()) {
    // if (!lsm.begin_SPI(LSM_CS)) {
    // if (!lsm.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
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
}

void loop() {
  Serial.print("Steps taken: "); 
  Serial.println(lsm.readPedometer());
  delay(100); // can wait as long as you like!
}
