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
 */

#include "CameraDisplayActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Monitor {

    void CameraDisplayActivity::attach(LGFX_Device* displayDevice) {
        Activity::attach(displayDevice);
        // Start JPEG feeder task
        xTaskCreate(
            reinterpret_cast<TaskFunction_t>(CameraDisplayActivity::worker),
            "RenderRemoteImageTask",
            10240, // this might require some adjustments
            this,
            tskIDLE_PRIORITY + 15,
            nullptr
        );
    }

    void CameraDisplayActivity::onResume() {
        canvas->setColorDepth(lgfx::rgb565_2Byte);

        if (view != nullptr) {
            view->deleteSprite();
            delete view;
            view = nullptr;
        }
        view = new LGFX_Sprite(canvas);
#ifdef BOARD_HAS_PSRAM
        view->setPsram(true);
#endif
        view->setColorDepth(lgfx::rgb565_2Byte);
        view->createSprite(160, 120);

        String key = "rcam-res-";
        key.concat(module.address);
        imageResolution = Config::getSetting(key.c_str(), isEsp32Camera ? "5" : "1");
        key = "rcam-qlt-";
        key.concat(module.address);
        imageQuality = Config::getSetting(key.c_str(), "10");

        isActivityReady = true;
        readyTimestamp = millis();
        feedError = OVERLAY_MESSAGE_CONNECTING;
    }

    void CameraDisplayActivity::onPause() {
        isActivityReady = false;
        http.end(); // terminate keep-alive connection
        frameSize.reset();
        // avoid disposing object used by the download thread here
    }

    void CameraDisplayActivity::onDraw() {
#ifdef CONFIG_ENABLE_POWER_MANAGER
        // Keep screen always on
        Service::PowerManager::setActive();
#endif
        int cx = canvas->width() / 2;
        int cy = canvas->height() / 2;
        // Display errors
        canvas->setCursor(0, cy);
        if (jpegFeedUrl.isEmpty()) {
            canvas->setTextColor(TFT_WHITE, TFT_BLUE);
            canvas->fillGradientRect(0, 0, canvas->width(), canvas->height(), 0x880000U, 0x000088U);
            canvas->setFont(&fonts::DejaVu12);
            canvas->drawCenterString("Image feed URL not set.", cx, cy - 24);
            canvas->drawCenterString("Use HomeGenie panel app", cx, cy);
            canvas->drawCenterString("to configure it.", cx, cy + 16);
        } else if (!feedError.isEmpty()) {
            bool isConnecting = feedError.equals(OVERLAY_MESSAGE_CONNECTING);
            if (isConnecting || (millis() - readyTimestamp) > 5000) {
                canvas->setTextColor(TFT_WHITE, isConnecting ? TFT_BLUE : TFT_RED);
                canvas->setFont(&fonts::FreeSansBoldOblique9pt7b);
                canvas->drawCenterString(feedError, cx, cy - 8);
            }
        }
    }


    JpegDimensions CameraDisplayActivity::getJpegDimensions(const uint8_t* jpg_data, size_t data_size) {
      JpegDimensions dims;
      for (size_t i = 0; i < data_size - 9; ++i) {
        if (jpg_data[i] == 0xFF) {
          uint8_t marker_type = jpg_data[i + 1];
          if (marker_type == 0xC0 || marker_type == 0xC1 || marker_type == 0xC2) {
            if (i + 8 < data_size) {
              dims.height = (jpg_data[i + 5] << 8) | jpg_data[i + 6];
              dims.width  = (jpg_data[i + 7] << 8) | jpg_data[i + 8];
              return dims;
            } else {
              return dims;
            }
          } else if (marker_type == 0xFF) {
            continue;
          } else if (marker_type == 0x00) {
            i++;
            continue;
          } else if (marker_type >= 0xD0 && marker_type <= 0xD7) { // RSTn markers
            continue;
          } else if (marker_type == 0xD8 || marker_type == 0xD9) { // SOI o EOI
            continue;
          }
        }
      }
      return dims; // Not found
    }

    [[noreturn]] void* CameraDisplayActivity::worker(void* cd) {
        auto cameraDisplay = (CameraDisplayActivity*)cd;
        unsigned long fpsStart = millis();
        unsigned int fps = 0;
        float averageFps = 0;
        unsigned long lastFrameTs = 0;
        for(;;) {
            long dts = 0;
            if (cameraDisplay != nullptr && !cameraDisplay->jpegFeedUrl.isEmpty() && cameraDisplay->isActivityReady && WiFi.isConnected() && !cameraDisplay->canvas->dmaBusy()) {
                auto canvas = cameraDisplay->canvas;
                auto view = cameraDisplay->view;
                auto w = (float)canvas->width();
                auto hw = (w / 2);
                lastFrameTs = millis();
                uint8_t* imageData = nullptr;
#ifdef ESP_CAMERA_SUPPORTED
                if (cameraDisplay->isEsp32Camera) {
                    auto fb = Sensors::Esp32Camera::cameraGetFrame(cameraDisplay->imageResolution.toInt(), cameraDisplay->imageQuality.toInt()); // (resolution 5 = 320x240, quality 10)
                    if (fb != nullptr) {
                        imageData = fb->buffer;
                        cameraDisplay->imageLen = fb->length;
                        cameraDisplay->feedError = "";
                    } else {
                        cameraDisplay->feedError = "CAM Error: No Frame";
                    }
                } else {
#endif
                    auto url = cameraDisplay->jpegFeedUrl;
                    url.concat("/");
                    url.concat(cameraDisplay->imageResolution);  // ESP32-CAM resolution (e.g. 3 = 240x176)
                    url.concat("/");
                    url.concat(cameraDisplay->imageQuality);     // ESP32-CAM quality, 10 (best) to 70 (lowest)
                    imageData = cameraDisplay->feedJpegImage(url.c_str());
#ifdef ESP_CAMERA_SUPPORTED
                }
#endif
                if (imageData != nullptr && cameraDisplay->imageLen > 0 && cameraDisplay->isActivityReady) {
                    dts = millis() - lastFrameTs;
                    if (cameraDisplay->frameSize.width == 0) {
                        cameraDisplay->frameSize = getJpegDimensions(imageData, cameraDisplay->imageLen);
                        if (cameraDisplay->frameSize.width > 0) {
                            view->deleteSprite();
                            view->createSprite(cameraDisplay->frameSize.width, cameraDisplay->frameSize.height);
                        }
                    }
                    float zf = w / cameraDisplay->frameSize.width;
                    canvas->startWrite(false);
                    view->drawJpg(imageData, cameraDisplay->imageLen);
                    view->setPivot((cameraDisplay->frameSize.width / 2) - (cameraDisplay->drawOffset.x / zf), cameraDisplay->view->getPivotY());
                    view->pushRotateZoom(0, zf, zf);
                    if (cameraDisplay->showOnScreenDisplay) {
                        auto fpsText = String(averageFps, 1);
                        canvas->setFont(&fonts::Font0);
                        auto tw = canvas->textWidth(fpsText);
                        canvas->setTextColor(TFT_WHITE, TFT_DARKGREEN);
                        // Display FPS
                        canvas->fillRoundRect(hw - 16, 4, 32, 16, 4, TFT_DARKGREEN);
                        canvas->setCursor(hw - (tw / 2), 8);
                        canvas->print(fpsText);
                        // Display name
                        canvas->setTextColor(TFT_WHITE, TFT_TRANSPARENT);
                        canvas->setFont(&fonts::Font0);
                        canvas->drawCenterString(cameraDisplay->module.name, hw, canvas->height() - 28);
                    }
                    canvas->endWrite();
                    fps++;
                    if (millis() - fpsStart > 5000) {
                        averageFps = ((float)fps / 5.0f);
                        fpsStart = millis();
                        if (averageFps < 1 && fps > 0) {
                            cameraDisplay->feedError = "Bad link quality.";
                        } else {
                            cameraDisplay->feedError = "";
                        }
                        fps = 0;
                    }
                }
#ifdef ESP_CAMERA_SUPPORTED
                if (cameraDisplay->isEsp32Camera) {
                    Sensors::Esp32Camera::cameraReleaseFrame();
                } else {
#endif
                    free(imageData);
#ifdef ESP_CAMERA_SUPPORTED
                }
#endif
            }
            long delay = (50 - dts);
            if (delay < 1) {
                delay = 1;
            }
            vTaskDelay((cameraDisplay != nullptr && cameraDisplay->isActivityReady ? delay : 100) * portTICK_PERIOD_MS);
        }
    }

    uint8_t* CameraDisplayActivity::feedJpegImage(const char* url) {
        imageLen = 0;
        //HTTPClient http; // moved to global object to implement keep-alive
        http.setReuse(true);
        http.setConnectTimeout(3000);
        http.setTimeout(1500);
        http.begin(url);
        http.addHeader("Connection", "keep-alive");
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            uint8_t* imageBuffer = nullptr;
#ifdef BOARD_HAS_PSRAM
            imageBuffer = (uint8_t*)ps_malloc(len);
#else
            imageBuffer = (uint8_t*)malloc(len);
#endif
            if (!imageBuffer) {
                feedError = "Could not allocate image buffer.";
                //http.end(); // prefer kee-alive
                return nullptr;
            }
            WiFiClient* stream = http.getStreamPtr();
            size_t bytesRead = 0;
            unsigned long startTime = millis();
            while (http.connected() && (bytesRead < len) && (millis() - startTime < 1500)) { // Timeout 1500ms
                if (stream->available()) {
                    bytesRead += stream->readBytes(&imageBuffer[bytesRead], min((size_t)(len - bytesRead), (size_t)HTTP_TCP_BUFFER_SIZE));
                }
            }

            if (bytesRead != len) {
                feedError = "Unexpected end of input";
                Serial.printf("CameraDisplayActivity: %s (%d out of %d bytes read).\n", feedError.c_str(), bytesRead, len);
                //http.end(); // prefer kee-alive
                free(imageBuffer);
                return nullptr;
            }

            //http.end(); // prefer kee-alive

            imageLen = bytesRead;
            feedError = "";
            return imageBuffer;
        } else {
            feedError = "Connection error...";
            //http.end(); // prefer kee-alive
            return nullptr;
        }
    }

}}}

#endif // ENABLE_UI
