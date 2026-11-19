#pragma once
#include "data/Module.h"

namespace PanelNotifications {
    constexpr const char* Feature = "HomeGeniePanel.Notifications";
    bool available();
    const char* updateDevice(bool registering, const String& token, const String& model);
    void restore(Data::Module* module);
    bool saveOption(Data::Module* module, const String& value);
    void notify(const String& message);
    void event(const String& domain, const String& address, const String& property, const String& value);
    void loop();
}
