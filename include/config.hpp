#ifndef config_h
#define config_h

#include "Profile.hpp"

#define CONFIG_VERSION 8 // Change to force reload of default config even if config structure hasn't changed

namespace Config
{
#pragma pack(push, 1)
    struct Conf // Default config if read of EEPROM fails, or not set
    {
        bool disp_unit_c = true; // °C/F

        uint8_t max_temp_c = 200; // Max. possible (or allowed) heater temperature (*C)
        
        uint16_t pid_pwm_window_ms = 5000; // For easiest handling, this might/should be approx. system-temp-delay time long

        double pid_Kp = 100.0;
        double pid_Ki = 2.0;
        double pid_Kd = 422.0;

        uint8_t pid_bangOn_temp_c = 50;
        uint8_t pid_bangOff_temp_c = 5;

        Profile::Profiles profile = Profile::Profiles::Manual;

        bool ssr_active_low = true; // SSR = on @ low level = true, or on high level

        uint8_t version = CONFIG_VERSION;
    };
#pragma pack(pop)

    struct EEPConfig // EEPROM Config struct
    {
        uint32_t crc;
        Conf conf;
    };

    extern Conf active;

    void load();
    void save();
}

#endif