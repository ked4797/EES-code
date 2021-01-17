void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);     //  opens serial port, sets data rate to 9600 bps
   lcd.begin(16, 2);       //// set up the LCD's number of columns and rows: 
   pinMode(A0,OUTPUT);
   pinMode(A1,OUTPUT);
   pinMode(A2,OUTPUT);
   pinMode(A3,OUTPUT);
   pinMode(A4,INPUT);
   lcd.print("Voltage Level");
}

void loop() {
  // put your main code here, to run repeatedly:
  //  Conversion formula for voltage
  analogValue = analogRead (A4);
  Serial.println(analogValue);
  delay (1000); 
  input_voltage = (analogValue * 5.0) / 1024.0;
  lcd.setCursor(0, 1);
  lcd.print("Voltage= ");
  lcd.print(input_voltage);
  Serial.println(input_voltage);
  delay(100);

    
}
