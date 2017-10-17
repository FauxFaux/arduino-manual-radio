
int sensorPin = A0;    // select the input pin for the potentiometer

#define end 10
int range[end];
int pos = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = analogRead(sensorPin);
  pos = (pos + 1) % end;
  range[pos] = val;

  if (pos == 0) {
    int total = 0;

    for (int i = 0; i < end; ++i)
      total += range[i];

 
    Serial.println(total / end);
  }
}

