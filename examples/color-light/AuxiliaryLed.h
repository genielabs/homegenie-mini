/*
 * HomeGenie-Mini (c) 2018-2026 G-Labs
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

#pragma once

#include <service/api/CommonApi.h>
#include <service/api/devices/ColorLight.h>
#include <Adafruit_NeoPixel.h>

// Owns the optional physical indicator; C2 is its independent color control.
// Status replication never changes C2's state or emits per-frame module events.
class AuxiliaryLed : public Data::ModuleParameter::UpdateListener {
private:
    Adafruit_NeoPixel* led = nullptr;
    Service::API::devices::ColorLight* colorLight = nullptr;
    uint32_t statusColor = 0;
    uint32_t independentColor = 0;
    bool independent = false;
    bool dirty = false;

    void writeColor(uint32_t color) {
        if (led != nullptr && led->getPixelColor(0) != color) {
            led->setPixelColor(0, color);
            dirty = true;
        }
    }

public:
    bool setup(int statusLedPin) {
        const int pin = Config::getSetting("auld-pin", "-1").toInt();
        if (pin < 0) return false;
        const int stripPin = Config::getSetting("leds-pin", "-1").toInt();
        if (pin == statusLedPin || pin == stripPin || pin == Config::StatusLedPin) {
            IO::Logger::error("Auxiliary LED pin conflicts with an existing LED output.");
            return false;
        }
        const int type = Config::getSetting("auld-typ", "82").toInt();
        const int speed = Config::getSetting("auld-spd", "0").toInt();
        independent = Config::getSetting("auld-mod", "status").equals("independent");
        led = new Adafruit_NeoPixel(1, pin, type + speed);
        led->begin();
        led->setPixelColor(0, 0);
        dirty = true;
        show();

        colorLight = new Service::API::devices::ColorLight(
            IO::IOEventDomains::HomeAutomation_HomeGenie, "C2", "Auxiliary Light");
        colorLight->module->setProperty(Service::API::WidgetApi::Implements::Scheduling, "true");
        colorLight->module->setProperty(Service::API::WidgetApi::Implements::Scheduling_ModuleEvents, "true");
        colorLight->onSetColor([this](Service::API::devices::LightColor color) {
            independentColor = color.getColor();
            if (independent) {
                writeColor(independentColor);
                show();
            }
        });
        colorLight->module->addWidgetOption(
            "LED.mode", independent ? "independent" : "status",
            UI_WIDGETS_FIELD_TYPE_SELECT ":Auxiliary LED mode:status|independent"
        )->withConfigKey("auld-mod")->addUpdateListener(this);
        Service::HomeGenie::getInstance()->addAPIHandler(colorLight);
        return true;
    }

    void onUpdate(Data::ModuleParameter* option) override {
        if (!option->is("LED.mode")) return;
        independent = option->value.equals("independent");
        writeColor(independent ? independentColor : statusColor);
        show();
    }

    void setStatusColor(uint32_t color) {
        statusColor = color;
        if (!independent) writeColor(color);
    }

    void show() {
        if (led != nullptr && dirty) {
            led->show();
            dirty = false;
        }
    }
};
