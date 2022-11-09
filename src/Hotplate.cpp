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
#include "Hotplate.hpp"
#include "config.hpp"

Hotplate::Hotplate(uint16_t interval_ms, uint8_t ssr_pin, Thermocouple *TcPtr) : Runnable(interval_ms), _myPID(&_input, &_setpoint, &_output,
                                                                                                               0, Config::active.pid_pwm_window_ms,
                                                                                                               Config::active.pid_Kp, Config::active.pid_Ki, Config::active.pid_Kd)
{
    _ssrPin = ssr_pin;
    _TcPtr = TcPtr;
}

void Hotplate::setup()
{
    pinMode(_ssrPin, OUTPUT);
    setPower(false); // Be sure it's off

    _myPID.setBangBang(Config::active.pid_bangOn_temp_c, Config::active.pid_bangOff_temp_c);
    _myPID.setTimeStep(PID_SAMPLE_MS); // time interval at which PID calculations are allowed to run in milliseconds
}

Hotplate::ControllerState Hotplate::getControllerState()
{
    return _controllerState;
}

uint16_t Hotplate::getOutput()
{
    return _output;
}

bool Hotplate::getPower()
{
    return _power;
}

uint16_t Hotplate::getSetpoint()
{
    return _setpoint;
}

void Hotplate::runProfile()
{
    _state = State::On;
    _profileStart_ms = millis();
    _profileNext_ms = _profileStart_ms;
}

short Hotplate::getProfileTimePosition()
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

/*
 * Get profile time/temp target dependent of current time/temp
 * @return -1 in the case where PROFILE_TIME_INTERVAL is not reached or profile ended
 */
short Hotplate::getProfileTemp()
{
    uint32_t now = millis();
    short nextTemp;

    if (_profileNext_ms && now < _profileNext_ms)
    {
        return -1;
    }
    _profileNext_ms += PROFILE_TIME_INTERVAL_MS;

    ProfileTimeTarget timeTarget;

    for (uint8_t i = 0; i < _profile2timeTargets[Config::active.profile]->size; i++)
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
            _input > timeTarget.temp_c) // FIXME: This would result in a wrong profile time left display if already hot
        {
            continue; // Already beyond this time or temp target
        }

        // In-line math in C sucks :-/
        float tempDiff = timeTarget.temp_c - _input;
        float timeLeft_ms = (1000.0 * timeTarget.time_s) + _profileStart_ms - now;
        float intervalsProfile = (1000.0 * timeTarget.time_s) / PROFILE_TIME_INTERVAL_MS;
        float intervalsLeft = (timeLeft_ms / PROFILE_TIME_INTERVAL_MS) - 1;

        nextTemp = _input + round(tempDiff / intervalsProfile * (intervalsProfile - intervalsLeft));

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
    _profileNext_ms = 0; // Stop looping through profile array
    return 0;            // Profile ended
}

void Hotplate::setPower(bool pow)
{
    digitalWrite(_ssrPin, (pow ^ Config::active.ssr_active_low) ? HIGH : LOW);
    _power = pow;
}

void Hotplate::setSetpoint(uint16_t setpoint)
{
    _setpoint = setpoint;

    if (_setpoint)
    {
        //_myPID.stop();
        _myPID.run();
        _state = State::On;
        _pwmWindowStart_ms = millis();
    }
    else
    {
        _state = State::Off;
        _controllerState = ControllerState::off;
        _myPID.stop();
        _output = 0;
        setPower(false); // Don't wait for the next loop()
    }
}

/**
 * @brief Update PID controllers PID gains from (probably actualized) config
 */
void Hotplate::updatePidGains()
{
    _myPID.setGains(Config::active.pid_Kp, Config::active.pid_Ki, Config::active.pid_Kd);
}

void Hotplate::loop()
{
    short nextTemp;
    _input = _TcPtr->getTemperature();

    switch (_state)
    {
    case State::Off:
        setPower(false);
        return;
        ;
    case State::On:
        if (Config::active.profile != Hotplate::Profile::Manual)
        {
            nextTemp = getProfileTemp();
            if (nextTemp > 0) // Allow manual correction per interval, and do not stop at end
            {
                setSetpoint(nextTemp);
            }
        }
        _myPID.run();
        break;
    }

    // Informative state changes. Logic copied from AutoPID.cpp
    if (Config::active.pid_bangOn_temp_c && ((_setpoint - _input) > Config::active.pid_bangOn_temp_c))
        _controllerState = ControllerState::bangOn;
    else if (Config::active.pid_bangOff_temp_c && ((_input - _setpoint) > Config::active.pid_bangOff_temp_c))
        _controllerState = ControllerState::bangOff;
    else
        _controllerState = ControllerState::pid;

    // Soft PWM
    uint32_t now = millis();
    if (now - _pwmWindowStart_ms > Config::active.pid_pwm_window_ms)
    { // time to shift the Relay Window
        _pwmWindowStart_ms += Config::active.pid_pwm_window_ms;
    }
    setPower(_output > now - _pwmWindowStart_ms);

#ifdef DEBUG_SERIAL_OFF
    Serial.print("Setpoint: ");
    Serial.print(_setpoint);
    Serial.print(", Input: ");
    Serial.print(_input);
    Serial.print(", Controller state: ");
    Serial.print(_controllerState);
    Serial.print(", Output: ");
    Serial.print(_output);
    Serial.print(", SSR: ");
    Serial.println(_power);
#endif

#ifdef DEBUG_SERIAL_PIDTUNER
    if (_state == State::On && now >= _pidTunerNext_ms)
    {
        _pidTunerNext_ms = now + PID_TUNER_INTERVAL_MS;
        Serial.print(now);
        Serial.print(", ");
        Serial.print(_power);
        Serial.print(", ");
        Serial.println(_input);
    }
#endif
}