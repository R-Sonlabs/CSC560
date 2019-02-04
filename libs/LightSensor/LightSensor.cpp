#include "LightSensor.h"

LightSensor::LightSensor(int pin, int sensorValue, int triggerValue)
{
   _pin = pin;
   _sensorValue = sensorValue;
   _triggerValue = triggerValue;
}

void LightSensor::triggered()
{
  _sensorValue = analogRead(_pin);
  Serial.println(_sensorValue);
  if(_sensorValue <= _triggerValue){
    Serial.println("Triggered");
    // while(true){
      
    // }
  }
}
