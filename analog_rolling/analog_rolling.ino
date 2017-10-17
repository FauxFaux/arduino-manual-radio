// all of these are properly tuned, the defaults are just guesses
uint16_t sig_fast = 0;
uint16_t sig_background = 10;
uint16_t sig_high = 100;
uint16_t high = 100;

uint8_t sig_background_update = 0;

// higher "DECAY": current reading more relevant
#define FAST_DECAY 40
#define BACKGROUND_DECAY 2

#define DECIMATE 20
#define WIGGLE 2
uint16_t deci_count = DECIMATE;
uint8_t deci_hi = 0;

uint8_t symbol_width = 6;
uint8_t cur_run = 0;
bool first_bit = false;
bool second_bit = false;

enum State {
  AWAIT_FIRST_HIGH,
  AWAIT_FIRST_LOW,
  AWAIT_SECOND_HIGH,
  AWAIT_SECOND_LOW,
  DATA,
  VERIFY_DATA,
} state = AWAIT_FIRST_HIGH;

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

    switch (state) {
      case AWAIT_FIRST_HIGH: {
        if (hi) {
          cur_run++;
        } else {
          if (cur_run > 3) {
            symbol_width = cur_run;
            state = AWAIT_FIRST_LOW;
          }

          cur_run = 0;
        }
      } break;
      case AWAIT_FIRST_LOW: {
        if (!hi) {
          cur_run++;
        } else {
          if (abs(cur_run - symbol_width) < WIGGLE) {
            state = AWAIT_SECOND_HIGH;
          } else {
            state = AWAIT_FIRST_HIGH;
          }
          cur_run = 0;
        }
      } break;
      case AWAIT_SECOND_HIGH: {
        if (hi) {
          cur_run++;
        } else {
          if (abs(cur_run - symbol_width) < WIGGLE) {
            state = AWAIT_SECOND_LOW;
          } else {
            state = AWAIT_FIRST_HIGH;
          }
          cur_run = 0;          
        }
      } break;
      case AWAIT_SECOND_LOW: {
        if (!hi) {
          cur_run++;
        } else {
          if (abs(cur_run - symbol_width) < WIGGLE) {
            state = DATA;
            Serial.print(symbol_width);
            Serial.print(": (");
            cur_run = 2 * symbol_width;
          } else {
            state = AWAIT_FIRST_HIGH;
            cur_run = 0;
          }
        }
      } break;
      case DATA: {
        Serial.print(hi ? "X" : ".");

        if (0 == cur_run--) {
          if (first_bit && !second_bit) {
            Serial.print(") 0 (");
            cur_run = 2 * symbol_width;
          } else if (!first_bit && second_bit) {
            Serial.print(") 1 (");
            cur_run = 2 * symbol_width;
          } else {
            Serial.println(") end");
            state = AWAIT_FIRST_HIGH;
            cur_run = 0;
          }
        } else if (cur_run == symbol_width / 2) {
          second_bit = hi;
          Serial.print("_");
        } else if (cur_run == symbol_width + symbol_width/2) {
          first_bit = hi;
          Serial.print("_");
        }
      } break;
    }
  }

#ifdef DEBUG_BACKGROUND
  printer++;

  if (printer == 2) {
    Serial.print(hi ? "X" : ".");
    printer = 0;
  }
#endif
}
