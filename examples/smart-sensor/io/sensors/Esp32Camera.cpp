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
#ifdef ESP_CAMERA_SUPPORTED

#include "Esp32Camera.h"

namespace IO { namespace Sensors {

    camera_fb_t* Esp32Camera::cameraFrame = nullptr;
    bool Esp32Camera::initialized = false;
    FrameBuffer* Esp32Camera::currentFrame = new FrameBuffer();
    Esp32Camera* Esp32Camera::instance = nullptr;
    bool Esp32Camera::sdcardInitialized = false;

    Esp32Camera::Esp32Camera(devices::Camera *cameraHandler) {
#ifndef ESP32_S3
        // On classic ESP32-CAM the internal LED has inverted on/off behaviour
        Config::StatusLedInvert = true;
#endif
        // Set the frame request handler
        cameraHandler->onFrameRequest([]{
            if (!initialized) {
                cameraStart();
            }
            cameraFrame = esp_camera_fb_get();
            currentFrame->buffer = cameraFrame->buf;
            currentFrame->length = cameraFrame->len;
            return currentFrame;
        });
        cameraHandler->onFrameRelease([]() {
            esp_camera_fb_return(cameraFrame);
        });
        cameraHandler->onFileSave([]() {
            auto rtc = Config::getRTC();
            char folderName[20];
            char fullPath[50];
            sprintf(folderName, "/%02d%02d%02d", rtc->getYear(), rtc->getMonth(), rtc->getDay());
            sprintf(fullPath, "%s/%02d%02d%02d%02d%02d%02d%03d.jpg", folderName, rtc->getYear(), rtc->getMonth(), rtc->getDay(), rtc->getHour(true), rtc->getMinute(), rtc->getSecond(), rtc->getMillis());
            if (Esp32Camera::sdcardInitialized || Esp32Camera::initMicroSDCard()) {
                // TODO: emit event `Camera.SDCard.LastSaved` Serial.println(fileName);
                return Esp32Camera::takeNewPhoto(folderName, fullPath);
            }
            return false;
        });
        // Get Camera handler module
        auto module = cameraHandler->getModuleList()->get(0);

        // Configure camera module UI widget
        module->setProperty(WidgetApi::DisplayModule, "homegenie/generic/camerainput");

        // Add options for image quality
        auto imageQuality = Config::getSetting(Camera_Sensor::ImageQuality, "20");
        module->addWidgetOption(
                // name, value
                Options::Image_Quality, imageQuality.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_SLIDER
                // label
                ":image_quality"
                // min:max:default
                ":10:63:20"
        )->withConfigKey(Camera_Sensor::ImageQuality)->addUpdateListener(&optionUpdateListener);

        // Add option for image size
        auto psramSize = esp_spiram_get_size();
        char resolutionOptions[512];
        if (psramSize == 0) { // NO PSRAM
            snprintf(resolutionOptions, sizeof(resolutionOptions),
                     UI_WIDGETS_FIELD_TYPE_SELECT
                     ":image_resolution"
                     ":QQVGA (160x120)=%d"
                     "|QCIF (176x144)=%d"
                     "|HQVGA (240x176)=%d"
                     "|240x240 (240x240)=%d"
                     "|QVGA (320x240)=%d",
                     (uint8_t)FRAMESIZE_QQVGA, (uint8_t)FRAMESIZE_QCIF,
                     (uint8_t)FRAMESIZE_HQVGA, (uint8_t)FRAMESIZE_240X240,
                     (uint8_t)FRAMESIZE_QVGA);
        } else if (psramSize <= 2097152) {
            snprintf(resolutionOptions, sizeof(resolutionOptions),
                     UI_WIDGETS_FIELD_TYPE_SELECT
                     ":image_resolution"
                     ":QQVGA (160x120)=%d"
                     "|QCIF (176x144)=%d"
                     "|HQVGA (240x176)=%d"
                     "|QVGA (320x240)=%d"
                     "|CIF (400x296)=%d"
                     "|HVGA (480x320)=%d"
                     "|VGA (640x480)=%d"
                     "|SVGA (800x600)=%d"
                     "|XGA (1024x768)=%d"
                     "|HD (1280x720)=%d",
                     (uint8_t)FRAMESIZE_QQVGA, (uint8_t)FRAMESIZE_QCIF,
                     (uint8_t)FRAMESIZE_HQVGA, (uint8_t)FRAMESIZE_QVGA, (uint8_t)FRAMESIZE_CIF,
                     (uint8_t)FRAMESIZE_HVGA, (uint8_t)FRAMESIZE_VGA, (uint8_t)FRAMESIZE_SVGA,
                     (uint8_t)FRAMESIZE_XGA, (uint8_t)FRAMESIZE_HD);
        } else {
            snprintf(resolutionOptions, sizeof(resolutionOptions),
                     UI_WIDGETS_FIELD_TYPE_SELECT
                     ":image_resolution"
                     // 2MP
                     ":QCIF (176x144)=%d"
                     "|HQVGA (240x176)=%d"
                     "|QVGA (320x240)=%d"
                     "|HVGA (480x320)=%d"
                     "|VGA (640x480)=%d"
                     "|SVGA (800x600)=%d"
                     "|XGA (1024x768)=%d"
                     "|HD (1280x720)=%d"
                     "|SXGA (1280x1024)=%d"
                     "|UXGA (1600x1200)=%d"
                     // 3MP
                     "|FHD (1920x1080)=%d"
                     "|P_HD (720x1280)=%d"
                     "|P_3MP (864x1536)=%d"
                     "|QXGA (2048x1536)=%d"
                     // 5MP
                     "|QHD (2560x1440)=%d"
                     "|WQXGA (2560x1600)=%d"
                     "|P_FHD (1080x1920)=%d"
                     "|QSXGA (2560x1920)=%d",
                     // 2MP
                     (uint8_t)FRAMESIZE_QCIF, (uint8_t)FRAMESIZE_HQVGA, (uint8_t)FRAMESIZE_QVGA,
                     (uint8_t)FRAMESIZE_HVGA, (uint8_t)FRAMESIZE_VGA, (uint8_t)FRAMESIZE_SVGA,
                     (uint8_t)FRAMESIZE_XGA, (uint8_t)FRAMESIZE_HD, (uint8_t)FRAMESIZE_SXGA,
                     (uint8_t)FRAMESIZE_UXGA,
                     // 3MP
                     (uint8_t)FRAMESIZE_FHD, (uint8_t)FRAMESIZE_P_HD, (uint8_t)FRAMESIZE_P_3MP,
                     (uint8_t)FRAMESIZE_QXGA,
                     // 5MP
                     (uint8_t)FRAMESIZE_QHD, (uint8_t)FRAMESIZE_WQXGA, (uint8_t)FRAMESIZE_P_FHD,
                     (uint8_t)FRAMESIZE_QSXGA);
        }
        auto imageResolution = Config::getSetting(Camera_Sensor::ImageResolution, "0");
        module->addWidgetOption(
                // name,   value
                Options::Image_Resolution, imageResolution.c_str(),
                resolutionOptions
        )->withConfigKey(Camera_Sensor::ImageResolution)->addUpdateListener(&optionUpdateListener);

        // Add options for brightness control
        auto brightnessControl = Config::getSetting(Camera_Sensor::BrightnessControl, "0");
        module->addWidgetOption(
                // name, value
                Options::Controls_Brightness, brightnessControl.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_SLIDER
                // label
                ":brightness_control"
                // min:max:default
                ":-2:2:0"
        )->withConfigKey(Camera_Sensor::BrightnessControl)->addUpdateListener(&optionUpdateListener);

        // Add options for saturation control
        auto saturationControl = Config::getSetting(Camera_Sensor::SaturationControl, "0");
        module->addWidgetOption(
                // name, value
                Options::Controls_Saturation, saturationControl.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_SLIDER
                // label
                ":saturation_control"
                // min:max:default
                ":-2:2:0"
        )->withConfigKey(Camera_Sensor::SaturationControl)->addUpdateListener(&optionUpdateListener);

        // Add options for contrast control
        auto contrastControl = Config::getSetting(Camera_Sensor::ContrastControl, "0");
        module->addWidgetOption(
                // name, value
                Options::Controls_Contrast, contrastControl.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_SLIDER
                // label
                ":contrast_control"
                // min:max:default
                ":-2:2:0"
        )->withConfigKey(Camera_Sensor::ContrastControl)->addUpdateListener(&optionUpdateListener);

        // Add option for image effects
        auto effectControl = Config::getSetting(Camera_Sensor::EffectControl, "0");
        module->addWidgetOption(
                // name,   value
                Options::Controls_Effect, effectControl.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_SELECT
                // label
                ":special_effect"
                // options (keys separated by pipe)
                ":no_effect=0"
                "|negative=1"
                "|grayscale=2"
                "|red_tint=3"
                "|green_tint=4"
                "|blue_tint=5"
                "|sepia=6"
        )->withConfigKey(Camera_Sensor::EffectControl)->addUpdateListener(&optionUpdateListener);

        // Add options for horizontal mirror
        auto horizontalMirror = Config::getSetting(Camera_Sensor::HMirrorControl, "0");
        module->addWidgetOption(
                // name, value
                Options::Controls_HMirror, horizontalMirror.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_CHECKBOX
                // label
                ":horizontal_mirror"
        )->withConfigKey(Camera_Sensor::HMirrorControl)->addUpdateListener(&optionUpdateListener);

        // Add options for vertical flip
        auto verticalFlip = Config::getSetting(Camera_Sensor::VFlipControl, "0");
        module->addWidgetOption(
                // name, value
                Options::Controls_VFlip, verticalFlip.c_str(),
                // type
                UI_WIDGETS_FIELD_TYPE_CHECKBOX
                // label
                ":vertical_flip"
        )->withConfigKey(Camera_Sensor::VFlipControl)->addUpdateListener(&optionUpdateListener);

    }

    void Esp32Camera::applySettings() {
        sensor_t * camera = esp_camera_sensor_get();
        camera->set_brightness(camera, Config::getSetting(Camera_Sensor::BrightnessControl, "0").toInt());     // -2 to 2
        camera->set_contrast(camera, Config::getSetting(Camera_Sensor::ContrastControl, "0").toInt());         // -2 to 2
        camera->set_saturation(camera, Config::getSetting(Camera_Sensor::SaturationControl, "0").toInt());     // -2 to 2
        camera->set_special_effect(camera, Config::getSetting(Camera_Sensor::EffectControl, "0").toInt());     // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        camera->set_whitebal(camera, 1);                                                      // 0 = disable , 1 = enable
        camera->set_awb_gain(camera, 1);                                                      // 0 = disable , 1 = enable
        camera->set_wb_mode(camera, 0);                                                       // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        camera->set_exposure_ctrl(camera, 1);                                                 // 0 = disable , 1 = enable
        camera->set_aec2(camera, 0);                                                          // 0 = disable , 1 = enable
        camera->set_ae_level(camera, 0);                                                      // -2 to 2
        camera->set_aec_value(camera, 300);                                                   // 0 to 1200
        camera->set_gain_ctrl(camera, 1);                                                     // 0 = disable , 1 = enable
        camera->set_agc_gain(camera, 0);                                                      // 0 to 30
        camera->set_gainceiling(camera, (gainceiling_t)0);                                    // 0 to 6
        camera->set_bpc(camera, 0);                                                           // 0 = disable , 1 = enable
        camera->set_wpc(camera, 1);                                                           // 0 = disable , 1 = enable
        camera->set_raw_gma(camera, 1);                                                       // 0 = disable , 1 = enable
        camera->set_lenc(camera, 1);                                                          // 0 = disable , 1 = enable
        camera->set_hmirror(camera, Config::getSetting(Camera_Sensor::HMirrorControl, "false").equals("true") ? 1 : 0);
        camera->set_vflip(camera, Config::getSetting(Camera_Sensor::VFlipControl, "false").equals("true") ? 1 : 0);
        camera->set_dcw(camera, 1);                                                           // 0 = disable , 1 = enable
        camera->set_colorbar(camera, 0);                                                      // 0 = disable , 1 = enable
    }

    void Esp32Camera::cameraStart(camera_grab_mode_t grabMode) {
        if (initialized) return;
#ifndef ESP32_S3
        // Disable WPS button on ESP32-CAM classic model
        // because GPIO0 is also used for X_CLOCK line
        // and would cause WPS reset when camera is enabled
        Config::ServiceButtonPin = -1;
#endif
        //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
        /*
        // TODO: add MMC support
        SD_MMC.begin();
        EEPROM.begin(16);
        */
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;

        // default settings works for ESP32-S3 CAMERA board and compatible
        config.pin_d0 = Config::getSetting(Camera_Sensor::CameraPin_cd0, ESP_CAMERA_CD0).toInt();
        config.pin_d1 = Config::getSetting(Camera_Sensor::CameraPin_cd1, ESP_CAMERA_CD1).toInt();
        config.pin_d2 = Config::getSetting(Camera_Sensor::CameraPin_cd2, ESP_CAMERA_CD2).toInt();
        config.pin_d3 = Config::getSetting(Camera_Sensor::CameraPin_cd3, ESP_CAMERA_CD3).toInt();
        config.pin_d4 = Config::getSetting(Camera_Sensor::CameraPin_cd4, ESP_CAMERA_CD4).toInt();
        config.pin_d5 = Config::getSetting(Camera_Sensor::CameraPin_cd5, ESP_CAMERA_CD5).toInt();
        config.pin_d6 = Config::getSetting(Camera_Sensor::CameraPin_cd6, ESP_CAMERA_CD6).toInt();
        config.pin_d7 = Config::getSetting(Camera_Sensor::CameraPin_cd7, ESP_CAMERA_CD7).toInt();
        config.pin_xclk = Config::getSetting(Camera_Sensor::CameraPin_xcl, ESP_CAMERA_XCL).toInt();
        config.pin_pclk = Config::getSetting(Camera_Sensor::CameraPin_pcl, ESP_CAMERA_PCL).toInt();
        config.pin_vsync = Config::getSetting(Camera_Sensor::CameraPin_vsn, ESP_CAMERA_VSN).toInt();
        config.pin_href = Config::getSetting(Camera_Sensor::CameraPin_hrf, ESP_CAMERA_HRF).toInt();
        config.pin_sccb_sda = Config::getSetting(Camera_Sensor::CameraPin_sda, ESP_CAMERA_SDA).toInt();
        config.pin_sccb_scl = Config::getSetting(Camera_Sensor::CameraPin_scl, ESP_CAMERA_SCL).toInt();

        config.pin_pwdn = Config::getSetting(Camera_Sensor::CameraPin_pwr, ESP_CAMERA_PWR).toInt();
        config.pin_reset = Config::getSetting(Camera_Sensor::CameraPin_rst, ESP_CAMERA_RST).toInt();

        config.xclk_freq_hz = Config::getSetting(Camera_Sensor::CameraPin_xfr, ESP_CAMERA_XFR).toInt();
        config.jpeg_quality = Config::getSetting(Camera_Sensor::ImageQuality, ESP_CAMERA_QUALITY).toInt();
        config.pixel_format = PIXFORMAT_JPEG;

        // Validate and apply frame size settings
        framesize_t res = (framesize_t)Config::getSetting(Camera_Sensor::ImageResolution, String(FRAMESIZE_QQVGA).c_str()).toInt();
        if (res < FRAMESIZE_QQVGA) {
            res = FRAMESIZE_QQVGA;
            Config::saveSetting(Camera_Sensor::ImageResolution, String(FRAMESIZE_QQVGA));
        }
        // Max 3MP resolution
        if (res > FRAMESIZE_QXGA) {
            res = FRAMESIZE_QXGA;
            Config::saveSetting(Camera_Sensor::ImageResolution, String(FRAMESIZE_QXGA));
        }
        config.frame_size = res;

        auto psramSize = esp_spiram_get_size();
        config.fb_count = (psramSize > 0) ? Config::getSetting(Camera_Sensor::CameraCfg_fbc, "2").toInt() : 1;
        config.grab_mode = grabMode;

        esp_err_t err = esp_camera_init(&config);
        if (err == ESP_OK) {
            initialized = true;
            applySettings();
        }
        if (err == ESP_ERR_NO_MEM) {
            // TODO: .. report errors via module property `Status.Error`
            Serial.println("\nESP32-CAMERA ERROR: insufficient memory.\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // TODO: .. report errors via module property `Status.Error`
            Serial.println("\nESP32-CAMERA ERROR: invalid configuration.\n");
        } else if (err == ESP_ERR_CAMERA_NOT_DETECTED) {
            // TODO: .. report errors via module property `Status.Error`
            Serial.println("\nESP32-CAMERA ERROR: camera not detected.\n");
        } else if (err != ESP_OK) {
            // TODO: .. report errors via module property `Status.Error`
            Serial.println("\nESP32-CAMERA ERROR: general error.\n");
        }
    }

    void Esp32Camera::cameraStop() {
        esp_err_t err = esp_camera_deinit();
        if (err != ESP_OK) {
            Serial.println("Error de-initializing camera sensor");
        }
        initialized = false;
    }

    void Esp32Camera::init(Camera* cameraHandler) {
        if (instance == nullptr) {
            instance = new Esp32Camera(cameraHandler);
        }
    }

}}

#endif