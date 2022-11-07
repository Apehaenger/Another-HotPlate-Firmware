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

Thermocouple::Thermocouple(int8_t pin_CLK, int8_t pin_CS, int8_t pin_DO) : MaxTc(pin_CLK, pin_CS, pin_DO) {}

float Thermocouple::getTemperature()
{
    if ((millis() - _lastRead_ms) < TC_MAX_READ_INTERVAL_MS)
    {
        return _lastTemp;
    }
    // TODO: Read temperate dependent on EEPROM unit setting (C/F)
    _lastTemp = MaxTc.readCelsius();
    _lastRead_ms = millis();

    return _lastTemp;
}
