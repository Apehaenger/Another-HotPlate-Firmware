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
    enum class Mode : uint8_t
    {
        Off = 0x0,
        Manual = 0x1,
        Profile = 0x2,
        PIDTuner = 0x4,
    };

    enum class State : uint8_t
    {
        Idle = 0x0,    // Waiting for "Press to start" (a flow)
        Start = 0x1,   // Start flow
        Heating = 0x2, // Heat up to to target temp
        Settle = 0x4,  // Wait temp get settled
        BangOn = 0x8,
        BangOff = 0x10,
        PID = 0x12,
    };

    Hotplate(uint16_t interval_ms, uint8_t ssr_pin);

    void setup() override;
    void loop() override;

    Mode getMode();
    uint16_t getOutput();
    bool getPower();
    uint16_t getSetpoint();
    State getState();

    bool isMode(Mode);
    bool isState(State);

    void setMode(Mode);
    void setState(State);
    void setSetpoint(uint16_t setpoint);

    void updatePidGains();

private:
    AutoPID _myPID;
    double _input, _setpoint = 0, _output = 0;
    uint32_t _pwmWindowStart_ms, _pidTunerOutputNext_ms = 0, _pidTunerStart_ms = 0;
    Mode _mode = Mode::Off;
    State _state = State::Idle;
    bool _power = false;

    // FIXME: Should go into setuo?!
    const uint16_t _pidTunerTargetTemp = 100;
    const uint8_t _pidTunerTempNoise_c = 2;
    const uint8_t _pidTunerTempSettled_c = 5;

    void setPower(bool);
};

#endif