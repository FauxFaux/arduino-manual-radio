#define RF 10

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RF, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 6; ++i) {
    digitalWrite(RF, HIGH);
    delay(30);
    digitalWrite(RF, LOW);
    delay(30);
  }
  digitalWrite(LED_BUILTIN, LOW);
  delay(3000);
}
