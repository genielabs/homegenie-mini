//
// Created by gene on 02/02/24.
//

#ifndef HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H
#define HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H

#include "HomeGenie.h"

#ifndef DISABLE_UI

#include "ui/Activity.h"
#include "ui/components/RoundButton.h"
#include "ui/bitmaps/HomeGenieLogo.h"

namespace UI  { namespace Activities { namespace Utilities {

    using namespace Service;
    using namespace UI::Components;

    class SystemInfoActivity: public Activity {
    public:
        SystemInfoActivity() {
            setDrawInterval(200);  // Task.h - 200ms loop frequency
        }

        void attach(lgfx::LGFX_Device* displayDevice) override {
            this->Activity::attach(displayDevice);

            diameter = (float) display->width();
            center = diameter / 2.0f;
            //zoom = (float) (std::min(display->width(), display->height())) / diameter;
        }

        void onResume() override;

        void onPause() override;

        void onDraw() override;

        void onTap(PointerEvent e) override {
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
                invalidate();
            }
        }
        void onSwipe(SwipeEvent e) override {}
        void onPan(PanEvent e) override {}
        void onTouch(PointerEvent e) override {}
        void onRelease(PointerEvent e) override {}


    private:
        RoundButton* configDeviceButton = nullptr;;
        RoundButton* rotateScreenButton = nullptr;;
        float center, diameter;
        enum ActivityColors {
            BACKGROUND,
            TEXT,
            ACCENT,
            WARN
        };

        void meditate() {
            display->clear();
            display->setFont(&lgfx::fonts::TomThumb);
            display->setTextColor(TFT_RED);
            display->setCursor(18, 116);
            display->write("Software Success! (= Press left mouse button to continue.");
            display->setCursor(60, 124);
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
        void meditate_box(const int color) {
            display->drawRect(2, 104, 236, 36, color);
            display->drawRect(3, 105, 234, 34, color);
            display->drawRect(4, 106, 232, 32, color);
        }

    };

}}}

#endif //DISABLE_UI

#endif //HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H
