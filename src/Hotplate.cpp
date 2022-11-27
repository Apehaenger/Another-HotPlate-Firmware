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

Hotplate::Hotplate(uint8_t ssr_pin) : _ssrPin(ssr_pin),
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
        _state = State::StandBy;
        _myPID.stop();
        _output = 0;
        setPower(false); // Don't wait for the next loop()
        return;
    }

    if (_state == State::StandBy)
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

bool Hotplate::pwmWindowReached()
{
    return (millis() - _pwmWindowStart_ms > Config::active.pid_pwm_window_ms);
}

void serialPrintLine()
{
    Serial.println("-------------------");
}

void Hotplate::loop()
{
    uint32_t now = millis();
    if (now < _nextInterval_ms)
    {
        return;
    }
    _nextInterval_ms = now + PID_SAMPLE_MS;

    _input = thermocouple.getTemperatureAverage();

    switch (_state)
    {
    case State::StandBy: // Wait for "Press start"
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
        _pwmWindowStart_ms = now;
        _state = State::Wait;
        break;
    case State::Wait: // Wait one pwmWindow before start. PID Tuner calculations may fail if not started with 0 output
        if (!pwmWindowReached())
        {
            break;
        }
        _pidTunerTempTarget = _input + PID_TUNER_TEMP_STEPS_C;
        _setpoint = _pidTunerTempTarget;
        _state = State::Heat;
        _output = Config::active.pid_pwm_window_ms;
        break;
    case State::Heat:
        if (_input > _setpoint) // By the use of _input the user may adapt the target during heatup
        {
            _setpoint = 0;
            _output = 0;
            _pwmWindowStart_ms = now;
            _pidTunerTempMax = _input;
            _state = State::Settle;
        }
        break;
    case State::Settle:
        if (_input > _pidTunerTempMax) // overshooting
        {
            _pidTunerTempMax = _input;
            break;
        }
        if (_input <= (_pidTunerTempMax - PID_TUNER_TEMP_SETTLED_C)) // Settled
        {
            if (_input + PID_TUNER_TEMP_STEPS_C < Config::active.max_temp_c)
            {
                // One more step
                _state = State::Wait;
                break;
            }
            _state = State::StandBy;
            _mode = Mode::Manual;
            serialPrintLine();
            Serial.print("Done. Last step overshot (BangON) = ");
            Serial.println(_pidTunerTempMax - _pidTunerTempTarget);
        }
        break;
    }

    // Soft PWM
    if (pwmWindowReached())
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

    if (isMode(Mode::PIDTuner) && !isState(State::StandBy) &&
        now >= _pidTunerOutputNext_ms)
    {
        _pidTunerOutputNext_ms = now + PID_TUNER_INTERVAL_MS;
        Serial.print((float)now / 1000);
        Serial.print(", ");
        Serial.print(_output);
        Serial.print(", ");
        Serial.println(_input);
    }
}