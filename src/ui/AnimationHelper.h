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

#ifndef HOMEGENIE_MINI_ANIMATIONHELPER_H
#define HOMEGENIE_MINI_ANIMATIONHELPER_H

namespace UI {

    enum TimeEasing {
        ANIMATION_EASING_LINEAR,
        ANIMATION_EASING_EASE_IN,
        ANIMATION_EASING_EASE_OUT,
        ANIMATION_EASING_EASE_IN_OUT,
        ANIMATION_EASING_SPIKE,
    };

    class AnimationHelper {
    public:

        static float lerp(float from, float to, float progress) {
            return from * (1.0 - progress) + (to * progress);
        }

        static float flip(float progress) {
            return 1 - progress;
        }

        static float easeIn(float progress) {
            return progress * progress;
        }

        static float easeOut(float progress) {
            return flip(sqrt(flip(progress)));
        }

        static float easeInOut(float progress)
        {
            return lerp(easeIn(progress), easeOut(progress), progress);
        }

        static float spike(float progress)
        {
            if (progress <= .5f) {
                return easeIn(progress / .5f);
            }
            return easeIn(flip(progress)/.5f);
        }

        static float animateFloat(float fromValue, float toValue, float progress, TimeEasing easing = ANIMATION_EASING_LINEAR) {
            switch (easing) {
                case ANIMATION_EASING_EASE_IN:
                    progress = easeIn(progress);
                    break;
                case ANIMATION_EASING_EASE_OUT:
                    progress = easeOut(progress);
                    break;
                case ANIMATION_EASING_EASE_IN_OUT:
                    progress = easeInOut(progress);
                    break;
                case ANIMATION_EASING_SPIKE:
                    progress = spike(progress);
                    break;
            }
            return lerp(fromValue, toValue, progress);
        }
    };

}

#endif //HOMEGENIE_MINI_ANIMATIONHELPER_H
