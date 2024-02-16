//
// Created by gene on 02/02/24.
//

#include "SystemInfoActivity.h"

#ifndef DISABLE_UI

namespace UI { namespace Activities { namespace Utilities {

    void SystemInfoActivity::onResume() {
        canvas->setColorDepth(lgfx::color_depth_t::palette_2bit);
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
                configDeviceButton = new RoundButton(canvas, center - 34, center + 80, 64);
                configDeviceButton->setFont(&lgfx::fonts::TomThumb);
                configDeviceButton->setTitle("Reconfigure");
                configDeviceButton->setSubtitle("Device");
                addControl((InputControl*)configDeviceButton);
            }
            if (rotateScreenButton == nullptr) {
                rotateScreenButton = new RoundButton(canvas, center + 34, center + 80, 64);
                rotateScreenButton->setFont(&lgfx::fonts::TomThumb);
                rotateScreenButton->setTitle("Rotate");
                rotateScreenButton->setSubtitle("Display");
                addControl((InputControl*)rotateScreenButton);
            }

        } else if (Config::isWiFiConfigured()) {

            if (configDeviceButton == nullptr) {
                configDeviceButton = new RoundButton(canvas, 16, center, 64);
                configDeviceButton->setFont(&lgfx::fonts::TomThumb);
                configDeviceButton->setTitle("Restore");
                configDeviceButton->setSubtitle("Config");
                addControl((InputControl*)configDeviceButton);
            }

        }

        //canvas->clear();
    }

    void SystemInfoActivity::onPause() {
    }

    void SystemInfoActivity::onDraw() {

        canvas->setTextColor(ActivityColors::TEXT);

        if (Config::isDeviceConfigured()) {

            canvas->fillCircle(center, center, center, ActivityColors::ACCENT);
            canvas->fillRect(0, 48, 240, 144, 0);

            canvas->setFont(&fonts::Font8x8C64);

            // system name and version
            canvas->setCursor(19, 58);
            canvas->print("**** HG-MINI OS V1.2 ****");

            auto homeGenie = HomeGenie::getInstance();
            auto systemModule = homeGenie->getDefaultModule();

            // free memory
            String bytesFree = "3583";
            auto bf = systemModule->getProperty(IOEventPaths::System_BytesFree);
            if (bf != nullptr && bf->value != nullptr) {
                bytesFree = bf->value;
            }
            canvas->setCursor(19, 68);
            canvas->printf("%s BYTES FREE\n", bytesFree.c_str());
            canvas->setCursor(19, 78);
            canvas->print("READY.");


            // device name
            canvas->setCursor(19, 100);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("DEVICE ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(Config::getDeviceName().c_str());

            // wi-fi status
            canvas->setCursor(27, 120);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("WI-FI ");
            if (WiFiClass::status() == WL_CONNECTED) {
                canvas->setTextColor(ActivityColors::TEXT);
                canvas->print("CONNECTED");
            } else {
                canvas->setTextColor(ActivityColors::WARN);
                canvas->print("DISCONNECTED");
            }
            // MAC and IP addresses
            canvas->setCursor(27, 130);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("  MAC ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(WiFi.macAddress().c_str());
            canvas->setCursor(27, 140);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("   IP ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->print(WiFi.localIP().toString().c_str());

        } else {

            canvas->clear();

            canvas->setFont(&fonts::Font2);

            canvas->setCursor(110, 4);
            canvas->print("GET");
            canvas->setCursor(71, 20);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("HomeGenie Panel");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->setCursor(61, 35);
            canvas->print("app on Google Play");
            canvas->setCursor(46, 50);
            canvas->print("to configure this device");

            canvas->setFont(&fonts::TomThumb);
            // MAC address
            canvas->setCursor(74, 222);
            canvas->setTextColor(ActivityColors::ACCENT);
            canvas->print("Wi-Fi MAC  ");
            canvas->setTextColor(ActivityColors::TEXT);
            canvas->println(WiFi.macAddress().c_str());

            canvas->drawBitmap(47, 68, qrcode_map, 150, 150, 2);

        }

//        canvas->setCursor(19, 128);
//        canvas->setTextColor(ActivityColors::TEXT);

    }

}}}

#endif
