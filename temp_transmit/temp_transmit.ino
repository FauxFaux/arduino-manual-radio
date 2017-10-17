#include <DHT.h>
#include <DHT_U.h>

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
DHT dht(11, DHT22);

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

void send_bit(bool hi) {
  if (hi) {
    set(LOW, HIGH); // 1
  } else {
    set(HIGH, LOW); // 0    
  }
}

void send_char(uint8_t val) {
  bool parity = false;
  for (uint8_t i = 0; i < 8; ++i) {
    bool b = val & (1 << i);
    parity ^= b;
    send_bit(b);
  }

  send_bit(parity);
}

void send_data(const char *msg, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    send_char(msg[i]);
  }
}

void send_string(const char *msg) {
  send_data(msg, strlen(msg));
}

void loop() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    return;
  }

  digitalWrite(LED_BUILTIN, HIGH); 

  // premable
  {
    for (int i = 0; i < 6; ++i) {
      send_bit(false);
    }

    send_bit(true);
    send_bit(true);
  }

  uint8_t int_part = (uint8_t)t;
  uint8_t dec_part = (t - (uint8_t)t) * 255;
  send_char(int_part);
  send_char(dec_part);
  send_char(int_part ^ dec_part);

  send_bit(true);
  digitalWrite(RF, LOW);
  
  digitalWrite(LED_BUILTIN, LOW);

  delay(2000);
}

