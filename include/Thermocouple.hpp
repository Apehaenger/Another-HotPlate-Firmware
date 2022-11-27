#ifndef Thermocouple_h
#define Thermocouple_h

#include <max6675.h>

#define TC_MAX_READ_INTERVAL_MS 220 // MAX6675 module can read values every 170-220 ms
#define TC_AVG_SAMPLES 5            // How much samples to use for average calculation. 5 sample @ 220ms result in a max temp delay of approx. 1s

/*
 * Simple Thermocouple wrapper around MAX6675. Mainly to be open for other TC controller...
 * Later, one might implement a bridge pattern
 */
class Thermocouple
{
public:
    Thermocouple(const int8_t pin_CLK, const int8_t pin_CS, const int8_t pin_DO);
    float getTemperature();
    float getTemperatureAverage();

private:
    MAX6675 _Tc;
    uint32_t _nextRead_ms = 0;
    float _lastTemp, _avgTemp;

    void readTemperature();
};

#endif