#ifndef Hotplate_h
#define Hotplate_h

#define PID_TUNER_INTERVAL_MS 500 // How often Serial.print values for PID Tuner

#include <AutoPID.h>
#include "Runnable.hpp"

#define PID_SAMPLE_MS 200 // Should be the shortest PTC-on time, but not shorter than a typical inrush-current period of a PTC (approx. 0.1s)

class Hotplate : public Runnable
{
    const uint8_t _ssrPin;

public:
/*    enum Mode
    {
        Off,
        On,
        PIDTuner,
    };*/

    enum ControllerState
    {
        Off,
        PID,
        BangOn,
        BangOff,
        PIDTuner,
    };

    Hotplate(uint16_t interval_ms, uint8_t ssr_pin);
    void setup() override;
    void loop() override;

    ControllerState getControllerState();
    uint16_t getOutput();
    bool getPower();
    uint16_t getSetpoint();

    void setControllerState(ControllerState);
    void setSetpoint(uint16_t setpoint);
    
    void updatePidGains();

private:
    AutoPID _myPID;
    //Mode _mode = Mode::Off;
    double _input, _setpoint = 0, _output = 0;
    uint32_t _pwmWindowStart_ms, _pidTunerOutputNext_ms = 0;
    ControllerState _controllerState = ControllerState::Off;
    bool _power = false;
    const uint16_t _pidTunerTargetTemp = 100; // FIXME: Should go into setuo
    void setPower(bool);
};

#endif