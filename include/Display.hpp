#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "main.hpp"
#include "Hotplate.hpp"

class Display
{
public:
    enum uiMode
    {
        Main,
        Setup
    };

    Display();
    void initialize();
    void update(Thermocouple *, Hotplate *);
    void changeUiMode(uiMode nextUi);

private:
    U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;
    uiMode uiM = uiMode::Main;

    uint16_t _lastTarget;
    bool _lastPower;
    double _lastTemp;
    short _lastProfileTimePosition = 0;

    void inputBangValues();
    void inputPidConstants();
    void inputReflowProfile(Hotplate *);
    void inputSsrType();
    void mainScreen(Thermocouple *, Hotplate *);
    void setStdFont();
    void setupScreen(Hotplate *);
    void userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);
    void userInterfaceInputDouble(const char *title, const char *pre, double *value, uint8_t numInt, uint8_t numDec, const char *post);
    u8g2_uint_t drawUTF8Lines(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s);
};

#endif