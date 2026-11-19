#include "PanelNotifications.h"
#include "PanelRelay.h"
#include "HomeGenie.h"
#include <Preferences.h>

namespace PanelNotifications {
namespace {
    constexpr size_t MaxDevices = 8;
    constexpr size_t MaxEvents = 4;
    struct Device {
        String token;
        String model;
        unsigned generation = 0;
    };
    struct Event {
        String message;
        unsigned recipients[MaxDevices]{};
    };
    Device devices[MaxDevices];
    Event events[MaxEvents];
    size_t head = 0, count = 0;
    bool loaded = false;
#ifdef ESP32
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    struct Lock {
        bool held = mutex && xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE;
        ~Lock() { if (held) xSemaphoreGive(mutex); }
    };
#else
    struct Lock { bool held = true; };
#endif
    String slotKey(size_t index) { return String("device") + index; }

    bool load(Preferences& prefs) {
        if (loaded) return true;
        for (size_t i = 0; i < MaxDevices; ++i) {
            String key = slotKey(i);
            if (!prefs.isKey(key.c_str())) continue;
            String entry = prefs.getString(key.c_str(), "");
            int separator = entry.indexOf('\n');
            if (separator <= 0) return false;
            devices[i].token = entry.substring(0, separator);
            devices[i].model = entry.substring(separator + 1);
            devices[i].generation = 1;
        }
        loaded = true;
        return true;
    }

    String featureKey(Data::Module* module) {
        uint64_t hash = 14695981039346656037ULL;
        String id = module->domain + "/" + module->address;
        for (size_t i = 0; i < id.length(); ++i) hash = (hash ^ uint8_t(id[i])) * 1099511628211ULL;
        char key[16];
        snprintf(key, sizeof(key), "pn%012llx", (unsigned long long)(hash & 0xffffffffffffULL));
        return key;
    }
}
bool available() { return PanelRelay::available(); }

const char* updateDevice(bool registering, const String& token, const String& model) {
    constexpr auto registered = R"({"Status":"OK","Message":"Registered"})";
    constexpr auto removed = R"({"Status":"OK","Message":"Unregistered"})";
    constexpr auto error = R"({"Status":"Error","Message":"Device registry unavailable"})";
    if (token.isEmpty()) return R"({"Status":"Error","Message":"Token missing"})";
    if (token.length() > 512 || token.indexOf('\n') >= 0 || model.length() > 160)
        return R"({"Status":"Error","Message":"Invalid device"})";
    Lock lock;
    Preferences prefs;
    if (!lock.held || !prefs.begin("panel-notify", false) || !load(prefs)) return error;
    size_t empty = MaxDevices;
    for (size_t i = 0; i < MaxDevices; ++i) {
        auto& device = devices[i];
        if (device.token.isEmpty() && empty == MaxDevices) empty = i;
        if (device.token != token) continue;
        if (registering && device.model == model) return registered;
        String key = slotKey(i);
        if (!registering) {
            if (!prefs.remove(key.c_str())) return error;
            device.token = "";
            device.model = "";
            ++device.generation;
            return removed;
        }
        empty = i;
        break;
    }
    if (!registering) return removed;
    if (empty == MaxDevices) return R"({"Status":"Error","Message":"Device registry full"})";
    String entry = token + "\n" + model;
    if (!prefs.putString(slotKey(empty).c_str(), entry)) return error;
    auto& device = devices[empty];
    if (device.token != token) ++device.generation;
    device.token = token;
    device.model = model;
    return registered;
}

bool saveOption(Data::Module* module, const String& value) {
    Preferences prefs;
    if (!prefs.begin("panel-options", false)) return false;
    String key = featureKey(module);
    if (prefs.getString(key.c_str(), "") == value) return true;
    return value.isEmpty() ? prefs.remove(key.c_str()) : prefs.putString(key.c_str(), value) > 0;
}

void restore(Data::Module* module) {
    if (!available() || !module) return;
    Preferences prefs;
    if (!prefs.begin("panel-options", true)) return;
    String value = prefs.getString(featureKey(module).c_str(), "");
    if (!value.isEmpty()) module->setProperty(Feature, value);
}

void event(const String& domain, const String& address, const String& property, const String& value) {
    if (!available()) return;
    bool on = value.toFloat() > 0;
    bool level = property == "Status.Level";
    bool door = property == "Sensor.DoorWindow";
    bool lockEvent = property == "Status.DoorLock";
    bool motion = on && property == "Sensor.MotionDetect";
    bool alarm = on && (property == "Sensor.Alarm" || property == "Sensor.ObjectDetect");
    if (!(level || door || lockEvent || motion || alarm)) return;
    String d = domain, a = address;
    auto module = Service::HomeGenie::getInstance()->getModule(&d, &a);
    auto flag = module ? module->getProperty(Feature) : nullptr;
    if (!flag || !(flag->value == "1" || flag->value.equalsIgnoreCase("On") || flag->value.equalsIgnoreCase("true"))) return;

    String message;
    const String& name = module->name.isEmpty() ? address : module->name;
    if (level) {
        const char* state = module->type.equalsIgnoreCase("Shutter") ? (on ? "OPEN" : "CLOSED") : (on ? "ON" : "OFF");
        message = name + " is now " + state;
    } else if (door) message = name + (on ? " is Open" : " is Closed");
    else if (lockEvent) message = name + (on ? " is Locked" : " is Unlocked");
    else if (motion) message = String("Motion detected in ") + name;
    else message = String("ALARM: ") + name + " detected activity!";
    notify(message);
}

void notify(const String& message) {
    if (!available() || !message.length() || strspn(message.c_str(), " \t\r\n") == message.length()) return;
    Lock guard;
    if (!guard.held) return;
    if (!loaded) {
        Preferences prefs;
        if (!prefs.begin("panel-notify", false) || !load(prefs)) return;
    }
    if (count == MaxEvents) {
        IO::Logger::warn("Panel notification queue full");
        return;
    }
    auto& job = events[(head + count) % MaxEvents];
    bool subscribed = false;
    for (size_t i = 0; i < MaxDevices; ++i) {
        job.recipients[i] = devices[i].token.isEmpty() ? 0 : devices[i].generation;
        subscribed |= job.recipients[i] != 0;
    }
    if (!subscribed) return;
    job.message = message;
    ++count;
}

void loop() {
    if (!available()) return;
    PanelRelay::loop();
    if (PanelRelay::busy()) return;
    String token, message;
    {
        Lock guard;
        if (!guard.held || !count) return;
        auto& job = events[head];
        for (size_t i = 0; i < MaxDevices; ++i) {
            if (!job.recipients[i]) continue;
            bool current = job.recipients[i] == devices[i].generation && !devices[i].token.isEmpty();
            job.recipients[i] = 0;
            if (current) {
                token = devices[i].token;
                message = job.message;
                break;
            }
        }
        if (token.isEmpty()) {
            job.message = "";
            head = (head + 1) % MaxEvents;
            --count;
            return;
        }
    }
    PanelRelay::send(token, message);
}
}
