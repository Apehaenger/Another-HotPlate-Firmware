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

Hotplate::Mode Hotplate::getMode()
{
    return _mode;
}

bool Hotplate::isMode(Mode mode)
{
    return _mode == mode;
}

void Hotplate::setMode(Mode mode)
{
    _mode = mode;
}

Hotplate::State Hotplate::getState()
{
    return _state;
}

bool Hotplate::isState(State state)
{
    return _state == state;
}

void Hotplate::setState(State state)
{
    _state = state;
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
        _mode = Mode::Manual;
        _state = State::Idle;
        _myPID.stop();
        _output = 0;
        setPower(false); // Don't wait for the next loop()
        return;
    }

    if (_state == State::Idle)
    {
        _state = State::PID;
    }
    _myPID.run();
    _pwmWindowStart_ms = millis();
}

/**
 * @brief Update PID controllers PID gains
 */
void Hotplate::updatePidGains()
{
    _myPID.setGains(Config::active.pid_Kp, Config::active.pid_Ki, Config::active.pid_Kd);
}

/**
 * @brief Is the current mode/setting a startable process (like PIDTuner or ReflowProfile), and idle (waiting to get started)?
 *
 * @return true if there is an idle process
 * @return false if not
 */
bool Hotplate::isIdleProcess()
{
    return ((isMode(Mode::PIDTuner) || Config::active.profile != Profile::Profiles::Manual) &&
            isState(State::Idle));
}

void serialPrintLine()
{
    Serial.println("-------------------");
}

void Hotplate::loop()
{
    uint32_t now = millis();
    _input = thermocouple.getTemperatureAverage();

    switch (_state)
    {
    case State::Idle: // Wait for "Press start"
        setPower(false);
        return;
    case State::PID:
    case State::BangOn:
    case State::BangOff:
        _myPID.run();
        // Informative state changes. Logic copied from AutoPID.cpp
        if (Config::active.pid_bangOn_temp_c && ((_setpoint - _input) > Config::active.pid_bangOn_temp_c))
            _state = State::BangOn;
        else if (Config::active.pid_bangOff_temp_c && ((_input - _setpoint) > Config::active.pid_bangOff_temp_c))
            _state = State::BangOff;
        else
            _state = State::PID;
        break;
    case State::Start: // Start/Init PID Tuner
        Serial.println("Copy & Paste to https://pidtuner.com");
        Serial.println("Time, Input, Output");
        serialPrintLine();
        // PID Tuner calculations may fail if not started with 0
        Serial.print("-10.00, 0, ");
        Serial.println(_input);
        Serial.print("-0.01, 0, ");
        Serial.println(_input);

        _setpoint = _pidTunerTempTarget;
        _pidTunerStart_ms = now;
        _pwmWindowStart_ms = now;
        _state = State::Heating;
        _output = Config::active.pid_pwm_window_ms;
        break;
    case State::Heating:
        if (_input > _setpoint) // By the use of _input the user may adapt the target during heatup
        {
            _pidTunerTempTarget = _setpoint;
            _setpoint = 0;
            _output = 0;
            _pwmWindowStart_ms = now;
            _pidTunerTempMax = _input; // Useless as it will overshoot in any case?!
            _state = State::Settle;
        }
        break;
    case State::Settle:
        if (_input > _pidTunerTempMax) // overshooting
        {
            _pidTunerTempMax = _input;
            break;
        }
        // if(_input <= (_pidTunerTempMax - _pidTunerTempNoise - _pidTunerTempSettled)) // Settled
        if (_input <= _pidTunerTempTarget) // Settled
        {
            _state = State::Idle;
            _mode = Mode::Manual;
            serialPrintLine();
            Serial.print("Done. Overshot (BangON) = ");
            Serial.println(_pidTunerTempMax - _pidTunerTempTarget);
        }
        break;
    }

    // Soft PWM
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
    Serial.print(_state);
    Serial.print(", Output: ");
    Serial.print(_output);
    Serial.print(", SSR: ");
    Serial.println(_power);
#endif

    if (isMode(Mode::PIDTuner) && !isState(State::Idle) &&
        now >= _pidTunerOutputNext_ms)
    {
        _pidTunerOutputNext_ms = now + PID_TUNER_INTERVAL_MS;
        Serial.print((float)(now - _pidTunerStart_ms) / 1000);
        Serial.print(", ");
        Serial.print(_output);
        Serial.print(", ");
        Serial.println(_input);
    }
}