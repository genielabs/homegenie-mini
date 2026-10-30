#include <assert.h>
#include <stdio.h>
#include "../../src/ConfigurationState.h"

using namespace DeviceSetup;

struct StoredRequest {
    uint32_t value = 0;
    unsigned writes = 0;
    bool fail = false;
    uint32_t getUInt(const char*, uint32_t) { return value; }
    unsigned putUInt(const char*, uint32_t next) {
        ++writes;
        if (fail) return 0;
        value = next;
        return sizeof(uint32_t);
    }
};

static void press(ServiceButtonHold& button, uint32_t at) {
    assert(!button.update(true, at, 3000));
    assert(!button.update(true, at + 31, 3000));
}
static bool release(ServiceButtonHold& button, uint32_t at) {
    assert(!button.update(false, at, 3000));
    return button.update(false, at + 31, 3000);
}

int main() {
    assert(configurationEntryMode(true) == Mode::Temporary);
    assert(configurationEntryMode(false) == Mode::Provisioning);
    assert(configurationEntryMode(true, true) == Mode::Provisioning);
    assert(configurationEntryMode(false, true) == Mode::Provisioning);
    StoredRequest store;
    assert(consumeBootRequest(store, "request") == Mode::Normal && store.writes == 0);
    assert(saveBootRequest(store, "request", Mode::Temporary));
    assert(consumeBootRequest(store, "request") == Mode::Temporary && store.value == 0);
    assert(consumeBootRequest(store, "request") == Mode::Normal);
    assert(saveBootRequest(store, "request", Mode::Provisioning));
    assert(consumeBootRequest(store, "request") == Mode::Provisioning && store.value == 0);
    assert(consumeBootRequest(store, "request") == Mode::Normal);
    store.value = 99;
    assert(consumeBootRequest(store, "request") == Mode::Normal && store.value == 0);
    store.value = uint32_t(Mode::Temporary);
    store.fail = true;
    assert(consumeBootRequest(store, "request") == Mode::Normal);
    assert(!saveBootRequest(store, "request", Mode::Provisioning));
    assert(!saveBootRequest(store, "request", Mode::Normal));
    ServiceButtonHold b;
    assert(!b.update(false, 0, 3000));
    press(b, 100);
    assert(!b.ready(3130, 3000));
    assert(b.ready(3131, 3000));
    // A single arbitrarily long hold never emits a reset/entry event while down.
    assert(!b.update(true, 60000, 3000));
    assert(release(b, 60001));
    assert(!b.update(false, 61000, 3000));
    // A separate second hold is needed for the second-stage action.
    press(b, 62000);
    assert(release(b, 66000));
    press(b, 70000);
    assert(!release(b, 70200));

    ServiceButtonHold bootHeld;
    assert(!bootHeld.update(true, 0, 3000));
    assert(!bootHeld.update(true, 9000, 3000));
    assert(!bootHeld.ready(9000, 3000));
    assert(!release(bootHeld, 9100));
    press(bootHeld, 10000);
    assert(release(bootHeld, 14000));

    ServiceButtonHold bounce;
    bounce.update(false, 0, 3000);
    bounce.update(true, 100, 3000);
    bounce.update(false, 110, 3000);
    assert(!bounce.update(false, 150, 3000));

    ServiceButtonHold wrap;
    wrap.update(false, 0xfffff000u, 3000);
    press(wrap, 0xfffffff0u);
    assert(wrap.ready(uint32_t(0xfffffff0u + 31 + 3000), 3000));
    assert(release(wrap, uint32_t(0xfffffff0u + 31 + 3500)));

    StatusIndicator led;
    for (uint32_t t = 0; t < 10000; t += 37) assert(!led.update(t, false, Mode::Normal));
    assert(!led.update(10000, true, Mode::Normal));
    assert(led.update(11000, false, Mode::Normal));
    assert(!led.update(11100, false, Mode::Normal));
    assert(led.update(11200, false, Mode::Normal));
    assert(!led.update(11300, true, Mode::Normal));
    assert(led.update(12000, false, Mode::Temporary));
    assert(led.update(12499, false, Mode::Temporary));
    assert(!led.update(12500, false, Mode::Temporary));
    assert(led.update(13000, false, Mode::Temporary));
    assert(led.update(14000, false, Mode::Provisioning));
    assert(!led.update(14100, false, Mode::Provisioning));
    assert(led.update(14200, false, Mode::Provisioning));
    assert(!led.update(14300, false, Mode::Provisioning));
    assert(!led.update(14999, false, Mode::Provisioning));
    // Configuration has precedence even if the radio is still connected.
    assert(led.update(15000, true, Mode::Provisioning));
    assert(!led.update(15100, true, Mode::Normal));
    StatusIndicator ledWrap;
    assert(ledWrap.update(0xffffff00u, false, Mode::Temporary));
    assert(!ledWrap.update(uint32_t(0xffffff00u + 500), false, Mode::Temporary));
    puts("PASS: one-shot requests and storage failure, hold/release, startup latch, debounce, rollover, offline/connected/lost Wi-Fi and configuration LED patterns");
}
