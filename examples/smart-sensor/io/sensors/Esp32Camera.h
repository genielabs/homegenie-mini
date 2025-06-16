/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

#define ESP_CAMERA_QUALITY "20"

#ifdef ESP32_S3_LCD_CAM
#define ESP_CAMERA_PWR "17"  // Power down is not used
#define ESP_CAMERA_XCL "8"
#define ESP_CAMERA_XFR "20000000"
#define ESP_CAMERA_VSN "6"
#define ESP_CAMERA_HRF "4"
#define ESP_CAMERA_PCL "9"
#define ESP_CAMERA_SDA "21"
#define ESP_CAMERA_SCL "16"
#define ESP_CAMERA_CD0 "12"
#define ESP_CAMERA_CD1 "13"
#define ESP_CAMERA_CD2 "15"
#define ESP_CAMERA_CD3 "11"
#define ESP_CAMERA_CD4 "14"
#define ESP_CAMERA_CD5 "10"
#define ESP_CAMERA_CD6 "7"
#define ESP_CAMERA_CD7 "2"
#define ESP_CAMERA_RST "-1" // Software reset will be performed
#elif ESP32_S3
#define ESP_CAMERA_PWR "-1"
#define ESP_CAMERA_XCL "15"
#define ESP_CAMERA_XFR "24000000"
#define ESP_CAMERA_VSN "6"
#define ESP_CAMERA_HRF "7"
#define ESP_CAMERA_PCL "13"
#define ESP_CAMERA_SDA "4"
#define ESP_CAMERA_SCL "5"
#define ESP_CAMERA_CD0 "11"
#define ESP_CAMERA_CD1 "9"
#define ESP_CAMERA_CD2 "8"
#define ESP_CAMERA_CD3 "10"
#define ESP_CAMERA_CD4 "12"
#define ESP_CAMERA_CD5 "18"
#define ESP_CAMERA_CD6 "17"
#define ESP_CAMERA_CD7 "16"
#define ESP_CAMERA_RST "-1"
#else
#define ESP_CAMERA_PWR "32"
#define ESP_CAMERA_XCL "0"
#define ESP_CAMERA_XFR "20000000"
#define ESP_CAMERA_VSN "25"
#define ESP_CAMERA_HRF "23"
#define ESP_CAMERA_PCL "22"
#define ESP_CAMERA_SDA "26"
#define ESP_CAMERA_SCL "27"
#define ESP_CAMERA_CD0 "5"
#define ESP_CAMERA_CD1 "18"
#define ESP_CAMERA_CD2 "19"
#define ESP_CAMERA_CD3 "21"
#define ESP_CAMERA_CD4 "36"
#define ESP_CAMERA_CD5 "39"
#define ESP_CAMERA_CD6 "34"
#define ESP_CAMERA_CD7 "35"
#define ESP_CAMERA_RST "-1"
#endif

#include "FS.h"
#include "SD_MMC.h"


namespace IO { namespace Sensors {

    using namespace Service;
    using namespace Service::devices;

    using namespace SensorApi::Configuration;

    class Esp32Camera {
    public:

        explicit Esp32Camera(devices::Camera* cameraHandler);

        static void cameraStart(camera_grab_mode_t grabMode = CAMERA_GRAB_WHEN_EMPTY);
        static void cameraStop();

        static void init(Camera *pCamera);

        static FrameBuffer* cameraGetFrame(uint8_t frameSize = 0, uint8_t quality = 0) {
            unsigned long ts = millis();
            while (isBusy) { // perhaps it would be best to use a semaphore
                // request timeout
                if (millis() - ts > 50) {
                    return currentFrame;
                }
                delay(1);
            }
            isBusy = true;

            if (frameSize > 0 && (framesize_t)frameSize != config.frame_size) {
                config.frame_size = (framesize_t)frameSize;
                auto v = String(frameSize);
                apiHandler->module->setProperty(Options::Image_Resolution, v);
                auto m = std::make_shared<QueuedMessage>(apiHandler->module, Options::Image_Resolution, v, &frameSize, Number);
                HomeGenie::getInstance()->getEventRouter().signalEvent(m);
            }

            if (!initialized) {
                cameraStart();
            }

            if (quality > 0 && quality != config.jpeg_quality) {
                config.jpeg_quality = quality;
                auto v = String(quality);
                apiHandler->module->setProperty(Options::Image_Quality, v);
                auto m = std::make_shared<QueuedMessage>(apiHandler->module, Options::Image_Quality, v, &quality, Number);
                HomeGenie::getInstance()->getEventRouter().signalEvent(m);
            }

            cameraFrame = esp_camera_fb_get();
            currentFrame->buffer = cameraFrame->buf;
            currentFrame->length = cameraFrame->len;

            return currentFrame;
        }
        static void cameraReleaseFrame() {
            esp_camera_fb_return(cameraFrame);
            isBusy = false;
        }

    private:
        static camera_fb_t* cameraFrame;
        static bool isBusy;
        static bool initialized;
        static bool sdcardInitialized;
        static FrameBuffer* currentFrame;
        static Esp32Camera* instance;
        static camera_config_t config;
        static devices::Camera* apiHandler;

        static void applySettings();

        static bool initMicroSDCard() {
            if (sdcardInitialized) return true;
            auto sdmc_clk = Config::getSetting(Camera_Sensor::SdCard_clk, "-1").toInt();
            auto sdmc_cmd = Config::getSetting(Camera_Sensor::SdCard_cmd, "-1").toInt();
            auto sdmc_pd0 = Config::getSetting(Camera_Sensor::SdCard_pd0, "-1").toInt();
            auto sdmc_pd1 = Config::getSetting(Camera_Sensor::SdCard_pd1, "-1").toInt();
            auto sdmc_pd2 = Config::getSetting(Camera_Sensor::SdCard_pd2, "-1").toInt();
            auto sdmc_pd3 = Config::getSetting(Camera_Sensor::SdCard_pd3, "-1").toInt();
            if (sdmc_clk != -1 && sdmc_cmd != -1 && sdmc_pd0 != -1) {
                // Start the MicroSD card
                SD_MMC.setPins(sdmc_clk, sdmc_cmd, sdmc_pd0, sdmc_pd1, sdmc_pd2, sdmc_pd3);
                if (!SD_MMC.begin("/sdcard", true)) {
                    Serial.println("MicroSD Card Mount Failed");
                    return false;
                }
                uint8_t cardType = SD_MMC.cardType();
                if (cardType == CARD_NONE) {
                    Serial.println("No MicroSD Card found");
                    return false;
                }
                sdcardInitialized = true;
            }
            return sdcardInitialized;
        }

        static bool takeNewPhoto(const char* folder, const char* fullPath) {
            bool success = false;
            if (!initialized) {
                cameraStart();
            }
            // Camera sensor successfully initialized
            if (initialized)  {
                // empty cache
                for (int i = 0; i < config.fb_count; i++) {
                    camera_fb_t* fb = esp_camera_fb_get();
                    esp_camera_fb_return(fb);
                }
                camera_fb_t* fb = esp_camera_fb_get();
                if (!fb) {
                    // Capture failed
                    return false;
                }
                // Save to file
                fs::FS &fs = SD_MMC;
                if (!fs.exists(folder)) {
                    fs.mkdir(folder);
                }
                File file = fs.open(fullPath, FILE_WRITE);
                if (file) {
                    success = file.write(fb->buf, fb->len) == fb->len;
                }
                file.close();
                esp_camera_fb_return(fb);
            }
            return success;
        }

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
