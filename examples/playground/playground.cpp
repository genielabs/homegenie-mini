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
 * - 2019-01-10 v1.0: initial release.
 *
 */

#include <HomeGenie.h>

#include "configuration.h"

using namespace IO;
using namespace Service;

HomeGenie* homeGenie;

/*
#    ofekp/TinyUPnP@3.2.0

#define UPNP_DEBUG 1
#include <TinyUPnP.h>

TinyUPnP tinyUPnP(10000);

void ssdpDeviceToString(ssdpDevice* d) {
    Serial.printf("SSDP device [%s] port [%d] path [%s]\n",
                  d->host.toString().c_str(), d->port, d->path.c_str());
}

void ssdpRefresh() {
    vTaskDelay(10000 * portTICK_PERIOD_MS);
    for (;;) {
        if (Config::isDeviceConfigured()) {

            Serial.println("\nSEARCHING SSDP devices...\n");
            //*
            ssdpDeviceNode *ssdpDeviceNodeList = tinyUPnP.listSsdpDevices();
            ssdpDeviceNode *node = ssdpDeviceNodeList;
            while (node != nullptr) {
                auto device = node->ssdpDevice;
                ssdpDeviceToString(device);
                auto prev = node;
                node = node->next;
                delete device;
                delete prev;
            }
            //* /
            //vTaskDelay(200 * portTICK_PERIOD_MS);
            Serial.println("\n----------------------\n");

        }
        vTaskDelay(60000 * portTICK_PERIOD_MS);
    }
}
*/

void setup() {
    homeGenie = HomeGenie::getInstance();
    //auto miniModule = homeGenie->getDefaultModule();

    // TODO: ..

    homeGenie->begin();

/*
    xTaskCreate(
            reinterpret_cast<TaskFunction_t>(&ssdpRefresh),
            "SSDP-Check",
            ESP_TASK_TIMER_STACK,
            nullptr,
            CONFIG_ARDUINO_UDP_TASK_PRIORITY,
            nullptr
    );
*/

}

void loop()
{
    homeGenie->loop();

    // TODO: ..

}
