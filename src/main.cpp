/*
 * This file is part of the Another-HotPlate-Firmware project (https://github.com/Apehaenger/Another-HotPlate-Firmware).
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
/*
 * Hardware Project:
 *    Tim's Hot Plate https://www.instructables.com/Tims-Hot-Plate
 *
 * Author: Jörg Ebeling <joerg@ebeling.ws>
 *
 * Usage:
 *     RAM      |    Flash    | Comment
 * ----------------------------------------------------------------------
 * 71.9%  1473    91.8% 28186   0.3.0 new Bootloader
 * 71.9%  1473    91.8% 28186   Removed while(true) loop in void loop()
 * 72.1%  1477    90.7% 27874   Reduced open iconic to used icon
 * 72.1%  1477    86.0% 26432   Reduced 8x13B font
 * 72.1%  1477    85.7% 26328   Reduced fur20 font
 * 78.9%  1615    86.6% 26592   Profiles, profile selection & storage, switch to 7x13B
 * 77.5%  1587    86.9% 26706   + "Push to start", before global optimization
 * 77.5%  1587    86.4% 26542   Moved global target to Hotplate::_setpoint and added/implemented getter and setter
 * 77.5%  1587    86.3% 26518   Moved global ssrOn to Hotplate::_power and added/implemented getter and setter
 * 77.1%  1579    86.0% 26418   Migrated rotary.cpp|hpp into main
 * 77.1%  1579    85.5% 26280   Optimized Thermocouple and removed global mVals
 * 77.1%  1580    85.6% 26296   Moved LED to class
 * 77.4%  1585    85.9% 26398   Fixed shared vars from TC/Hotplate
 * 75.5%  1546    87.0% 26728   Finished reflow profile
 * 75.6%  1548    87.1% 26758   Changed unsigned long to uint32_t and fixed two bugs
 * 62.9%  1289    82.8% 25432   0.4 without DEBUG_SERIAL
 * 62.7%  1285    82.8% 25428   0.4.0 without DEBUG_SERIAL
 * 64.2%  1315    86.2% 26470   Wasted 1042!! Bytes for more clear Runnable implementation
 * 64.4%  1319    86.7% 26632   Temp average
 * 63.3%  1297    86.4% 26544   Optimized Hotplate
 */
#include <Arduino.h>
#include "main.hpp"
#include "config.hpp"
#include "Thermocouple.hpp"
#include "Display.hpp"
#include "Hotplate.hpp"
#include "Led.hpp"

#if defined ATMEGA328_NEW_CH340_DBG || defined ATMEGA328_NEW_FTDI_DBG
#undef DEBUG_SERIAL
#define DEBUG_AVRSTUB
#endif

#ifdef DEBUG_AVRSTUB
#include "avr8-stub.h"
#endif

// Init classes
Runnable *Runnable::headRunnable = NULL;  // Runnable super-class 
Led HotLed(LED_PIN);
Thermocouple Tc(TC_CLK_PIN, TC_CS_PIN, TC_DO_PIN);
Hotplate Hotp(PID_SAMPLE_MS, SSR_Pin, Tc);
Display Disp(INTERVAL_DISP, &Tc, &Hotp);

// Internal vars
volatile byte rotary_aValPrev = 0;             // Rotary A, last level, see ISR(PCINT1_vect)
volatile byte rotary_sValPrev = 1;             // Rotary S, last level, see ISR(PCINT1_vect)
volatile unsigned long rotary_sPressed_ms = 0; // volatile, see ISR(PCINT1_vect)

void setup()
{
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  Serial.println("Initializing...");
#endif
#ifdef DEBUG_AVRSTUB
  debug_init();
#endif
  Config::load();

  Runnable::setupAll();

  // FIXME JE: Check/Test if the internal pull up would save the external soldered ones
  pinMode(ROTARY_A_PIN, INPUT_PULLUP); // Arduino Analog input 0 (PCINT8), input and set pull up resistor:
  pinMode(ROTARY_B_PIN, INPUT_PULLUP); // Arduino Analog input 1 (PCINT9) an input and set pull up resistor:
  pinMode(ROTARY_S_PIN, INPUT_PULLUP); // Arduino Analog input 2 (PCINT10) an input and set pull up resistor:
  // This is ATMEGA368 specific, see page 75 of long datasheet
  // PCICR: Pin Change Interrupt Control Register - enables interrupt vectors
  // Bit 2 = enable PC vector 2 (PCINT23..16)
  // Bit 1 = enable PC vector 1 (PCINT14..8)
  // Bit 0 = enable PC vector 0 (PCINT7..0)
  PCICR |= (1 << PCIE1); // Set port bit in CICR for vector 1 (PCINT14..8)
  // Pin change mask registers decide which pins are enabled as triggers:
  PCMSK1 |= (1 << ROTARY_A_INT);
  PCMSK1 |= (1 << ROTARY_B_INT);
  PCMSK1 |= (1 << ROTARY_S_INT);

  interrupts(); // Enable interrupts

#ifdef DEBUG_SERIAL
  Serial.println("loop()...");
#endif
}

void loop()
{
  Runnable::loopAll();
  HotLed.blinkByTemp(Tc.getTemperatureAverage());
}

void onPlusPressed()
{
  if (Hotp.getSetpoint() < Config::active.pid_max_temp_c)
  {
    Hotp.setSetpoint(Hotp.getSetpoint() + 1);
  }
}

void onMinusPressed()
{
  if (Hotp.getSetpoint())
  {
    Hotp.setSetpoint(Hotp.getSetpoint() - 1);
  }
}

void onPushPressed()
{
  if (Config::active.profile != Hotplate::Profile::Manual &&
      Hotp.getControllerState() == Hotplate::ControllerState::off)
  { // Start reflow profile
    Hotp.runProfile();
  }
  else
  {
    Hotp.setSetpoint(0);
  }
}

void onPushLongPressed()
{
  Disp.changeUiMode(Display::uiMode::Setup);
}

ISR(PCINT1_vect)
{
  byte pVal; // Port value (8 Bits)
  byte aValAct;
  byte bValAct;
  byte sValAct;

  pVal = ROTARY_PORT_CMD;               // Read relevenat port (8 bit)
  aValAct = pVal & (1 << ROTARY_A_INT); // Mask out all except ...
  aValAct = aValAct >> ROTARY_A_INT;    // shift to right for bit0 position
  bValAct = pVal & (1 << ROTARY_B_INT);
  bValAct = bValAct >> ROTARY_B_INT;
  sValAct = pVal & (1 << ROTARY_S_INT);
  sValAct = sValAct >> ROTARY_S_INT;

  // A/B state callbacks
  if (rotary_aValPrev != aValAct && aValAct)
  {
    if (aValAct != bValAct) // CW
    {
      onPlusPressed();
    }
    else // CCW
    {
      onMinusPressed();
    }
  }
  rotary_aValPrev = aValAct;

  // S state callbacks
  if (rotary_sValPrev == 1 && !sValAct) // button is pressed
  {
    rotary_sPressed_ms = millis();
    onPushPressed();
  }
  else if (!rotary_sValPrev && sValAct && rotary_sPressed_ms) // button is released && was pressed before (init fuse)
  {
    if ((millis() - rotary_sPressed_ms) > LONG_PRESS_TIME_MS)
    {
      onPushLongPressed();
    }
  }
  rotary_sValPrev = sValAct;
}