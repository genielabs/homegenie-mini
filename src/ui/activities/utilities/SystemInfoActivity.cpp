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

#include "SystemInfoActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Utilities {

    SystemInfoActivity::SystemInfoActivity() {
        setDrawInterval(200);  // Task.h - 200ms loop frequency
        setColorDepth(lgfx::palette_2bit);
    }

    void SystemInfoActivity::onStart() {
        if (display->width() < display->height()) {
            diameter = (float)display->width();
        } else {
            diameter = (float)display->height();
        }
        center = diameter / 2.0f;
        //zoom = (float) (std::min(display->width(), display->height())) / diameter;
    }

    void SystemInfoActivity::onTap(PointerEvent e) {
        if (configDeviceButton != nullptr && e.target == configDeviceButton) {
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            String systemMode = preferences.getString(CONFIG_KEY_system_mode);
            // toggle config mode flag
            preferences.putString(CONFIG_KEY_system_mode, systemMode.equals("config") ? "" : "config");
            preferences.end();
            meditate();
            esp_restart();
        }
        if (rotateScreenButton != nullptr && e.target == rotateScreenButton) {
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            uint32_t displayRotation = preferences.getUInt(CONFIG_KEY_screen_rotation);
            if (displayRotation < 3) displayRotation++;
            else displayRotation = 0;
            // toggle config mode flag
            preferences.putUInt(CONFIG_KEY_screen_rotation, displayRotation);
            preferences.end();
            display->setRotation(displayRotation);
            // force re-allocation with new size
            pause();
            resume();
        }
    }

    void SystemInfoActivity::onResume() {
        if (display->width() < display->height()) {
            diameter = (float)display->width();
        } else {
            diameter = (float)display->height();
        }
        center = diameter / 2.0f;
        //zoom = (float) (std::min(display->width(), display->height())) / diameter;

        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

        // background color
        //canvas->setPaletteColor(0, 248, 248, 248);
        // text color
        canvas->setPaletteColor(1, 0, 160, 40);
        // accent color
        canvas->setPaletteColor(2, 0, 255, 255);
        // warn color
        canvas->setPaletteColor(3, 255, 255, 0);

        if (Config::isDeviceConfigured()) {

            if (configDeviceButton == nullptr) {
                configDeviceButton = new RoundButton(canvas, ox + center - 34, oy + center + 80, 64);
                configDeviceButton->setFont(&lgfx::fonts::TomThumb);
                configDeviceButton->setTitle("Reconfigure");
                configDeviceButton->setSubtitle("Device");
                addControl((InputControl*)configDeviceButton);
            } else {
                configDeviceButton->setLocation(ox + center - 34, oy + center + 80);
            }
            if (rotateScreenButton == nullptr) {
                rotateScreenButton = new RoundButton(canvas, ox + center + 34, oy + center + 80, 64);
                rotateScreenButton->setFont(&lgfx::fonts::TomThumb);
                rotateScreenButton->setTitle("Rotate");
                rotateScreenButton->setSubtitle("Display");
                addControl((InputControl*)rotateScreenButton);
            } else {
                rotateScreenButton->setLocation(ox + center + 34, oy + center + 80);
            }

        } else if (Config::isWiFiConfigured() || Config::system.systemMode.equals("config")) {

            if (configDeviceButton == nullptr) {
                configDeviceButton = new RoundButton(canvas, ox + 16, oy + center, 64);
                configDeviceButton->setFont(&lgfx::fonts::TomThumb);
                configDeviceButton->setTitle("Restore");
                configDeviceButton->setSubtitle("Config");
                addControl((InputControl*)configDeviceButton);
            } else {
                configDeviceButton->setLocation(ox + 16, oy + center);
            }

        }

        //canvas->clear();
    }

    void SystemInfoActivity::onPause() {
    }

    void SystemInfoActivity::onDraw() {
        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

        canvas->setTextColor(ActivityColors::TEXT);

        if (Config::isDeviceConfigured()) {

            canvas->fillCircle(ox + center, oy + center, center, ActivityColors::ACCENT);
            canvas->fillRect(ox + 0, oy + 48, 240, 144, 0);

            canvas->setFont(&fonts::Font8x8C64);

            // system name and version
            canvas->setCursor(ox + 19, oy + 58);
            canvas->print("**** HG-MINI OS V1.2 ****");

            auto homeGenie = HomeGenie::getInstance();
            auto systemModule = homeGenie->getDefaultModule();

            // free memory
            String bytesFree = "3583";
            auto bf = systemModule->getProperty(IOEventPaths::System_BytesFree);
            if (bf != nullptr && bf->value != nullptr) {
                bytesFree = bf->value;
            }
            canvas->setCursor(ox + 19, oy + 68);
            canvas->printf("%s BYTES FREE\n", bytesFree.c_str());
            canvas->setCursor(ox + 19, oy + 78);
            canvas->print("READY.");


            // device name
            canvas->setCursor(ox + 19, oy + 100);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("DEVICE ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(Config::system.friendlyName.c_str());

            // wi-fi status
            canvas->setCursor(ox + 27, oy + 120);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("WI-FI ");
            if (ESP_WIFI_STATUS == WL_CONNECTED) {
                canvas->setTextColor(ActivityColors::TEXT);
                canvas->print("CONNECTED");
            } else {
                canvas->setTextColor(ActivityColors::WARN);
                canvas->print("DISCONNECTED");
            }
            // MAC and IP addresses
            canvas->setCursor(ox + 27, oy + 130);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("  MAC ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(WiFi.macAddress().c_str());
            canvas->setCursor(ox + 27, oy + 140);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("   IP ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(WiFi.localIP().toString().c_str());

        } else {

            canvas->clear();

            canvas->setFont(&fonts::Font2);

            canvas->setCursor(ox + 110, oy + 4);
            canvas->print("GET");
            canvas->setCursor(ox + 71, oy + 20);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("HomeGenie Panel");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->setCursor(ox + 61, oy + 35);
            canvas->print("app on Google Play");
            canvas->setCursor(ox + 46, oy + 50);
            canvas->print("to configure this device");

            canvas->setFont(&fonts::TomThumb);
            // MAC address
            canvas->setCursor(ox + 74, oy + 222);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("Wi-Fi MAC  ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->println(WiFi.macAddress().c_str());

            canvas->drawBitmap(ox + 47, oy + 68, qrcode_map, 150, 150, 2);

        }

//        canvas->setCursor(19, 128);
//        canvas->setTextColor(ActivityColors::TEXT);
    }

    void SystemInfoActivity::meditate() {
        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

        display->clear();
        display->setFont(&lgfx::fonts::TomThumb);
        display->setTextColor(TFT_RED);
        display->setCursor(ox + 18, oy + 116);
        display->write("Software Success! (= Press left mouse button to continue.");
        display->setCursor(ox + 60, oy + 124);
        display->write("Guru Meditation #00000042.1010102A");
        delay(150);
        meditate_box(TFT_RED);
        delay(700);
        meditate_box(TFT_BLACK);
        delay(700);
        meditate_box(TFT_RED);
        delay(700);
        meditate_box(TFT_BLACK);
    }

    void SystemInfoActivity::meditate_box(const int color) {
        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

        display->drawRect(ox + 2, oy + 104, 236, 36, color);
        display->drawRect(ox + 3, oy + 105, 234, 34, color);
        display->drawRect(ox + 4, oy + 106, 232, 32, color);
    }

}}}

#endif // ENABLE_UI
