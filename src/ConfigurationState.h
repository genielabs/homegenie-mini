#pragma once

#include <stdint.h>

namespace DeviceSetup {

// Persisted values remain compatible with existing one-shot boot markers.
enum class Mode : uint8_t { Normal = 0, Temporary = 1, Provisioning = 2 };

inline Mode configurationEntryMode(bool hasCredentials, bool fullConfiguration = false) {
    return fullConfiguration || !hasCredentials ? Mode::Provisioning : Mode::Temporary;
}

template<class Storage>
Mode consumeBootRequest(Storage& storage, const char* key) {
    uint32_t requested = storage.getUInt(key, 0);
    if (!requested) return Mode::Normal;
    bool consumed = storage.putUInt(key, 0) == sizeof(uint32_t);
    if (consumed && (requested == uint32_t(Mode::Temporary) || requested == uint32_t(Mode::Provisioning))) {
        return static_cast<Mode>(requested);
    }
    return Mode::Normal;
}

template<class Storage>
bool saveBootRequest(Storage& storage, const char* key, Mode mode) {
    if (mode != Mode::Temporary && mode != Mode::Provisioning) return false;
    return storage.putUInt(key, uint32_t(mode)) == sizeof(uint32_t);
}

// Polling/debounce only; no ISR, task or shared heap is introduced.
// A hold already in progress at startup is ignored until its first release.
class ServiceButtonHold {
public:
    bool update(bool pressed, uint32_t now, uint32_t holdMs) {
        if (!initialized) {
            initialized = true;
            candidate = stable = pressed;
            changedAt = pressedAt = now;
            armed = !pressed;
            return false;
        }
        if (candidate != pressed) {
            candidate = pressed;
            changedAt = now;
        }
        if (candidate == stable || uint32_t(now - changedAt) < 30) return false;
        stable = candidate;
        if (stable) {
            pressedAt = now;
            return false;
        }
        bool fire = armed && uint32_t(now - pressedAt) >= holdMs;
        armed = true;
        return fire;
    }

    bool ready(uint32_t now, uint32_t holdMs) const {
        return armed && stable && candidate && uint32_t(now - pressedAt) >= holdMs;
    }

private:
    bool initialized = false, candidate = false, stable = false, armed = false;
    uint32_t changedAt = 0, pressedAt = 0;
};

class StatusIndicator {
public:
    bool update(uint32_t now, bool connected, Mode configuration) {
        if (connected) everConnected = true;
        uint8_t next = configuration == Mode::Provisioning ? 3
            : configuration == Mode::Temporary ? 2
            : everConnected && !connected ? 1 : 0;
        if (next != pattern) {
            pattern = next;
            changedAt = now;
        }
        uint32_t elapsed = uint32_t(now - changedAt);
        if (pattern == 3) {
            // Two 100 ms flashes, then a 700 ms pause: full provisioning (WPS + Bluetooth where supported).
            uint32_t phase = elapsed % 1000;
            return phase < 100 || (phase >= 200 && phase < 300);
        }
        if (pattern == 2) return elapsed % 1000 < 500;
        // Preserve the existing 100 ms on / 100 ms off connection-loss signal.
        if (pattern == 1) return elapsed % 200 < 100;
        return false;
    }

private:
    bool everConnected = false;
    uint8_t pattern = 0;
    uint32_t changedAt = 0;
};

} // namespace DeviceSetup
