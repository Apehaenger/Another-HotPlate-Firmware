#ifndef config_h
#define config_h

#include "Profile.hpp"

#define CONFIG_VERSION 5 // Change to force reload of default config even if config structure hasn't changed

namespace Config
{
    typedef struct Conf // Default config if read of EEPROM fails, or not set
    {
        bool disp_unit_c = true; // °C/F

        uint16_t pid_pwm_window_ms = 5000; // For best autotune results, this should be at least system-temp-delay time long
        uint16_t pid_max_temp_c = 300;     // Max. PTC/PID temperature (*C)

        double pid_Kp = 50;
        double pid_Ki = 0;
        double pid_Kd = 0;
        uint8_t pid_bangOn_temp_c = 0;
        uint8_t pid_bangOff_temp_c = 0;

        Profile::Profiles profile = Profile::Profiles::Manual;

        bool ssr_active_low = true; // SSR = on @ low level = true, or on high level

        uint8_t version;
    } Conf;

    typedef struct EEPConfig // EEPROM Config struct
    {
        uint32_t crc;
        Conf conf;
    } EEPConfig;

    extern Conf active;

    void load();
    void save();
}

#endif