
#ifndef LightSensor_h
#define LightSensor_h

#include "Arduino.h"

class LightSensor
{
  public:
    LightSensor(int pin, int sensorValue, int triggerValue);
    void triggered();
  private:
    int _pin;
    int _sensorValue;
    int _triggerValue;
};

#endif