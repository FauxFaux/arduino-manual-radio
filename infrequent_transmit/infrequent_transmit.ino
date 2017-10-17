#define RF 10
#define DELAY 25

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RF, OUTPUT);
}

void set(int first, int second) {
    digitalWrite(RF, first);
    delay(DELAY);
    digitalWrite(RF, second);
    delay(DELAY);  
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 6; ++i) {
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
