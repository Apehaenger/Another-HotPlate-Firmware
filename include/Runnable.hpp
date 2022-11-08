#ifndef Runnable_h
#define Runnable_h

#include "Thermocouple.hpp"

/*
 * A "Runnable" super-class
 * Motivated/copied from https://paulmurraycbr.github.io/ArduinoTheOOWay.html
 */
class Runnable
{
public:
    Runnable(uint32_t interval_ms);

    virtual void setup() = 0;
    virtual void loop(Thermocouple *) = 0;

    static void setupAll();
    static void loopAll(Thermocouple *);

private:
    static Runnable *headRunnable;
    Runnable *nextRunnable;
    uint32_t _nextInterval_ms;
    uint16_t _interval_ms;
};

#endif