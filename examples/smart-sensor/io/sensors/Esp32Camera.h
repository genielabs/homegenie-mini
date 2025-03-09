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
 * - 2025-02-25 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_ESP32CAMERA_H
#define HOMEGENIE_MINI_ESP32CAMERA_H

#ifdef ESP_CAMERA_SUPPORTED

#include <HomeGenie.h>
#include <src/service/api/devices/Dimmer.h>
#include <src/service/api/devices/Camera.h>

#include "esp_camera.h"
#include "soc/rtc_cntl_reg.h"

#include "../../api/SensorApi.h"

namespace IO { namespace Sensors {

    using namespace Service;
    using namespace Service::devices;

    using namespace SensorApi::Configuration;

    class Esp32Camera {
    public:

        explicit Esp32Camera(devices::Camera* cameraHandler);

        static void cameraStart();
        static void cameraStop();

        static void init(Camera *pCamera);

    private:
        static camera_fb_t* cameraFrame;
        static bool initialized;
        static FrameBuffer* currentFrame;
        static Esp32Camera* instance;

        static void applySettings();
    };

    // UI options update listener
    static class : public ModuleParameter::UpdateListener {
    public:
        void onUpdate(ModuleParameter* option) override {
            int v = option->value.toInt();
            sensor_t* s = esp_camera_sensor_get();
            if (s != nullptr) {
                if (option->is(Options::Controls_Brightness)) {
                    v = configValueNormalize(Camera_Sensor::BrightnessControl, -3, 3, v);
                    s->set_brightness(s, v);
                } else if (option->is(Options::Controls_Saturation)) {
                    v = configValueNormalize(Camera_Sensor::SaturationControl, -3, 3, v);
                    s->set_saturation(s, v);
                } else if (option->is(Options::Controls_Contrast)) {
                    v = configValueNormalize(Camera_Sensor::ContrastControl, -3, 3, v);
                    s->set_contrast(s, v);
                } else if (option->is(Options::Controls_Effect)) {
                    v = configValueNormalize(Camera_Sensor::EffectControl, 0, 6, v);
                    s->set_special_effect(s, v);
                } else if (option->is(Options::Controls_HMirror)) {
                    s->set_hmirror(s, option->value.equals("true") ? 1 : 0);
                } else if (option->is(Options::Controls_VFlip)) {
                    s->set_vflip(s, option->value.equals("true") ? 1 : 0);
                } else if (option->is(Options::Image_Quality)) {
                    v = configValueNormalize(Options::Image_Quality, 10, 63, v);
                    s->set_quality(s, v);
                } else if (option->is(Options::Image_Resolution)) {
                    Esp32Camera::cameraStop();
                }
            }
        }
    private:
        static int configValueNormalize(const char* key, int min, int max, int v) {
            if (v < min) {
                v = min;
                Config::saveSetting(key, String(v));

            } else if (v > max) {
                v = max;
                auto val = String(v);
                Config::saveSetting(key, String(v));
            }
            return v;
        }
    } optionUpdateListener;

}}

#endif // ESP_CAMERA_SUPPORTED

#endif //HOMEGENIE_MINI_ESP32CAMERA_H
