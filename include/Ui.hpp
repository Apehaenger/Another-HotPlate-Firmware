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
    //    void setMainScreenTitle(const char*);

private:
    typedef struct MainScreenCrcData
    {
        Hotplate::Mode hpMode;
        bool hpPower;
        uint16_t hpSetpoint;
        Hotplate::State hpState;
        short profileSecLeft;
        float tcTempAvg;
    } MainScreenCrcData;

    U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;
    Mode _mode = Mode::Main;

    uint32_t _nextInterval_ms = 0;
    uint32_t _lastMainScreenCrc;

    void displayMainScreen();
    void displaySetupScreen();

    void inputBangValues();
    void inputMaxTemp();
    void inputPidConstants();
    void inputReflowProfile();
    void inputSsrType();

    void setStdFont();

    void userInterfaceInputDouble(const char *title, const char *pre, double *value, uint8_t numInt, uint8_t numDec, const char *post);

    u8g2_uint_t drawUTF8Lines(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s);
};

#endif