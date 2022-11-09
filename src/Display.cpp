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
#include "Display.hpp"
#include "config.hpp"
#include "../assets/fonts/my_u8g2_font_7x13B.hpp"
#include "../assets/fonts/my_u8g2_font_open_iconic_embedded_2x.hpp"
#include "../assets/fonts/my_u8g2_font_fur20.hpp"

/*
 * 0.96" Display. Two colored version has:
 *   16 pixel i.e. yellow
 *    1 non-counted empty pixel row
 *   48 pixel i.e. blue
 */

Display::Display(uint16_t interval_ms, Thermocouple *TcPtr, Hotplate *HotPtr) : Runnable(interval_ms), u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE)
{
    _TcPtr = TcPtr;
    _HotPtr = HotPtr;
}

void Display::setup()
{
    u8g2.begin(ROTARY_S_PIN, ROTARY_A_PIN, ROTARY_B_PIN);
    u8g2.clear();
}

void Display::mainScreen()
{
    // Check if we need to make the expensive display redraw
    if (_TcPtr->getTemperature() == _lastTemp &&
        _HotPtr->getSetpoint() == _lastTarget &&
        _HotPtr->getPower() == _lastPower &&
        _HotPtr->getProfileTimePosition() == _lastProfileTimePosition)
    {
        return;
    }

    char cbuf[12]; // Longest entry length = "Target: 123\0"
    String s = "";
    u8g2_uint_t y;

    u8g2.firstPage();
    do
    {
        // Standard (small font)
        setStdFont();

        // Profile / Mode
        u8g2.drawStr(0, 13, _HotPtr->profile2str[Config::active.profile]);

        // 2nd row
        y = 27;
        if (Config::active.profile != Hotplate::Profile::Manual &&
            _HotPtr->getControllerState() == Hotplate::ControllerState::off)
        {
            s = "Push to start";
            u8g2.drawStr((u8g2.getDisplayWidth() - u8g2.getStrWidth(s.c_str())) / 2, y, s.c_str());
        }
        else
        {
            // Target temperature
            _lastTarget = _HotPtr->getSetpoint();
            sprintf(cbuf, "Target: %3d", _lastTarget);
            u8g2.drawFrame(u8g2.getStrWidth(cbuf) - (3 * 7) - 2, 16, (3 * 7) + 4, 13);
            u8g2.drawStr(0, y, cbuf);
            if (Config::active.profile != Hotplate::Profile::Manual)
            {
                sprintf(cbuf, "%3ds", _HotPtr->getProfileTimePosition());
                u8g2.drawStr(85, y, cbuf);
            }
        }

        // Controller state
        switch (_HotPtr->getControllerState())
        {
        case Hotplate::ControllerState::bangOn:
            s = "BangON";
            u8g2.setFontMode(0);
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, 29, u8g2.getStrWidth(s.c_str()) + 2, 13);
            u8g2.setDrawColor(0);
            u8g2.drawStr(1, 40, s.c_str());
            break;
        case Hotplate::ControllerState::pid:
            s = "PID ";
            s += _HotPtr->getOutput();
            s += "/";
            s += Config::active.pid_pwm_window_ms;
            u8g2.drawStr(1, 40, s.c_str());
            break;
        case Hotplate::ControllerState::bangOff:
            u8g2.drawStr(1, 40, "BangOFF");
            break;
        default:
            break;
        }

        // Unit
        u8g2.setDrawColor(1);
        u8g2.drawUTF8(108, 62, "°C");

        // ----- Large font stuff -----
        u8g2.setFont(my_u8g2_font_fur20);

        // Temperature
        dtostrf(_TcPtr->getTemperature(), 5, 1, cbuf);
        s = String(cbuf);
        s.concat(" °C");
        u8g2.drawUTF8(35, 64, s.c_str());
        _lastTemp = _TcPtr->getTemperature();

        // SSR Power
        _lastPower = _HotPtr->getPower();
        if (_lastPower)
        {
            u8g2.setFont(my_u8g2_font_open_iconic_embedded_2x);
            u8g2.drawStr(5, 62, "C"); // Power symbol = 67 = C
        }

    } while (u8g2.nextPage());
}

void Display::changeUiMode(uiMode nextUi)
{
    uiM = nextUi;
}

void Display::userInterfaceInputDouble(const char *title, const char *pre, double *value, uint8_t numInt, uint8_t numDec, const char *post)
{
    uint8_t iV;
    String valueStr = "", prePart, postPart;

    // Double -> String
    char cbuf[numInt + numDec + 1];
    if (*value == 0)
    {
        for (uint8_t i = 0; i < numInt; i++)
            valueStr += "0";
        valueStr += ".";
        for (uint8_t i = 0; i < numDec; i++)
            valueStr += "0";
    }
    else
    {
        dtostrf(*value, numInt + numDec + 1, numDec, cbuf);
        valueStr = String(cbuf);
    }

    // Serial.print("valueStr: ");
    // Serial.println(valueStr);

    // Loop over als string chars
    for (uint8_t i = 0; i < valueStr.length(); i++)
    {
        if (valueStr[i] == '.')
            continue;

        if (valueStr[i] == ' ')
            valueStr[i] = '0';

        prePart = pre;
        prePart += valueStr.substring(0, i);
        prePart += "[";

        iV = valueStr[i] & 0x0f;

        postPart = "]";
        postPart += valueStr.substring(i + 1);

        /*
        Serial.print("i: ");
        Serial.print(i, HEX);
        Serial.print(", pre: '");
        Serial.print(pre);
        Serial.print("', prePart: '");
        Serial.print(prePart);
        Serial.print("', edit digit ");
        Serial.print(iV, HEX);
        Serial.print(", postPart: '");
        Serial.print(postPart);
        Serial.println("'");
        */

        if (u8g2.userInterfaceInputValue(title, prePart.c_str(), &iV, 0, 9, 1, postPart.c_str()) == 0)
            return; // We don't have a "home" (escape) button yet
        valueStr[i] = iV + 0x30;
    }
    *value = valueStr.toDouble();
}

void Display::inputBangValues()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        // FIXME: The following should go into a loop with a mapped "Kx" string and config var
        if (u8g2.userInterfaceInputValue("Bang-ON until\ntarget-temp", "minus ", &Config::active.pid_bangOn_temp_c, 0, 255, 3, " °C") == 0)
            return; // We don't have a "home" (escape) button yet
        if (u8g2.userInterfaceInputValue("Bang-OFF at\ntarget-temp", "plus ", &Config::active.pid_bangOff_temp_c, 0, 255, 3, " °C") == 0)
            return; // We don't have a "home" (escape) button yet
    } while (u8g2.nextPage());
}

void Display::inputPidConstants()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        // FIXME: The following should go into a loop with a mapped "Kx" string and config var
        const char *title = "Select\nPID constant";
        userInterfaceInputDouble(title, "Kp = ", &Config::active.pid_Kp, 3, 1, "");
        userInterfaceInputDouble(title, "Ki = ", &Config::active.pid_Ki, 3, 1, "");
        userInterfaceInputDouble(title, "Kd = ", &Config::active.pid_Kd, 3, 1, "");
    } while (u8g2.nextPage());
}

void Display::setStdFont()
{
    // u8g2.setFont(my_u8g2_font_8x13B);
    u8g2.setFont(my_u8g2_font_7x13B);
}

void Display::inputSsrType()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        uint8_t sel = u8g2.userInterfaceSelectionList("Setup SSR Type", (Config::active.ssr_active_low ? 1 : 2), "Active Low\nActive High");
        switch (sel)
        {
        case 1: // Active Low
            Config::active.ssr_active_low = true;
            break;
        case 2: // Active High
            Config::active.ssr_active_low = false;
            break;
        }
    } while (u8g2.nextPage());
}

void Display::inputReflowProfile()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        String modeList = "";

        for (uint8_t i = 0; i < sizeof(Hotplate::profile2str) / sizeof(char *); ++i)
        {
            if (i > 0)
                modeList += "\n";
            modeList += _HotPtr->profile2str[i];
        }

        uint8_t sel = u8g2.userInterfaceSelectionList("Setup Profile", Config::active.profile + 1, modeList.c_str());
        Config::active.profile = static_cast<Hotplate::Profile>(sel - 1);
    } while (u8g2.nextPage());
}

void Display::setupScreen()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        String s = "Setup (";
        s += VERSION_TEXT;
        s += ")";

        /* Setup
         * -----
         * Reflow Profile
         * Display unit
         * SSR Type
         * PID Coefficients
         * Calibration
         * Save to EEPROM
         * Quit
         */
        uint8_t sel = u8g2.userInterfaceSelectionList(s.c_str(), 1,
                                                      "Reflow Profile\n(Display unit)\nSSR Type\nPID constants\nBangBang\n(Calibration)\nLoad saved\nSave & Quit\nQuit");
        //                                                    1               2              3         4             5           6             7            8        9
        switch (sel)
        {
        case 1: // Reflow Profile
            inputReflowProfile();
            break;
        case 3: // SSR Type
            inputSsrType();
            break;
        case 4: // PID constants
            inputPidConstants();
            _HotPtr->updatePidGains();
            break;
        case 5: // BangBang values
            inputBangValues();
            break;
        case 7: // Load saved
            Config::load();
            break;
        case 8: // Save & Quit
            Config::save();
            changeUiMode(uiMode::Main);
            break;
        default:
            changeUiMode(uiMode::Main);
        }
    } while (u8g2.nextPage());
}

void Display::loop()
{
    switch (uiM)
    {
    case uiMode::Setup:
        setupScreen();
        /*
                        // Double -> String
                        char cbuf[10];
                        dtostrf(conf->pid_Kp, 8, 2, cbuf);
                        Serial.print("Before: ");
                        Serial.println(cbuf);

                        setStdFont();
                        inputPidConstants();

                        // Double -> String
                        dtostrf(conf->pid_Kp, 8, 2, cbuf);
                        Serial.print("After: ");
                        Serial.println(cbuf);
        */
        break;
    default:
        mainScreen();
    }
}