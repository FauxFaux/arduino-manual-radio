// all of these are properly tuned, the defaults are just guesses
uint16_t sig_fast = 0;
uint16_t sig_background = 10;
uint16_t sig_high = 100;
uint16_t high = 100;

uint8_t sig_background_update = 0;

#define FAST_DECAY 40
#define BACKGROUND_DECAY 2

uint16_t printer = 0;

void setup() {
  Serial.begin(9600);
}

uint16_t sample() {
  unsigned long val = analogRead(A0);

  if (val > 1024) {
    return 1024;
  }

  return (uint16_t) val;
}

#define DECAY(sig, val, decay)                    \
  sig = uint16_t((                                \
    (uint32_t(decay) * uint32_t(val)) +           \
    (uint32_t(256 - decay) * uint32_t(sig))) \
    / uint32_t(256))

void loop() {
  uint16_t val = sample();

  DECAY(sig_fast, val, FAST_DECAY);

  if (sig_fast > high) {
    high = sig_fast;
  }

  uint16_t trigger = sig_background + ((sig_high - sig_background) / 2);

  if (!sig_background_update++) {
    Serial.println();
    Serial.print("background: ");
    Serial.print(sig_background);
    Serial.print(" sig_high:");
    Serial.print(sig_high);
    Serial.print(" high:");
    Serial.print(high);
    Serial.print(" trigger:");
    Serial.print(trigger);
    Serial.print(" -- ");
    DECAY(sig_background, val, BACKGROUND_DECAY);
    DECAY(sig_high, high, BACKGROUND_DECAY);
    high = sig_background;
  }

  printer++;

  if (printer == 2) {
    //Serial.print(val);
    //Serial.print(" - ");
    if (sig_fast > trigger) {
      Serial.print("X");
      //Serial.println(sig_fast);
    } else {
      Serial.print(".");
      //Serial.println(sig_fast);
    }
    printer = 0;
  }
}
