//
// Created by gene on 08/01/24.
//

#ifndef HOMEGENIE_MINI_STEPPERDRIVER_H
#define HOMEGENIE_MINI_STEPPERDRIVER_H


#include "examples/shutter/io/IShutterDriver.h"

#include "io/Logger.h"

#define STEPPER_DRIVER_NS_PREFIX "IO::Components:ShutterControl::StepperDriver"

namespace IO { namespace Components {
        class StepperDriver: public IShutterDriver {
        private:
            // TODO: ...
        public:
            StepperDriver() {}
            void init() {
                //Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", SERVO_CONTROL_NS_PREFIX, servoPin, minUs, maxUs);
                // TODO: ...
                Logger::info("|  ✔ %s", STEPPER_DRIVER_NS_PREFIX);
            }
            void stop() {
                // TODO: ...
            }
            void open() {
                // TODO: ...
            }
            void close() {
                // TODO: ...
            }
        };
    }}


#endif //HOMEGENIE_MINI_STEPPERDRIVER_H
