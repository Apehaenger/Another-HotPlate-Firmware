#ifndef Hotplate_h
#define Hotplate_h

#define PROFILE_TIME_INTERVAL_MS 10000
#define PID_TUNER_INTERVAL_MS 500 // How often Serial.print values for PID Tuner

#include <AutoPID.h>
#include "Runnable.hpp"
#include "Thermocouple.hpp"

#define PID_SAMPLE_MS 200 // Should be the shortest PTC-on time, but not shorter than a typical inrush-current period of a PTC (approx. 0.1s)

class Hotplate : public Runnable
{
    const uint8_t _ssrPin;
    Thermocouple &_TcRef;

public:
    enum State
    {
        Off,
        On,
        // Autotune,
    };

    enum Profile : uint8_t // User selectable reflow profile/mode
    {
        Manual,
        Sn42Bi576Ag04,
        Sn965Ag30Cu05,
    };

    const char *profile2str[3] = {
        [Manual] = "Manual",
        [Sn42Bi576Ag04] = "Sn42/Bi57.6/Ag0.4",
        [Sn965Ag30Cu05] = "Sn96.5/Ag3.0/Cu0.5",
    };

    enum ControllerState
    {
        off,
        pid,
        bangOn,
        bangOff,
    };

    Hotplate(uint16_t interval_ms, uint8_t ssr_pin, Thermocouple &TcRef);
    void setup() override;
    void loop() override;

    ControllerState getControllerState();
    uint16_t getOutput();
    short getProfileTimePosition();
    bool getPower();
    uint16_t getSetpoint();
    void runProfile();
    void setSetpoint(uint16_t setpoint);
    void updatePidGains();

private:
    typedef struct
    {
        uint16_t time_s; // After n seconds...
        uint16_t temp_c; // reach this target temp
    } ProfileTimeTarget;

    typedef struct
    {
        ProfileTimeTarget *timeTargets;
        uint8_t size;        // size of timeTargets[]
        uint16_t duration_s; // Duration (s) of the complete profile (last entry)
    } ProfileTimeTargets;

    // FIXME: Should be possible directly within mode2timeTargets
    ProfileTimeTarget _profileTimeTargets_Sn42Bi576Ag04[4] = {{90, 90}, {180, 130}, {210, 138}, {240, 165}};
    ProfileTimeTargets _profileTimeTarget_Sn42Bi576Ag04 = {_profileTimeTargets_Sn42Bi576Ag04, sizeof(_profileTimeTargets_Sn42Bi576Ag04) / sizeof(ProfileTimeTarget), 240};
    //
    ProfileTimeTarget _profileTimeTargets_tSn965Ag30Cu05[4] = {{90, 150}, {180, 175}, {210, 217}, {240, 249}};
    ProfileTimeTargets _profileTimeTarget_tSn965Ag30Cu05 = {_profileTimeTargets_tSn965Ag30Cu05, sizeof(_profileTimeTargets_tSn965Ag30Cu05) / sizeof(ProfileTimeTarget), 240};

    ProfileTimeTargets *_profile2timeTargets[3] = {
        [Manual] = nullptr,
        [Sn42Bi576Ag04] = &_profileTimeTarget_Sn42Bi576Ag04,
        [Sn965Ag30Cu05] = &_profileTimeTarget_tSn965Ag30Cu05,
    };

    AutoPID _myPID;
    State _state = State::Off;
    double _input = 0, _setpoint = 0, _output = 0;
    uint32_t _pwmWindowStart_ms, _profileStart_ms, _profileNext_ms = 0, _pidTunerNext_ms = 0;
    ControllerState _controllerState = ControllerState::off;
    bool _power = false;

    short getProfileTemp();
    void setPower(bool);
};

#endif