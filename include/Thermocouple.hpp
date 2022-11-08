#ifndef Thermocouple_h
#define Thermocouple_h

#include <max6675.h>

#define TC_MAX_READ_INTERVAL_MS 220 // MAX6675 module can read values every 170-220 ms

/*
 * Simple Thermocouple wrapper around MAX6675
 * Later, we might to implement a bridge pattern
 */
class Thermocouple {
public:
    Thermocouple(int8_t pin_CLK, int8_t pin_CS, int8_t pin_DO);
    float getTemperature();

private:
    MAX6675 _Tc;
    unsigned long _lastRead_ms = 0;
    float _lastTemp;
};

#endif