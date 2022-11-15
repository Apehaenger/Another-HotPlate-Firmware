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
 * TODO:
 * - Ui should be split up into Ui and Display. The button logic from main should go then into Ui
 * - Resturcture Ui class with more cleare methods like pushToStart(), setTitle(),
 *   so that the display get controlled by the related flow and not visa versa lije now.
 * - Redraw only the relevant stuff
 *
 * Usage:
 *     RAM      |    Flash    | Comment
 * ----------------------------------------------------------------------
 * 71.9%  1473    91.8% 28186   0.3.0 new Bootloader
 * 62.9%  1289    82.8% 25432   0.4 without DEBUG_SERIAL
 * 62.7%  1285    82.8% 25428   0.4.0 without DEBUG_SERIAL
 * 64.2%  1315    86.2% 26470   Wasted 1042!! Bytes for more clear Runnable implementation
 * 63.9%  1309    86.2% 26482   Separated profile handling into separate class and switched to global class pointer
 * 72.8%  1490    89.5% 27492   Added/activated Serial for PID Tuner. It's going to become narrow...
 * 81.1%  1660    95.8% 29416   PID Tuner and final corrections. Everything looks fine
 * 79.9%  1636    95.4% 29316   Simplifid initial PID-Tuner wait time
 * 79.9%  1636    95.3% 29282   Optimized profile time calculations
 * 79.9%  1636    95.3% 29282   Patched AutoPID derivate calculation
 * 79.2%  1622    94.9% 29144   Hotplate not an sub-class of Runnable anymore = - 14 byte RAM, -138 byte flash
 * 77.7%  1592    91.4% 28090   Removed Runnable super-class and saved 44 byte RAM, 1192 byte Flash
 */
#include <Arduino.h>
#include "main.hpp"
#include "config.hpp"
#include "Led.hpp"

#if defined ATMEGA328_NEW_CH340_DBG || defined ATMEGA328_NEW_FTDI_DBG
#undef DEBUG_SERIAL
#define DEBUG_AVRSTUB
#endif

#ifdef DEBUG_AVRSTUB
#include "avr8-stub.h"
#endif

// Init classes
Led hotLed(LED_PIN);
Thermocouple thermocouple(TC_CLK_PIN, TC_CS_PIN, TC_DO_PIN);
Hotplate hotplate(SSR_Pin);
Profile profile;
Ui ui;

// Internal vars
volatile byte rotary_aValPrev = 0;             // Rotary A, last level, see ISR(PCINT1_vect)
volatile byte rotary_sValPrev = 1;             // Rotary S, last level, see ISR(PCINT1_vect)
volatile unsigned long rotary_sPressed_ms = 0; // volatile, see ISR(PCINT1_vect)

void setup()
{
#ifndef DEBUG_AVRSTUB
  Serial.begin(115200); // TODO: -> Setup?
  Serial.println("Init...");
#endif
#ifdef DEBUG_AVRSTUB
  debug_init();
#endif

  Config::load();
  ui.setup();
  hotplate.setup();

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
  profile.loop();
  hotplate.loop();
  ui.loop();
  hotLed.blinkByTemp(thermocouple.getTemperatureAverage());
}

/**
 * @brief Start if a process like PIDTuner or ReflowProfile is idle (waiting to get started)
 *
 * @return true if there was an idle process
 * @return false if there isn't an idle process
 */
bool startIfStandByProcess()
{
  if (hotplate.isStandBy())
  {
    hotplate.setState(Hotplate::State::Start);
    return true;
  }
  if (profile.isStandBy())
  {
    return profile.startProfile();
  }
  return false;
}

void onPlusPressed()
{
  if (hotplate.getSetpoint() < Config::active.pid_max_temp_c)
  {
    startIfStandByProcess();
    hotplate.setSetpoint(hotplate.getSetpoint() + 1);
  }
}

void onMinusPressed()
{
  if (hotplate.getSetpoint())
  {
    startIfStandByProcess();
    hotplate.setSetpoint(hotplate.getSetpoint() - 1);
  }
}

void onPushPressed()
{
  if (!startIfStandByProcess())
  {
    hotplate.setMode(Hotplate::Mode::Manual);
    hotplate.setState(Hotplate::State::StandBy);
    profile.stopProfile();
    hotplate.setSetpoint(0);
  }
}

void onPushLongPressed()
{
  ui.changeMode(Ui::Mode::Setup);
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