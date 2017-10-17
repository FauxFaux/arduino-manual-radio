bool counting_lows = false;
long run_len = 0;
#define min_run 3

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long val = analogRead(A0);
  if (counting_lows) {
    if (val > 400 && run_len > min_run) {
      counting_lows = false;
      Serial.print("Lows: ");
      Serial.println(run_len);
      run_len = 0;
    }
  } else {
    if (val < 100 && run_len > min_run) {
      counting_lows = true;
      Serial.print("High: ");
      Serial.println(run_len);
      run_len = 0;
    }
  }
  run_len++;
}
