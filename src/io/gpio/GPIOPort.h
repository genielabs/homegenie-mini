/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
 *
 *
 * This file is part of HomeGenie-Mini (HGM).
 *
 *  HomeGenie-Mini is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HomeGenie-Mini is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HomeGenie-Mini.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Authors:
 * - Generoso Martello <gene@homegenie.it>
 *
 *
 * Releases:
 * - 2019-02-03 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_GPIOPORT_H
#define HOMEGENIE_MINI_GPIOPORT_H

#ifndef ESP8266
#include <Preferences.h>
#endif

#include <io/Logger.h>

#include "Config.h"

#include "io/IOEvent.h"
#include "io/IOEventDomains.h"
#include "io/IOEventPaths.h"

#define GPIO_PORT_NS_PREFIX           "IO::GPIO::GPIOPort"

#define GPIO_PORT_LEVEL_MIN       0.0F
#define GPIO_PORT_LEVEL_MAX       254.0F

namespace IO { namespace GPIO {

    class GPIOPort: public IIOEventSender  {
    public:
        void begin() override;
        void setInput(uint8_t pinNumber, void(*callback)());
        void setOutput(uint8_t pinNumber, uint8_t value);
        float on(uint8_t pinNumber);
        float off(uint8_t pinNumber);
        float toggle(uint8_t pinNumber);
        float level(uint8_t pinNumber, float level);
        static float loadLevel(int pinNumber) {
#ifndef ESP8266
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, true);
            float buttonState = preferences.getFloat((String("gpio:") + pinNumber + ":level").c_str(), 0);
            preferences.end();
            return buttonState;
#else
            return 0;
#endif
        }
        static float loadLastOnLevel(int pinNumber) {
#ifndef ESP8266
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, true);
            float buttonState = preferences.getFloat((String("gpio:") + pinNumber + ":last").c_str(), 0);
            preferences.end();
            return buttonState;
#else
            return 0;
#endif
        }
        static void saveLevel(int pinNumber, float level) {
#ifndef ESP8266
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putFloat((String("gpio:") + pinNumber + ":level").c_str(), level);
            preferences.end();
#endif
        }
        static void saveLastOnLevel(int pinNumber, float level) {
#ifndef ESP8266
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putFloat((String("gpio:") + pinNumber + ":last").c_str(), level);
            preferences.end();
#endif
        }
    };

}}


#endif //HOMEGENIE_MINI_GPIOPORT_H
