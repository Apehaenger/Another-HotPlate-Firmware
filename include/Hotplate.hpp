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
    /*
     * FIXME: Should switch to "flagged enums" and merge them to State,
     * but they're tricky/ugly with <= C 11.x
     */
    enum class Mode : uint8_t
    {
        Manual = 0x0,
        PIDTuner = 0x1,
    };

    /*
     * This State enum is for informational (display) AS WELL AS control flow purposes.
     * It has mixed usage mainly to save some byte.
     * At the moment there's only the PIDTuner who's using the control flow related items
     * like (StandBy), Start, Heating and Settle and the used names are held to be generic,
     * so that they can be used also for other (future) control flows.
     * 
     * FIXME: Should switch to "flagged enums" and merge also Mode here,
     * but they're tricky/ugly with <= C 11.x
     */
    enum class State : uint8_t
    {
        // Control flow purposes
        StandBy = 0x0, // Waiting for "Press to start" (a process flow)
        Start = 0x1,   // Start flow
        Heating = 0x2, // Heat up to to target temp
        Settle = 0x4,  // Wait temp get settled
        // Informational (display) purposes
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

    bool isStandBy();
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
    Mode _mode = Mode::Manual;
    State _state = State::StandBy;
    bool _power = false;

    // FIXME: Should go into setuo?!
    uint16_t _pidTunerTempTarget = 100;
    const uint8_t _pidTunerTempNoise = 2;
    const uint8_t _pidTunerTempSettled = 10;
    uint16_t _pidTunerTempMax;

    void setPower(bool);
};

#endif