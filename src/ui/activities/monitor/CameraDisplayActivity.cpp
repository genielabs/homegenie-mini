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
    TaskHandle_t CameraDisplayActivity::taskHandle;
    WiFiClientSecure CameraDisplayActivity::secureClient;
    WiFiClient CameraDisplayActivity::wifiClient;
    CameraDisplayActivity* CameraDisplayActivity::cameraInstance{};
    CameraDisplayActivity* CameraDisplayActivity::previousInstance{};

    CameraDisplayActivity::CameraDisplayActivity(const char *moduleAddress) {
        //setDrawInterval(33); // Task.h
        setColorDepth(lgfx::rgb565_2Byte);
        setAlwaysOn(true);
        module.domain  = IO::IOEventDomains::HomeAutomation_HomeGenie;
        module.address = moduleAddress;
        module.type    = ModuleType::Sensor;
        // load stored name for this module
        auto key = String(CONFIG_KEY_ACTIVITY_TITLE); key.concat(moduleAddress);
        module.name = Config::getSetting(key.c_str(), moduleAddress);
        module.setProperty(Implements::Scheduling, "true");
        module.setProperty(Implements::Scheduling_ModuleEvents, "true");
        module.onNameUpdated = [this](const char* oldName, const char* newName) {
            auto key = String(CONFIG_KEY_ACTIVITY_TITLE);
            key.concat(module.address);
            Config::saveSetting(key.c_str(), newName);
        };
        moduleList.add(&module);
        HomeGenie::getInstance()->addAPIHandler(this);
        // Add UI control to set the camera module associated
        // to this CameraDisplayActivity
        optionUpdateListener = new RemoteCameraUrlUpdateListener(this);
        feedUrlConfigKey = String("rcam-"); feedUrlConfigKey.concat(moduleAddress);
        module.addWidgetOption(
                // name, value
                CameraApi::Property::RemoteCamera_EndPoint, "",
                // type
                UI_WIDGETS_FIELD_TYPE_MODULE_TEXT
                // options
                ":any"
                ":sensor"
                ":Widget.DisplayModule=homegenie/generic/camerainput"
                ":uri"
        )->withConfigKey(feedUrlConfigKey.c_str())->addUpdateListener(optionUpdateListener);
    }

    CameraDisplayActivity::~CameraDisplayActivity() {
        module.getProperty(CameraApi::Property::RemoteCamera_EndPoint)->removeUpdateListener(optionUpdateListener);
        delete optionUpdateListener;
        // TODO: IMPLEMENT FreeRTOS Task shutdown/delete
    }

    bool CameraDisplayActivity::canHandleDomain(String *domain) {
        return domain->equals(module.domain);
    }

    bool CameraDisplayActivity::handleRequest(APIRequest *request, ResponseCallback *callback) {
        return false;
    }

    bool CameraDisplayActivity::handleEvent(IIOEventSender *, const char *domain, const char *address,
                                            const char *eventPath, void *eventData, IOEventDataType) {
        return false;
    }

    LinkedList<Module *> *CameraDisplayActivity::getModuleList() {
        return &moduleList;
    }

    Module *CameraDisplayActivity::getModule(const char *domain, const char *address) {
        if (module.domain.equals(domain) && module.address.equals(address))
            return &module;
        return nullptr;
    }

    void CameraDisplayActivity::onTap(PointerEvent e) {
        showOnScreenDisplay = !showOnScreenDisplay;
    }

    void CameraDisplayActivity::onStart() {
        // Start JPEG feeder task
        if (taskHandle == nullptr) {
            xTaskCreate(
                    reinterpret_cast<TaskFunction_t>(CameraDisplayActivity::worker),
                    "FrameGetTask",
                    10240, // this might require some adjustments
                    this,
                    tskIDLE_PRIORITY,
                    &taskHandle
            );
        }
    }

    void CameraDisplayActivity::onPause() {
        isActivityReady = false;
        frameSize.reset();
        // set to idle task priority when paused
        if (previousInstance == nullptr) {
            vTaskPrioritySet(taskHandle, tskIDLE_PRIORITY);
        } else if (previousInstance != this) {
            cameraInstance = previousInstance;
            previousInstance = nullptr;
        }
    }

    void CameraDisplayActivity::onResume() {
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

        String urlKey = "rcam-";
        urlKey.concat(module.address);
        jpegFeedUrl = Config::getSetting(urlKey.c_str());
        String key = "rcam-res-";
        key.concat(module.address);
        imageResolution = Config::getSetting(key.c_str(), isEsp32Camera ? "5" : "3");
        key = "rcam-qlt-";
        key.concat(module.address);
        imageQuality = Config::getSetting(key.c_str(), "10");

        readyTimestamp = millis();
        feedError = OVERLAY_MESSAGE_CONNECTING;
        isActivityReady = true;
        // set to higher task priority when resumed
        vTaskPrioritySet(taskHandle, tskIDLE_PRIORITY + 5);
        previousInstance = cameraInstance;
        cameraInstance = this;
    }

    void CameraDisplayActivity::onDraw() {
#ifdef CONFIG_ENABLE_POWER_MANAGER
        // Keep screen always on
        Service::PowerManager::setActive();
#endif
        if (display->dmaBusy()) {
            return;
        }
        canvas->startWrite();
        if (imageData && imageLen > 0) {
            if (frameSize.width > 0 && (frameSize.width != view->width() || frameSize.height != view->height())) {
                // frame resolution changed
                view->deleteSprite();
                view->createSprite(frameSize.width, frameSize.height);
            }
            auto w = (float)canvas->width();
            auto hw = (w / 2);
            float zf = w / frameSize.width;
            view->drawJpg(imageData, imageLen);
            view->setPivot((frameSize.width / 2) - (drawOffset.x / zf), view->getPivotY());
            view->pushRotateZoom(0, zf, zf);
            imageLen = 0;
            if (showOnScreenDisplay) {
                auto frameInfoText = String(frameSize.width, 0);
                frameInfoText.concat("x");
                frameInfoText.concat(String(frameSize.height, 0));
                frameInfoText.concat(" @");
                frameInfoText.concat(String(averageFps, 1));
                canvas->setFont(&fonts::Font0);
                auto tw = canvas->textWidth(frameInfoText);
                canvas->setTextColor(TFT_WHITE, TFT_DARKGREEN);
                // Display Resolution and FPS
                canvas->fillRoundRect(hw - ((tw / 2) + 16), 4, tw + 32, 16, 4, TFT_DARKGREEN);
                canvas->setCursor(hw - (tw / 2), 8);
                canvas->print(frameInfoText);
                // Display name
                canvas->setTextColor(TFT_WHITE, TFT_BLACK);
                canvas->setFont(&fonts::Font0);
                canvas->drawCenterString(module.name, hw, canvas->height() - 20);
            }
        }
        // Display info / errors
        int cx = canvas->width() / 2;
        int cy = canvas->height() / 2;
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
        canvas->endWrite();
    }

    void CameraDisplayActivity::setJpegFeedUrl(const String &feedUrl) {
        jpegFeedUrl = feedUrl;
        // override config setting with transformed value (with 'GetPicture' API call appended)
        Config::saveSetting(feedUrlConfigKey.c_str(), jpegFeedUrl);
        IO::Logger::info("Remote camera module set to: %s", jpegFeedUrl.c_str());
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

    uint8_t* CameraDisplayActivity::getJpegImage(const String& url) {
        bufferSize = 0;
        auto parsedUrl = Utility::parseURL(url);
        WiFiClient* client;
        HTTPClient http;
        if (parsedUrl.protocol.equalsIgnoreCase("https")) {
            client = &secureClient;
            secureClient.setInsecure();
        } else {
            client = &wifiClient;
        }
        http.begin(*client, parsedUrl.url);
        if (parsedUrl.hasCredentials()) {
            http.setAuthorization(parsedUrl.username.c_str(), parsedUrl.password.c_str());
        }
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            uint8_t* imageBuffer = nullptr;
#ifdef ESP32
            if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0) {
                imageBuffer = (uint8_t*) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
            }
#endif
            if (!imageBuffer) {
                imageBuffer = (uint8_t*)malloc(len);
            }
            if (!imageBuffer) {
                feedError = "Could not allocate image buffer.";
                http.end();
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
                http.end();
                free(imageBuffer);
                return nullptr;
            }

            http.end();

            bufferSize = bytesRead;
            feedError = "";
            return imageBuffer;
        } else {
            feedError = "Connection error...";
            http.end();
            return nullptr;
        }
    }

    [[noreturn]] void* CameraDisplayActivity::worker(void* cd) {
        unsigned long fpsStart = millis();
        unsigned long lastFrameTs{};
        unsigned int fps{};
        for(;;) {
            long dts{};
            auto c = cameraInstance;
            if (c && !c->jpegFeedUrl.isEmpty() && c->isActivityReady && WiFi.isConnected()) {
                uint8_t* bufferData = nullptr;
                lastFrameTs = millis();
#ifdef ESP_CAMERA_SUPPORTED
                if (c->isEsp32Camera) {
                    auto fb = Sensors::Esp32Camera::cameraGetFrame(c->imageResolution.toInt(), c->imageQuality.toInt());
                    if (fb != nullptr) {
                        bufferData = fb->buffer;
                        c->bufferSize = fb->length;
                        c->feedError = "";
                    } else {
                        c->feedError = "CAM Error: No Frame";
                    }
                } else {
#endif
                    auto url = c->jpegFeedUrl;
                    if (url.endsWith("/Camera.GetPicture") || url.endsWith("/Camera.GetPicture/")) {
                        // detected HomeGenie Camera API, set configured resolution
                        if (!url.endsWith("/")) {
                            url.concat("/");
                        }
                        url.concat(c->imageResolution);  // ESP32-CAM resolution (e.g. 3 = 240x176)
                        url.concat("/");
                        url.concat(c->imageQuality);     // ESP32-CAM quality, 10 (best) to 70 (lowest)
                    }
                    bufferData = c->getJpegImage(url.c_str());
#ifdef ESP_CAMERA_SUPPORTED
                }
#endif
                if (c && bufferData && c->bufferSize > 0) {
                    dts = millis() - lastFrameTs;
                    if (c->frameSize.width == 0) {
                        c->frameSize = getJpegDimensions(bufferData, c->bufferSize);
                    }
                    if (c->frameSize.width > 0) {
                        // if imageLen is 0 onDraw() is ready to receive a new fram
                        if (c->imageData && c->imageLen == 0) {
                            // release previous onDraw() image
                            free(c->imageData);
                            c->imageData = nullptr;
                        }
                        // copy new buffer to imageData for next onDraw() refresh
                        if (!c->imageData) {
#ifdef ESP32
                            if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0) {
                                c->imageData = (uint8_t*) heap_caps_malloc(c->bufferSize, MALLOC_CAP_SPIRAM);
                            }
#endif
                            if (!c->imageData) {
                                c->imageData = (uint8_t *)malloc(c->bufferSize);
                            }
                            if (c->imageData) {
                                memcpy(c->imageData, bufferData, c->bufferSize);
                                c->imageLen = c->bufferSize;
                                // Calculate actual display FPS
                                fps++;
                                if (millis() - fpsStart > 5000) {
                                    c->averageFps = ((float) fps / 5.0f);
                                    fpsStart = millis();
                                    if (c->averageFps < 1 && fps > 0) {
                                        c->feedError = "Bad link quality.";
                                    } else {
                                        c->feedError = "";
                                    }
                                    fps = 0;
                                }
                            }
                        }
                    }
#ifdef ESP_CAMERA_SUPPORTED
                    if (c->isEsp32Camera) {
                        Sensors::Esp32Camera::cameraReleaseFrame();
                    } else {
#endif
                        free(bufferData);
#ifdef ESP_CAMERA_SUPPORTED
                    }
#endif
                }
            }
            long delay = (100 - dts); // 10fps max.
            if (delay < 1) {
                delay = 1;
            }
            vTaskDelay((c && c->isActivityReady ? delay : 200) * portTICK_PERIOD_MS);
        }
    }

}}}

#endif // ENABLE_UI
