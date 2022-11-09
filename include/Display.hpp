#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Hotplate.hpp"

class Display : public Runnable
{
public:
    enum uiMode
    {
        Main,
        Setup
    };

    Display(uint16_t interval_ms, Thermocouple *TcPtr, Hotplate *HotPtr);
    void setup();
    void loop();
    void changeUiMode(uiMode nextUi);

private:
    U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;
    Thermocouple *_TcPtr;
    Hotplate *_HotPtr;
    uiMode uiM = uiMode::Main;

    uint16_t _lastTarget;
    bool _lastPower;
    double _lastTemp;
    short _lastProfileTimePosition = 0;

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