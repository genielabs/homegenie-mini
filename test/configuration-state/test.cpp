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
    assert(configurationEntryMode() == Mode::Temporary);
    assert(configurationEntryMode(false) == Mode::Temporary);
    assert(configurationEntryMode(true) == Mode::Provisioning);
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

    StatusIndicator offline;
    for (uint32_t t = 0; t < 30000; t += 37)
        assert(!offline.update(t, false, Mode::Normal, false));
    StatusIndicator connected;
    assert(!connected.update(0, true, Mode::Normal, true));
    assert(!connected.update(1950, true, Mode::Normal, true));
    assert(connected.update(1951, true, Mode::Normal, true));
    assert(connected.update(2001, true, Mode::Normal, true));
    assert(!connected.update(2002, true, Mode::Normal, true));
    assert(!connected.update(3952, true, Mode::Normal, true));
    assert(connected.update(3953, true, Mode::Normal, true));
    // Disconnected on the first boot: no grace window and no expiry.
    StatusIndicator lost;
    assert(!lost.update(0, false, Mode::Normal, true));
    assert(!lost.update(100, false, Mode::Normal, true));
    assert(lost.update(101, false, Mode::Normal, true));
    assert(lost.update(201, false, Mode::Normal, true));
    assert(!lost.update(202, false, Mode::Normal, true));
    for (uint32_t t = 303; t < 30000; t += 101)
        assert(lost.update(t, false, Mode::Normal, true) == ((t / 101) % 2 != 0));
    // Compare every sample with the original core loop through loss/reconnection.
    StatusIndicator historical;
    uint32_t oldTs = 0; bool oldOn = false;
    for (uint32_t t = 0; t < 60000; t += 7) {
        bool online = (t < 10000 || t >= 15000) && (t < 35000 || t >= 40000);
        uint32_t age = t - oldTs;
        if (online) {
            if ((!oldOn && age > 1950) || (oldOn && age > 50)) {
                oldOn = !oldOn; oldTs = t;
            }
        } else if (age > 100) { oldOn = !oldOn; oldTs = t; }
        assert(historical.update(t, online, Mode::Normal, true) == oldOn);
    }
    StatusIndicator setup;
    assert(setup.update(12000, false, Mode::Temporary, false));
    assert(setup.update(12499, false, Mode::Temporary, false));
    assert(!setup.update(12500, false, Mode::Temporary, false));
    assert(setup.update(14000, true, Mode::Provisioning, true));
    assert(!setup.update(14100, true, Mode::Provisioning, true));
    assert(setup.update(14200, true, Mode::Provisioning, true));
    assert(!setup.update(14300, true, Mode::Provisioning, true));
    assert(!setup.update(14999, true, Mode::Provisioning, true));
    StatusIndicator rollover;
    const uint32_t base = 0xfffffff0u;
    assert(rollover.update(base, false, Mode::Normal, true));
    assert(!rollover.update(uint32_t(base + 101), false, Mode::Normal, true));
    assert(rollover.update(uint32_t(base + 202), false, Mode::Normal, true));
    puts("PASS: one-shot requests and storage failure, hold/release, startup latch, debounce, rollover, offline/connected/lost Wi-Fi and configuration LED patterns");
}
