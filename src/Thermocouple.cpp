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
#include "Thermocouple.hpp"

Thermocouple::Thermocouple(const int8_t pin_CLK, const int8_t pin_CS, const int8_t pin_DO) : _Tc(pin_CLK, pin_CS, pin_DO) {}

void Thermocouple::readTemperature()
{
    if ((millis() < _nextRead_ms))
    {
        return;
    }
    _nextRead_ms = millis() + TC_MAX_READ_INTERVAL_MS;

    // TODO: Read temperate dependent on EEPROM unit setting (C/F)
    _lastTemp = _Tc.readCelsius();

    // Cumulative (rolling) average temperature of the last TC_AVG_SAMPLES
    // See https://en.wikipedia.org/wiki/Moving_average#Cumulative_average
    _avgTemp -= _avgTemp / TC_AVG_SAMPLES;
    _avgTemp += _lastTemp / TC_AVG_SAMPLES;
}

float Thermocouple::getTemperature()
{
    readTemperature();
    return _lastTemp;
}

float Thermocouple::getTemperatureAverage()
{
    readTemperature();
    return _avgTemp;
}
