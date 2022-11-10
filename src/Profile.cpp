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
#include <Arduino.h>
#include "main.hpp"
//#include "Profile.hpp"
#include "config.hpp"

Profile::Profile(uint16_t interval_ms, Thermocouple &TcRef, Hotplate &HotRef) : Runnable(interval_ms),
                                                                                _TcRef(TcRef),
                                                                                _HotRef(HotRef)
{
}

void Profile::setup() {}

void Profile::startProfile()
{
    if(!_profileStart_ms)
    {
        _profileStart_ms = millis();
        _nextInterval_ms = 0;
    }
}

void Profile::stopProfile()
{
    _profileStart_ms = 0;
}

short Profile::getSecondsLeft()
{
    // In-line math in C sucks!
    long timePosMs = millis() - _profileStart_ms - (1000UL * _profile2timeTargets[Config::active.profile]->duration_s);
    short timePosS = round(0.001 * timePosMs);

#ifdef DEBUG_SERIAL_PROTIMPOS
    Serial.print("millis(): ");
    Serial.print(millis());
    Serial.print(", profile start(ms): ");
    Serial.print(_profileStart_ms);
    Serial.print(", profile duration(s): ");
    Serial.print(_profile2timeTargets[Config::active.profile]->duration_s);
    Serial.print(", timePosMs: ");
    Serial.print(timePosMs);
    Serial.println(", timePosS: ");
#endif

    return timePosS;
}

void Profile::loop()
{
    if (Config::active.profile == Profile::Profiles::Manual ||
        !_profileStart_ms)
    {
        return;
    }

    short nextTemp = getTempTarget();
    if(!nextTemp)
    {
        return;
    }
    hotplate.setSetpoint(nextTemp);
}

/*
 * Get profile time/temp target dependent of current time/temp
 * @return -1 in the case where PROFILE_TIME_INTERVAL is not reached or profile ended
 */
short Profile::getTempTarget()
{
    uint32_t now = millis();
    short nextTemp;

    ProfileTimeTarget timeTarget;

    for (uint8_t i = 0; i < _profile2timeTargets[Config::active.profile]->length; i++)
    {
        timeTarget = _profile2timeTargets[Config::active.profile]->timeTargets[i]; // Make code more readable

#ifdef DEBUG_SERIAL_PROTEM
        Serial.print("Profile: ");
        Serial.print(Config::active.profile);
        Serial.print(" = ");
        Serial.print(Hotplate::profile2str[Config::active.profile]);
        Serial.print(": time_s = ");
        Serial.print(timeTarget.time_s);
        Serial.print(", temp_c = ");
        Serial.println(timeTarget.temp_c);
#endif
        if ((now - _profileStart_ms) > (1000UL * timeTarget.time_s) ||
            thermocouple.getTemperatureAverage() > timeTarget.temp_c) // FIXME: This would result in a wrong profile time left display if already hot
        {
            continue; // Already beyond this time or temp target
        }

        // In-line math in C sucks :-/
        float tempDiff = timeTarget.temp_c - thermocouple.getTemperatureAverage();
        float timeLeft_ms = (1000.0 * timeTarget.time_s) + _profileStart_ms - now;
        float intervalsProfile = (1000.0 * timeTarget.time_s) / PROFILE_TIME_INTERVAL_MS;
        float intervalsLeft = (timeLeft_ms / PROFILE_TIME_INTERVAL_MS) - 1;

        nextTemp = thermocouple.getTemperatureAverage() + round(tempDiff / intervalsProfile * (intervalsProfile - intervalsLeft));

#ifdef DEBUG_SERIAL_PROTEM
        Serial.print("TimeTarget = ");
        Serial.print(i);
        Serial.print(": tempDiff = ");
        Serial.print(tempDiff);
        Serial.print(", timeLeft(ms) = ");
        Serial.print(timeLeft_ms);
        Serial.print(", intervals = ");
        Serial.print(intervalsProfile);
        Serial.print(", intervalsLeft = ");
        Serial.print(intervalsLeft);
        Serial.print(", new Setpoint = ");
        Serial.println(nextTemp);
#endif
        return nextTemp;
    }
    _profileStart_ms = 0; // Stop looping through profile array
    return 0;             // Profile ended
}