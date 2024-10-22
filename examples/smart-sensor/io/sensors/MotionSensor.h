//
// Created by gene on 19/02/24.
//

#ifndef HOMEGENIE_MINI_MOTIONSENSOR_H
#define HOMEGENIE_MINI_MOTIONSENSOR_H

#include <HomeGenie.h>

#define MOTION_SENSOR_NS_PREFIX           "IO::Sensors::MotionSensor"

namespace IO { namespace Sensors {

    using namespace Service;

    class MotionSensor : Task, public IIOEventSender {
    public:
        explicit MotionSensor(uint8_t pin) {
            setLoopInterval(200);
            sensorPin = pin;
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            auto motionDetect = new ModuleParameter(IOEventPaths::Sensor_MotionDetect, "0");
            m->properties.add(motionDetect);
            auto idleTime = new ModuleParameter(IOEventPaths::Status_IdleTime, "0");
            m->properties.add(idleTime);
        }
        void begin() override;
        void loop() override;

    private:
        uint8_t sensorPin = 0;
        bool motionDetected = false;
        unsigned long idleTime = 0;
        unsigned long lastUpdate = 0;
        void clearIdle();
        void updateIdle();
    };

}} // Sensors

#endif //HOMEGENIE_MINI_MOTIONSENSOR_H
