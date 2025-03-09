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

#include "Camera.h"

namespace Service { namespace API { namespace devices {

    Camera::Camera(const char *domain, const char *address, const char *name) {
        module = new Module();
        module->domain = domain;
        module->address = address;
        module->type = ModuleApi::ModuleType::Sensor;
        module->name = name;

        // Set the UI widget used to display this module
        module->setProperty(WidgetApi::DisplayModule, "homegenie/generic/camerainput");

        moduleList.add(module);
    }

    void Camera::init() {
    }

    bool Camera::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (command->Command == "Camera.GetPicture") {

            FrameBuffer* fb = setOnFrameRequestCallback();
            responseCallback->writeBinary("image/jpeg", fb->buffer, fb->length);
            setOnFrameReleaseCallback();

            return true;
        }
        return false;
    }

    bool Camera::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool Camera::handleEvent(IIOEventSender *sender,
                                     const char* domain, const char* address,
                                     const char *eventPath, void *eventData, IOEventDataType dataType) {
        auto module = getModule(domain, address);
        if (module) {
            auto event = String((char *) eventPath);
            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = QueuedMessage(domain, address, event.c_str(), "",
                                   nullptr, IOEventDataType::Undefined);
            // Data type handling
            switch (dataType) {
                case Number:
                    m.value = String(*(int32_t *) eventData);
                    break;
                case Float:
                    m.value = String(*(float *) eventData);
                    break;
                case Text:
                    m.value = String(*(String *) eventData);
                    break;
                default:
                    m.value = String(*(int32_t *) eventData);
            }
            module->setProperty(event, m.value,
                                nullptr, IOEventDataType::Undefined);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }
        return false;
    }

    Module* Camera::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            auto module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* Camera::getModuleList() {
        return reinterpret_cast<LinkedList<Data::Module *> *>(&moduleList);
    }

}}}
