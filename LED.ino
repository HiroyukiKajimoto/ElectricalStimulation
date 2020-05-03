int LEDPIN=12;

void setup() {
  pinMode(LEDPIN,OUTPUT);
  Serial.begin(921600);
}

void loop() {
  digitalWrite(LEDPIN, HIGH);   // sets the LED on
  delay(1000);                  // waits for a second
  digitalWrite(LEDPIN, LOW);    // sets the LED off
  delay(1000);                  // waits for a second
}
