#include <LightSensor.h>

LightSensor input(A2,0,700);

void setup() {
  Serial.begin(9600);
}

void loop() {
  // read the value from the sensor:
  input.triggered();
  delay(100);
}
