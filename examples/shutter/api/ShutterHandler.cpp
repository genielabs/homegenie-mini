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
 * - 2024-01-06 Initial release
 *
 */

#include "ShutterHandler.h"

namespace Service { namespace API {

    ShutterHandler::ShutterHandler() {

    }

    void ShutterHandler::init() {

        for (int i = 1; i <= 8; i++) {

            auto controlPin = Config::getSetting(K(ControlPin, i), "-1").toInt();
            if (controlPin < 0) continue;

            auto shutterControl = new ShutterControl(i);
            auto shutterModule = addModule(i);

            // TODO: maybe this cross referencing can be avoided
            //       incorporating module into shutterControl
            shutterControl->setModule(shutterModule);
            shutterModule->shutterControl = shutterControl;

            HomeGenie::getInstance()->addIOHandler(shutterControl);

            auto optionUpdateListener = new ParameterListener(shutterControl);

            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor, "0", // "0" = "Servo", "1" = "ServoEncoder"
                    // field type
                    UI_WIDGETS_FIELD_TYPE_SELECT_FILTER
                    // label
                    ":motor_type"
                    // options
                    ":motor_servo=Servo"
                    "|motor_servo_encoder=ServoEncoder"
// TODO: stepper driver not yet completed (wip)
//                "|motor_stepper=Stepper"
            )->withConfigKey(K(MotorType, i))->addUpdateListener(optionUpdateListener);

            // Servo options

            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor_Servo_AngleSpeed, "5",
                    // field type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":speed"
                    // min:max:default
                    ":1:10:5"
            )->withConfigKey(K(AngleSpeed, i))->addUpdateListener(optionUpdateListener);

            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor_Servo_AngleMin, "0",
                    // field type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":angle_min_value"
                    // min:max:default
                    ":0:180:0"
            )->withConfigKey(K(AngleMin, i))->addUpdateListener(optionUpdateListener);

            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor_Servo_AngleMax, "180",
                    // field type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":angle_max_value"
                    // min:max:default
                    ":0:180:180"
            )->withConfigKey(K(AngleMax, i))->addUpdateListener(optionUpdateListener);


            // ServoEncoder options
            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor_ServoEncoder_Speed, "5",
                    // field type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":speed"
                    // min:max:default
                    ":1:10:5"
            )->withConfigKey(K(MotorSpeed, i))->addUpdateListener(optionUpdateListener);

            shutterModule->addWidgetOption(
                    // name, value
                    Options::Shutter_Motor_ServoEncoder_Steps, "15",
                    // field type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":motor_steps"
                    // min:max:default
                    ":1:100:15"
            )->withConfigKey(K(MotorSteps, i))->addUpdateListener(optionUpdateListener);

            shutterModule->addWidgetOption(
                    // name, value (shown as button text)
                    Options::Shutter_Motor_ServoEncoder_Calibrate, Calibration::StartLabel,
                    // field type
                    UI_WIDGETS_FIELD_TYPE_BUTTON
                    // label
                    ":calibration"
                    // API command to call on click
                    ":Shutter.Calibrate"
            )->addUpdateListener(optionUpdateListener);

        }

    }

    bool ShutterHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {

        auto module = (ShutterModule*)getModule(command->Domain.c_str(), command->Address.c_str());

        if (module != nullptr) {
            auto shutterControl = module->shutterControl;

            if (command->Command == ControlApi::Control_Level) {

                float level = command->OptionsString.toFloat();
                shutterControl->setLevel(level);
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == ControlApi::Control_Toggle) {

                shutterControl->toggle();
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == ControlApi::Control_Close || command->Command == ControlApi::Control_Off) {

                shutterControl->close();
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == ControlApi::Control_Open || command->Command == ControlApi::Control_On) {

                shutterControl->open();
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == ShutterApi::Shutter_Calibrate) {

                shutterControl->calibrate();
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else {

                return false;

            }
            return true;
        }

        return false;
    }

    bool ShutterHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::Automation_Components);
    }

    bool ShutterHandler::handleEvent(IIOEventSender *sender,
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

    Module* ShutterHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            auto module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* ShutterHandler::getModuleList() {
        return reinterpret_cast<LinkedList<Data::Module *> *>(&moduleList);
    }

    ShutterModule* ShutterHandler::addModule(int index) {

        // Shutter module
        auto shutterModule = new ShutterModule();
        shutterModule->domain = IO::IOEventDomains::Automation_Components;
        shutterModule->address = String("S") + String(index);
        shutterModule->type = "Shutter";
        shutterModule->name = String("Motor ") + String(index);

        // Explicitly enable "scheduling" features for this module
        shutterModule->setProperty(WidgetApi::Implements::Scheduling, "true");

        // Set widget prefs
        // (if you want your motors be able to dance with music, uncomment the next line :)
        //shutterModule->setProperty(WidgetApi::Preference::AudioLight, "true");

        // Add level property
        auto shutterLevel = new ModuleParameter(IOEventPaths::Status_Level);
        shutterModule->properties.add(shutterLevel);

        moduleList.add(shutterModule);

        return shutterModule;
    }

}}
