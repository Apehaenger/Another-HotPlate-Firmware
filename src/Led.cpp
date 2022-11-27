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
#include "main.hpp"
#include "Led.hpp"

Led::Led(uint8_t pin) : _pin(pin)
{
    pinMode(pin, OUTPUT);
}

void Led::blinkByTemp(const float temp)
{
    uint32_t currentMillis = millis();
    if (temp >= (LED_WARM_TEMP + LED_HOT_TEMP))
    {
        digitalWrite(_pin, 1); // Very-Hot LED
        return;
    }
    if (temp >= LED_HOT_TEMP)
    {
        digitalWrite(_pin, (currentMillis / LED_HOT_MILLIS) % 2); // Hot LED
        return;
    }
    if (temp >= LED_WARM_TEMP)
    {
        digitalWrite(_pin, (currentMillis / LED_WARM_MILLIS) % 2); // Warm LED
        return;
    }
    digitalWrite(_pin, 0);
}