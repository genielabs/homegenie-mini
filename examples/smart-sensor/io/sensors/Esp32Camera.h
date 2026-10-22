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
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <vector>
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

        // Each task may hold one shared frame lease, released by that same task.
        static FrameBuffer* cameraGetFrame(uint8_t frameSize = 0, uint8_t quality = 0);
        static void cameraReleaseFrame();
        static void cameraUpdateOption(ModuleParameter* option);

    private:
        static camera_fb_t* cameraFrame;
        static SemaphoreHandle_t cameraMutex;
        static std::vector<TaskHandle_t> frameReaders;
        static unsigned int waitingWriters;
        class CameraLock {
            bool acquired;
        public:
            explicit CameraLock(TickType_t wait = pdMS_TO_TICKS(250))
                : acquired(cameraMutex != nullptr && xSemaphoreTakeRecursive(cameraMutex, wait) == pdTRUE) {}
            ~CameraLock() { if (acquired) xSemaphoreGiveRecursive(cameraMutex); }
            explicit operator bool() const { return acquired; }
            // Wait without holding the mutex so existing readers can release their leases.
            bool waitForIdle(TickType_t wait = portMAX_DELAY) {
                for (auto reader : frameReaders) {
                    if (reader == xTaskGetCurrentTaskHandle()) return false;
                }
                const TickType_t started = xTaskGetTickCount();
                ++waitingWriters;
                while (cameraFrame != nullptr) {
                    if (wait != portMAX_DELAY && xTaskGetTickCount() - started >= wait) {
                        --waitingWriters;
                        return false;
                    }
                    xSemaphoreGiveRecursive(cameraMutex);
                    vTaskDelay(1);
                    xSemaphoreTakeRecursive(cameraMutex, portMAX_DELAY);
                }
                --waitingWriters;
                return true;
            }
            CameraLock(const CameraLock&) = delete;
            CameraLock& operator=(const CameraLock&) = delete;
        };
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

        static bool takeNewPhoto(const char* folder, const char* fullPath);
        static int configValueNormalize(const char* key, int min, int max, int v);

    };

    // UI options update listener
    static class : public ModuleParameter::UpdateListener {
    public:
        void onUpdate(ModuleParameter* option) override {
            Esp32Camera::cameraUpdateOption(option);
        }
    } optionUpdateListener;

}}

#endif // ESP_CAMERA_SUPPORTED

#endif //HOMEGENIE_MINI_ESP32CAMERA_H
