#pragma once
#include <Arduino.h>

namespace PanelRelay {
    const char* updateKey(const String& body);
    bool available();
    bool busy();
    void send(const String& token, const String& message);
    void loop();
}
