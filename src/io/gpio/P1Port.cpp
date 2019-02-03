//
// Created by gene on 03/02/19.
//

#include "P1Port.h"

namespace IO { namespace GPIO {

    static uint8_t IOPin[] = { D5, D6, D7, D8 };

    void P1Port::setOutput(uint8_t pinNumber, uint8_t value) {
        auto pin = IOPin[pinNumber-1];
        float level = (value/P1PORT_GPIO_LEVEL_MAX);
        // address is a private member inherited from `IOEventSender` superclass
        address = (const uint8_t*)(String("D")+pinNumber).c_str();
        pinMode(pin, OUTPUT);
        if (value == P1PORT_GPIO_LEVEL_MAX) {
            digitalWrite(pin, HIGH);
        } else if (value == P1PORT_GPIO_LEVEL_MIN) {
            digitalWrite(pin, LOW);
        } else if (value > P1PORT_GPIO_LEVEL_MIN && value < P1PORT_GPIO_LEVEL_MAX) {
            analogWrite(pin, lround((PWMRANGE/P1PORT_GPIO_LEVEL_MAX)*value));
        }
        Logger::info("@%s [%s %d]", P1PORT_NS_PREFIX, (IOEventPaths::Status_Level), String(level).c_str());
        sendEvent((const uint8_t*)(IOEventPaths::Sensor_Luminance), &level, Float);
    }
}}
