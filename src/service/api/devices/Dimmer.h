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
 */

#ifndef HOMEGENIE_MINI_DIMMER_H
#define HOMEGENIE_MINI_DIMMER_H

#include <HomeGenie.h>

#include "Switch.h"

namespace Service { namespace API { namespace devices {

    class DimmerLevel {
    public:
        unsigned long duration = 0;
        bool isAnimating = false;
        void setLevel(float l, unsigned long transitionMs) {
            duration = transitionMs;
            if (l < 0) l = 0;
            else if (l > 1) l = 1;
            ol = level;
            level = l;
            startTime = millis();
            isAnimating = true;
        }
        float getProgress() {
            float p = (float)(millis() - startTime) / (float)duration;
            if (p >= 1) {
                isAnimating = false;
                p = 1;
            }
            return p;
        }
        float getLevel() {
            return ol + ((level - ol) * getProgress());
        }
    private:
        float level = 0;
        float ol = 0;
        unsigned long startTime = -1;

    };

    class Dimmer: public Switch {
    public:
        Dimmer(const char* domain, const char* address, const char* name);
        void loop() override;

        bool handleRequest(APIRequest*, ResponseCallback*) override;

        void dim(long transition = -1);
        void bright(long transition = -1);
        void setLevel(float l, long transition = -1);

        unsigned long getDefaultTransition() const {
            return defaultTransitionMs;
        }
        void setDefaultTransition(unsigned long transitionMs) {
            defaultTransitionMs = transitionMs;
        }

        void onSetLevel(std::function<void(float)> callback) {
            setLevelCallback = std::move(callback);
        }
    private:
        std::function<void(float)> setLevelCallback = nullptr;
        float lastCallbackLevel = 0;
        unsigned long defaultTransitionMs = 300;
    protected:
        DimmerLevel level;
    };

}}}

#endif //HOMEGENIE_MINI_DIMMER_H
