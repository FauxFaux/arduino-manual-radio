// all of these are properly tuned, the defaults are just guesses
uint16_t sig_fast = 0;
uint16_t sig_background = 10;
uint16_t sig_high = 100;
uint16_t high = 100;

uint8_t sig_background_update = 0;

// higher "DECAY": current reading more relevant
#define FAST_DECAY 40
#define BACKGROUND_DECAY 2

#define DECIMATE 10
#define WIGGLE 3
uint16_t deci_count = DECIMATE;
uint8_t deci_hi = 0;

uint8_t symbol_width = 6;
uint8_t cur_run = 0;
bool cur_running = false;

bool stashed = false;
bool stashed_bit = false;

uint8_t stashed_chars[3];
uint8_t stashed_pos = 0;

enum State {
  AWAIT_FIRST_HIGH,
  AWAIT_FIRST_LOW,
  AWAIT_SECOND_HIGH,
  AWAIT_SECOND_LOW,
  DATA,
  VERIFY_DATA,
} state = AWAIT_FIRST_HIGH;

enum DecoderState {
  PREMABLE_LOW,
  PREMABLE_HI,
  BYTES,
} dec_state;

uint8_t decoded = 0;
uint8_t dec_pos = 0;

//#define DEBUG_BACKGROUND
#ifdef DEBUG_BACKGROUND
uint8_t printer = 0;
#endif

//#define DEBUG_CLOCK

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

    if (hi == cur_running) {
      if (cur_run > 128) {
        cur_run = 0;
        cur_running = false;
      }
    } else {
      if (cur_run > 1) {
        enjoy_run(cur_running, cur_run);
        cur_running =! cur_running;
        cur_run = 0;
      }
    }

#ifdef DEBUG_CLOCK
    if (state == DATA) {
      Serial.print(hi ? "X" : ".");
    }
#endif

    ++cur_run;
  }

#ifdef DEBUG_BACKGROUND
  printer++;

  if (printer == 2) {
    Serial.print(hi ? "X" : ".");
    printer = 0;
  }
#endif
}

void enjoy_run(bool hi, uint8_t cur_run) {
# define was_symbol (abs(cur_run - symbol_width) < WIGGLE)

  switch (state) {
    case AWAIT_FIRST_HIGH: {
      if (hi && cur_run > 3) {
        symbol_width = cur_run;
        state = AWAIT_FIRST_LOW;
      }
    } break;
    case AWAIT_FIRST_LOW: {
      state = (!hi && was_symbol) ? AWAIT_SECOND_HIGH : AWAIT_FIRST_HIGH;
    } break;
    case AWAIT_SECOND_HIGH: {
      state = (hi && was_symbol) ? AWAIT_SECOND_LOW : AWAIT_FIRST_HIGH;
    } break;
    case AWAIT_SECOND_LOW: {
      state = (!hi && was_symbol) ? DATA : AWAIT_FIRST_HIGH;
#ifdef DEBUG_CLOCK
      Serial.print(symbol_width);
      Serial.print(": (");
#endif
    } break;
    case DATA: {
      const int half = symbol_width / 2;
      const int dubla = symbol_width * 2;
      if (cur_run < half || cur_run > dubla + half) {
        state = AWAIT_FIRST_HIGH;
        dec_state = PREMABLE_LOW;
#ifdef DEBUG_CLOCK
        Serial.println(") end (run length problem)");
#endif
      } else if (abs(cur_run - symbol_width) < half) {
        enjoy_bit(hi);
      } else {
        // it was longer than half a run, must be two bits in a row
        enjoy_bit(hi);
        enjoy_bit(hi);
      }
    } break;
  }
}

void enjoy_bit(bool hi) {
  if (!stashed) {
    stashed = true;
    stashed_bit = hi;
#ifdef DEBUG_CLOCK
    Serial.print("_");
#endif
    return;
  }
  stashed = false;
  if (stashed_bit && !hi) {
    enjoy_decoded_bit(0);
#ifdef DEBUG_CLOCK
    Serial.print(") 0 (");
#endif
  } else if (!stashed_bit && hi) {
    enjoy_decoded_bit(1);
#ifdef DEBUG_CLOCK
    Serial.print(") 1 (");
#endif
  } else {
    state = AWAIT_FIRST_HIGH;
    dec_state = PREMABLE_LOW;
#ifdef DEBUG_CLOCK
    Serial.println(") end (encoding violated)");
#endif
  }
}

void enjoy_decoded_bit(bool hi) {
#if DEBUG_DECODE
  Serial.print(hi ? "X" : ".");
#endif
  switch (dec_state) {
    case PREMABLE_LOW: {
      if (hi) {
        dec_state = PREMABLE_HI;
#if DEBUG_DECODE
        Serial.print("(");
#endif
      }
    } break;
    case PREMABLE_HI: {
      if (hi) {
        dec_state = BYTES;
        decoded = 0;
        dec_pos = 0;
        stashed_pos = 0;
#if DEBUG_DECODE        
        Serial.print(")");
#endif
      } else {
        dec_state = PREMABLE_LOW;
      }
    } break;
    case BYTES: {
      if (dec_pos < 8) {
        decoded |= ((hi ? 1 : 0) << dec_pos);
        ++dec_pos;
      } else {
        bool recieved_parity = false;
        for (uint8_t i = 0; i < 8; ++i) {
          recieved_parity ^= ((decoded & (1 << i)) == (1 << i));
        }

#if DEBUG_DECODE
        Serial.print(" => ");
        Serial.print(decoded);
        Serial.print(" (parity: ");
        Serial.print(hi);
        Serial.print(recieved_parity);
        Serial.println(")");
#endif

        if (hi == recieved_parity) {
          stashed_chars[stashed_pos++] = decoded;
          if (stashed_pos == 3) {
            if ((stashed_chars[0] ^ stashed_chars[1]) == stashed_chars[2]) {
              float val = stashed_chars[0] + stashed_chars[1]/255.0;
              Serial.print("Value checks out: ");
              Serial.println(val);
            } else {
              Serial.print("Checksum failure: ");
              Serial.print(stashed_chars[0]);
              Serial.print(" ^ ");
              Serial.print(stashed_chars[1]);
              Serial.print(" != ");
              Serial.println(stashed_chars[2]);
            }
            dec_state = PREMABLE_LOW;
            state = AWAIT_FIRST_HIGH;          
          }
        } else {
          dec_state = PREMABLE_LOW;
          state = AWAIT_FIRST_HIGH;
        }

        dec_pos = 0;
        decoded = 0;
      }
    }
  }
}

