#ifndef main_h
#define main_h

#include "Ui.hpp"
#include "Thermocouple.hpp"
#include "Hotplate.hpp"
#include "Profile.hpp"

//#define DEBUG_SERIAL
//#define DEBUG_UI_SERIAL

// Thermocouple (MAX6675) pins
#define TC_DO_PIN 6
#define TC_CS_PIN 7
#define TC_CLK_PIN 8

// Hot-LED
#define LED_PIN 4

// Hotplate's SSR
#define SSR_Pin 5

// Rotary knob
#define ROTARY_PORT_CMD PINC // All rotary pins need to be connected to this Port

#define ROTARY_A_PIN A0
#define ROTARY_A_INT PCINT8 // FIXME JE: Isn't there a usable pin/pcint map?

#define ROTARY_B_PIN A1
#define ROTARY_B_INT PCINT9 // FIXME JE: Isn't there a usable pin/pcint map?

#define ROTARY_S_PIN A2
#define ROTARY_S_INT PCINT10 // FIXME JE: Isn't there a usable pin/pcint map?

#define LONG_PRESS_TIME_MS 500 // Long-Press if larger

// Internal
#define VERSION_TEXT "0.5.0b"

extern Ui ui;
extern Thermocouple thermocouple;
extern Hotplate hotplate;
extern Profile profile;

#endif