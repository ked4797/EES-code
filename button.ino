// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(4, INPUT_PULLUP);
}

// the loop function runs over and over again forever
void loop() {
    Serial.println(digitalRead(4));
    delay(20);
}
