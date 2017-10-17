#define RF 10

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RF, OUTPUT);
}

void set(int first, int second) {
    digitalWrite(RF, first);
    delay(50);
    digitalWrite(RF, second);
    delay(50);  
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 5; ++i) {
    set(HIGH, LOW);
  }

  set(HIGH, LOW); // 0
  set(LOW, HIGH); // 1
  set(LOW, HIGH); // 1
  set(HIGH, LOW); // 0
  set(LOW, LOW);
  
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
}
