#ifndef Profile_h
#define Profile_h

#define PROFILE_TIME_INTERVAL_MS 10000 // Allows a user to adjust the target for the amount of interval

class Profile
{
public:
    enum Profiles : uint8_t // User selectable reflow profile/mode
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

    Profile();
    void loop();

    short getSecondsLeft();

    bool isStandBy();

    bool startProfile();
    void stopProfile();

private:
    typedef struct
    {
        uint16_t time_s; // After n seconds...
        uint16_t temp_c; // reach this target temp
    } ProfileTimeTarget;

    typedef struct
    {
        uint8_t length; // length of (the next) timeTargets[]
        ProfileTimeTarget *timeTargets;
        uint16_t duration_s; // Duration (s) of the complete profile (last entry)
    } ProfileTimeTargets;

    // FIXME: Should be possible directly within _profile2timeTargets
    const ProfileTimeTargets _profileTimeTarget_Sn42Bi576Ag04 = {4, (ProfileTimeTarget[]){{90, 90}, {180, 130}, {210, 138}, {240, 165}}, 240};
    const ProfileTimeTargets _profileTimeTarget_tSn965Ag30Cu05 = {4, (ProfileTimeTarget[]){{90, 150}, {180, 175}, {210, 217}, {240, 249}}, 240};
    ProfileTimeTargets const *_profile2timeTargets[3] = {
        [Manual] = nullptr,
        [Sn42Bi576Ag04] = &_profileTimeTarget_Sn42Bi576Ag04,
        [Sn965Ag30Cu05] = &_profileTimeTarget_tSn965Ag30Cu05,
    };

    uint32_t _nextInterval_ms = 0, _profileStart_ms = 0;

    uint16_t getTempTarget();
};

#endif