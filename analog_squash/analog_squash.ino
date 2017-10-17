
int sensorPin = A0;    // select the input pin for the potentiometer

long sum = 0;
int count = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = analogRead(sensorPin);
  
  sum += val;
  ++count;

  if (count == 100) {
    Serial.println(sum/count);
    sum = 0;
    count = 0;
  }
}

