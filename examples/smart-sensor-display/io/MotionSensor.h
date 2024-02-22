//
// Created by gene on 19/02/24.
//

#ifndef HOMEGENIE_MINI_MOTIONSENSOR_H
#define HOMEGENIE_MINI_MOTIONSENSOR_H

#include <HomeGenie.h>

#include "../configuration.h"

#define MOTION_SENSOR_NS_PREFIX           "IO::Env::MotionSensor"

namespace IO { namespace Env {

    using namespace Service;

    class MotionSensor : Task, public IIOEventSender {
    public:
        MotionSensor(uint8_t pin) {
            setLoopInterval(200);
            sensorPin = pin;
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            auto motionDetect = new ModuleParameter(IOEventPaths::Sensor_MotionDetect);
            m->properties.add(motionDetect);
        }
        void begin() override;
        void loop() override;

    private:
        uint8_t sensorPin = 0;
        bool motionDetected = false;
    };

}} // Env

#endif //HOMEGENIE_MINI_MOTIONSENSOR_H
