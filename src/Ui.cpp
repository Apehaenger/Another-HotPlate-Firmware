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
#include "CRC32.h"
#include "main.hpp"
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
Ui::Ui() : u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE) {}

void Ui::setup()
{
    u8g2.begin(ROTARY_S_PIN, ROTARY_A_PIN, ROTARY_B_PIN);
    u8g2.clear();
}

void Ui::displayMainScreen()
{
    // CRC32 of main screen relevant values
    // Do it via struct and a single crc.calculate() call instead of multiple crc.update() calls which would be more expensive!
    MainScreenCrcData crcData = {hotplate.getMode(), hotplate.getPower(), hotplate.getSetpoint(), hotplate.getState(),
                                 profile.getSecondsLeft(), thermocouple.getTemperatureAverage()};
    uint32_t mainScreenCrc = CRC32::calculate((uint8_t *)&crcData, sizeof(crcData));

#ifdef DEBUG_UI_SERIAL
    Serial.print("mainScreenCrc: ");
    Serial.print(mainScreenCrc, HEX);
    Serial.print(", _lastMainScreenCrc: ");
    Serial.println(_lastMainScreenCrc, HEX);
#endif

    // Check if we need to make the expensive display redraw
    if (mainScreenCrc == _lastMainScreenCrc)
    {
        return;
    }
    _lastMainScreenCrc = mainScreenCrc;

    char cbuf[14]; // Longest entry length = "PID ????/5000\0", before "Target: 123\0"
    String s;
    u8g2_uint_t x, y;

    u8g2.firstPage();
    do
    {
        // Standard (small font)
        setStdFont();

        // 1st row
        y = 9;
        // Mode
        switch (hotplate.getMode())
        {
        case Hotplate::Mode::PIDTuner:
            u8g2.drawStr(0, y, "PID Tuner:");
            x = 71;
            switch (hotplate.getState())
            {
            case Hotplate::State::Wait:
                u8g2.drawStr(x, y, "Wait...");
                break;
            case Hotplate::State::Heat:
                u8g2.drawStr(x, y, "Heat...");
                break;
            case Hotplate::State::Settle:
                u8g2.drawStr(x, y, "Settle...");
                break;
            default:
                break;
            }
            break;
        default:
            u8g2.drawStr(0, y, profile.profile2str[Config::active.profile]);
            break;
        }

        // 2nd row
        y = 25; // For two color display need to be >= 25
        if (hotplate.isStandBy() || (!hotplate.isMode(Hotplate::Mode::PIDTuner) && profile.isStandBy()))
        {
            const char *cp = "Push to start";
            u8g2.drawStr((u8g2.getDisplayWidth() - u8g2.getStrWidth(cp)) / 2, y, cp);
        }
        else
        {
            // Target temperature
            sprintf(cbuf, "Target: %3d", hotplate.getSetpoint());
            // u8g2.drawFrame(u8g2.getStrWidth(cbuf) - (3 * 7) - 3, y - 12, (3 * 7) + 6, 15);
            u8g2.drawStr(0, y, cbuf);
            if (Config::active.profile != Profile::Profiles::Manual && !hotplate.isMode(Hotplate::Mode::PIDTuner))
            {
                sprintf(cbuf, "%3ds", profile.getSecondsLeft());
                u8g2.drawStr(85, y, cbuf);
            }
        }

        // Row 3 = Controller state
        x = 1;
        y = 40;
        switch (hotplate.getState())
        {
        case Hotplate::State::BangOn:
            strcpy(cbuf, "BangON");
            u8g2.setFontMode(0);
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, 29, u8g2.getStrWidth(cbuf) + 2, 13);
            u8g2.setDrawColor(0);
            u8g2.drawStr(x, y, cbuf);
            break;
        case Hotplate::State::PID:
            sprintf(cbuf, "PID %5d/%5d", hotplate.getOutput(), Config::active.pid_pwm_window_ms);
            u8g2.drawStr(x, y, cbuf);
            break;
        case Hotplate::State::BangOff:
            u8g2.drawStr(x, y, "BangOFF");
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
        dtostrf(thermocouple.getTemperatureAverage(), 5, 1, cbuf);
        u8g2.drawUTF8(35, 64, cbuf);

        // SSR Power
        if (hotplate.getPower())
        {
            u8g2.setFont(my_u8g2_font_open_iconic_embedded_2x);
            u8g2.drawStr(5, 62, "C"); // Power symbol = 67 = C
        }

    } while (u8g2.nextPage());
}

void Ui::userInterfaceInputDouble(const char *title, const char *pre, double *value, uint8_t numInt, uint8_t numDec, const char *post)
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
        valueStr.replace(' ', '0');
    }

    // Serial.print("valueStr: ");
    // Serial.println(valueStr);

    // Loop over als string chars
    for (uint8_t i = 0; i < valueStr.length(); i++)
    {
        if (valueStr[i] == '.')
            continue;

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

void Ui::inputBangValues()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        if (u8g2.userInterfaceInputValue("Bang-ON until\ntarget-temp", "minus ", &Config::active.pid_bangOn_temp_c, 0, 255, 3, " °C") == 0)
            return; // We don't have a "home" (escape) button yet
        if (u8g2.userInterfaceInputValue("Bang-OFF at\ntarget-temp", "plus ", &Config::active.pid_bangOff_temp_c, 0, 255, 3, " °C") == 0)
            return; // We don't have a "home" (escape) button yet
    } while (u8g2.nextPage());
}

void Ui::inputMaxTemp()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        if (u8g2.userInterfaceInputValue("Set max.", "Temperature ", &Config::active.max_temp_c, 0, 255, 3, " °C") == 0)
            return; // We don't have a "home" (escape) button yet
    } while (u8g2.nextPage());
}

void Ui::inputPidConstants()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        const char *title = "Select\nPID constant";
        userInterfaceInputDouble(title, "Kp = ", &Config::active.pid_Kp, 4, 1, "");
        userInterfaceInputDouble(title, "Ki = ", &Config::active.pid_Ki, 4, 1, "");
        userInterfaceInputDouble(title, "Kd = ", &Config::active.pid_Kd, 4, 1, "");
    } while (u8g2.nextPage());
}

void Ui::setStdFont()
{
    // u8g2.setFont(my_u8g2_font_8x13B);
    u8g2.setFont(my_u8g2_font_7x13B);
}

void Ui::inputSsrType()
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

void Ui::inputReflowProfile()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        String modeList = "";

        for (uint8_t i = 0; i < sizeof(Profile::profile2str) / sizeof(char *); ++i)
        {
            if (i > 0)
                modeList += "\n";
            modeList += profile.profile2str[i];
        }

        uint8_t sel = u8g2.userInterfaceSelectionList("Setup Profile", Config::active.profile + 1, modeList.c_str());
        Config::active.profile = static_cast<Profile::Profiles>(sel - 1);
    } while (u8g2.nextPage());
}

void Ui::displaySetupScreen()
{
    u8g2.firstPage();
    do
    {
        setStdFont();
        String s = "Setup (";
        s += VERSION_TEXT;
        s += ")";

        uint8_t sel = u8g2.userInterfaceSelectionList(s.c_str(), 1,
                                                      "Reflow Profile\n(Display unit)\nSSR Type\nMax. Temperature\nPID constants\nBangBang\nPID Tuner\nLoad saved\nSave & Quit\nQuit");
        //                                                    1               2              3         4                 5           6          7           8          9         10
        switch (sel)
        {
        case 1: // Reflow Profile
            inputReflowProfile();
            break;
        case 3: // SSR Type
            inputSsrType();
            break;
        case 4: // BangBang values
            inputMaxTemp();
            break;
        case 5: // PID constants
            inputPidConstants();
            hotplate.updatePidGains();
            break;
        case 6: // BangBang values
            inputBangValues();
            break;
        case 7: // PID Tuner
            hotplate.setMode(Hotplate::Mode::PIDTuner);
            changeMode(Mode::Main);
            break;
        case 8: // Load saved
            Config::load();
            break;
        case 9: // Save & Quit
            Config::save();
            changeMode(Mode::Main);
            break;
        default:
            changeMode(Mode::Main);
        }
    } while (u8g2.nextPage());
}

void Ui::loop()
{
    uint32_t now = millis();
    if (now < _nextInterval_ms)
    {
        return;
    }
    _nextInterval_ms = now + INTERVAL_DISP;

    switch (_mode)
    {
    case Mode::Setup:
        displaySetupScreen();
        break;
    default:
        displayMainScreen();
    }
}