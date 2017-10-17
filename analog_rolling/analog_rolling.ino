// all of these are properly tuned, the defaults are just guesses
uint16_t sig_fast = 0;
uint16_t sig_background = 10;
uint16_t sig_high = 100;
uint16_t high = 100;

uint8_t sig_background_update = 0;

// higher "DECAY": current reading more relevant
#define FAST_DECAY 40
#define BACKGROUND_DECAY 2

#define DECIMATE 80
uint16_t deci_count = DECIMATE;
uint8_t deci_hi = 0;

//#define DEBUG_BACKGROUND
#ifdef DEBUG_BACKGROUND
uint8_t printer = 0;
#endif

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
#ifdef DEBUG_BACKGROUND
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
#endif
    DECAY(sig_background, val, BACKGROUND_DECAY);
    DECAY(sig_high, high, BACKGROUND_DECAY);
    high = sig_background;
  }

  bool hi = sig_fast > trigger;

  if (hi) {
    ++deci_hi;
  }

  if (!--deci_count) {
    deci_count = DECIMATE;
    bool hi = deci_hi > DECIMATE / 2;
    deci_hi = 0;
    Serial.print(hi ? "X" : ".");
  }

#ifdef DEBUG_BACKGROUND
  printer++;

  if (printer == 2) {
    Serial.print(hi ? "X" : ".");
    printer = 0;
  }
#endif
}
