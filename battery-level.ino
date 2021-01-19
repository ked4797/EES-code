void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);     //  opens serial port, sets data rate to 9600 bps
   pinMode(A0,INPUT);
}

void loop() {
  printVolts();
}

void printVolts()
{
  int sensorValue = analogRead(A0); //read the A0 pin value
  float voltage = sensorValue * (8 / 1023.00); //convert the value to a true voltage.
  Serial.print("voltage = ");
  Serial.print(voltage); //print the voltage to LCD
  Serial.println(" V");
  delay(1000);
}
