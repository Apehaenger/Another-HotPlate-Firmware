/*
 * This file is part of the Another-Reflow-HotPlate-Firmware project (https://github.com/Apehaenger/Another-Reflow-HotPlate-Firmware).
 * Copyright (c) 2022 Jörg Ebeling
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <Arduino.h>
#include <EEPROM.h>
#include "main.hpp"
#include "config.hpp"
#include "CRC32.h"

namespace Config
{
        Conf active; // Default config

        void load()
        {
                EEPConfig eConf;
                EEPROM.get(0, eConf);

                uint32_t configChecksum = CRC32::calculate(&eConf.conf, 1);

#ifdef DEBUG_SERIAL
                Serial.print("EEPROM CRC: ");
                Serial.print(eConf.crc, HEX);
                Serial.print(", Conf CRC: ");
                Serial.print(configChecksum, HEX);
                uint32_t defaultChecksum = CRC32::calculate(&active, 1);
                Serial.print(", default Conf CRC: ");
                Serial.println(defaultChecksum, HEX);
#endif

                if (eConf.crc == configChecksum && eConf.conf.version == active.version)
                {
                        active = eConf.conf;
                }
        }

        void save()
        {
                EEPConfig eConf;

                eConf.crc = CRC32::calculate(&active, 1);
                eConf.conf = active;

#ifdef DEBUG_SERIAL
                Serial.print("New Conf CRC: ");
                Serial.println(eConf.crc, HEX);
#endif
                // No need to compare the config checksum if it changed, as we use put() which in turn use EEPROM.update() and write only on change
                EEPROM.put(0, eConf);
        }
}
