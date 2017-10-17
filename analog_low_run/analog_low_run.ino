unsigned long rolling = 0;

int low_run = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long val = analogRead(A0);

  #define POWER 256
  #define DECAY 178

  rolling = (DECAY * val + (POWER - DECAY) * rolling) / POWER;

  if (rolling < 3) {
    low_run++;

    if (low_run == 120) {
      Serial.println(val);
      low_run = 0;
    }
  } else {
    low_run = 0;
  }
}
