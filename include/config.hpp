#ifndef config_h
#define config_h

#include "Hotplate.hpp"

namespace Config
{
    typedef struct Conf // Default config if read of EEPROM fails, or not set
    {
        bool disp_unit_c = true; // °C/F

        uint16_t pid_pwm_window_ms = 5000; // For best autotune results, this should be at least system-temp-delay time long
        uint16_t pid_max_temp_c = 300;     // Max. PTC/PID temperature (*C)

        /* Kp tuning (with TC cables around plate), BangOn & BangOff = 0:
         * Kp   Ki   Kd  Setpoint °C    Result
         * -------------------------------------------------------------------
         * 10   0   0       165         Max. 159, then approx. 152 +/- 1
         * 40   0   0       165         Max. 169, then 158-162
         * 30   0   0       165         Max. 170, then 159-166
         * 20   0   0       165         Max. 168, then 155-156 Quite good
         * 15   0   0       165         Max. 164, then 153-162
         * 10   0   0       165         Max. 160, then 140-143. Repeated test, quite slow!
         * 20   0   0       165         Max. 164, then 150-162
         * 50   0   0       165         Max. 170, then 156-165
         */
        double pid_Kp = 50;

        /* Ki tuning (with TC cables around plate), BangOn & BangOff = 0:
         * Kp   Ki   Kd  Setpoint °C    Result
         * -------------------------------------------------------------------
         * 20   100  0      165         Max. 180, aborted
         * 20   300  0      165         Max. 181, aborted
         * 20    10  0      165         Max. 181, then 168-177
         * 20     5  0      165         Max. 178, then 167-175
         */
        double pid_Ki = 0;
        double pid_Kd = 0;
        uint8_t pid_bangOn_temp_c = 40;
        uint8_t pid_bangOff_temp_c = 5;

        Hotplate::Profile profile = Hotplate::Profile::Manual;

        bool ssr_active_low = true; // SSR = on @ low level = true, or on high level
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