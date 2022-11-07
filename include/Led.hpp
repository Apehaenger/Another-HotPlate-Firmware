#ifndef Led_h
#define Led_h

#include <Arduino.h>

#define LED_WARM_TEMP 50     // > temp for Warm LED FIXME: Need to become dynamic on C/F unit
#define LED_WARM_MILLIS 1000 // Hot-LED blink rate (ms)

#define LED_HOT_TEMP 100   // > temp for Warm LED FIXME: Need to become dynamic on C/F unit
#define LED_HOT_MILLIS 500 // Hot-LED blink rate (ms)

class Led
{
public:
    Led(uint8_t pin);

    void blinkByTemp(double temp);

private:
    uint8_t _pin;
};

#endif