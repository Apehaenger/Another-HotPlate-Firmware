/*
 * This file is part of the Another-Reflow-HotPlate-Firmware project (https://github.com/Apehaenger/Another-Reflow-HotPlate-Firmware).
 * Copyright (c) 2022 Jörg Ebeling
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef Runnable_h
#define Runnable_h

#include <Arduino.h>

/*
 * A "Runnable" super-class
 * Motivated/copied from https://paulmurraycbr.github.io/ArduinoTheOOWay.html
 */
class Runnable
{
    static Runnable *headRunnable;

    Runnable *_nextRunnable;
    uint32_t _nextInterval_ms;
    const uint16_t _interval_ms;

public:
    Runnable(uint32_t interval_ms) : _interval_ms(interval_ms)
    {
        _nextRunnable = headRunnable;
        headRunnable = this;
    }

    virtual void setup() = 0;
    virtual void loop() = 0;

    static void setupAll()
    {
        for (Runnable *r = headRunnable; r; r = r->_nextRunnable)
            r->setup();
    } 

    static void loopAll()
    {
        for (Runnable *r = headRunnable; r; r = r->_nextRunnable)
        {
            uint32_t now = millis();
            if (now >= r->_nextInterval_ms)
            {
                r->_nextInterval_ms = now + r->_interval_ms;
                r->loop();
            }
        }
    }
};

#endif