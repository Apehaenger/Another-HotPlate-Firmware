#ifndef Ui_h
#define Ui_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Hotplate.hpp"

#define INTERVAL_DISP 100 // (max) Display refresh rate (if dirty)

class Ui
{
public:
    enum Mode
    {
        Main,
        Setup
    };

    Ui();
    void setup();
    void loop();
    void changeMode(Mode nextMode) { _mode = nextMode; };

private:
    U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;
    Mode _mode = Mode::Main;

    uint32_t _nextInterval_ms = 0;
    uint16_t _lastTarget;
    bool _lastPower;
    float _lastTemp;
    short _lastProfileSecondLeft;
    Hotplate::State _lastState;
    Hotplate::Mode _lastMode;

    void inputBangValues();
    void inputPidConstants();
    void inputReflowProfile();
    void inputSsrType();

    void mainScreen();

    void setStdFont();
    void setupScreen();

    void userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);
    void userInterfaceInputDouble(const char *title, const char *pre, double *value, uint8_t numInt, uint8_t numDec, const char *post);

    u8g2_uint_t drawUTF8Lines(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s);
};

#endif