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
#include "config.hpp"

Hotplate::Hotplate(uint16_t interval_ms, uint8_t ssr_pin) : Runnable(interval_ms),
                                                            _ssrPin(ssr_pin),
                                                            _myPID(&_input, &_setpoint, &_output,
                                                                   0, Config::active.pid_pwm_window_ms,
                                                                   Config::active.pid_Kp, Config::active.pid_Ki, Config::active.pid_Kd)
{
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

void Hotplate::setControllerState(ControllerState state)
{
    _controllerState = state;
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

void Hotplate::setPower(bool pow)
{
    digitalWrite(_ssrPin, (pow ^ Config::active.ssr_active_low) ? HIGH : LOW);
    _power = pow;
}

void Hotplate::setSetpoint(uint16_t setpoint)
{
    _setpoint = setpoint;

    if (!_setpoint)
    {
        //_mode = Mode::Off;
        _controllerState = ControllerState::Off;
        _myPID.stop();
        _output = 0;
        setPower(false); // Don't wait for the next loop()
        return;
    }

    if (_controllerState == ControllerState::Off)
    {
        _controllerState = ControllerState::PID;
    }
    _myPID.run();
    //_mode = Mode::On;
    _pwmWindowStart_ms = millis();
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
    _input = thermocouple.getTemperatureAverage();

    switch (_controllerState)
    {
    case ControllerState::Off:
        setPower(false);
        return;
    case ControllerState::PID:
    case ControllerState::BangOn:
    case ControllerState::BangOff:
        _myPID.run();
        // Informative state changes. Logic copied from AutoPID.cpp
        if (Config::active.pid_bangOn_temp_c && ((_setpoint - _input) > Config::active.pid_bangOn_temp_c))
            _controllerState = ControllerState::BangOn;
        else if (Config::active.pid_bangOff_temp_c && ((_input - _setpoint) > Config::active.pid_bangOff_temp_c))
            _controllerState = ControllerState::BangOff;
        else
            _controllerState = ControllerState::PID;
        break;
    case ControllerState::PIDTuner:
        break;
    }

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

    if (_controllerState == ControllerState::PIDTuner && now >= _pidTunerOutputNext_ms)
    {
        _pidTunerOutputNext_ms = now + PID_TUNER_INTERVAL_MS;
        Serial.print((float)now / 1000);
        Serial.print(", ");
        Serial.print(_power);
        Serial.print(", ");
        Serial.println(_input);
    }
}