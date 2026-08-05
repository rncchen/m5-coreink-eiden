#include <M5Unified.h>
#include "app/app.h"

using namespace railboard;

App app;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    // 80 MHz roughly halves awake-time current; it is also Wi-Fi's minimum clock.
    setCpuFrequencyMhz(80);
    // BM8563 alarm IRQ flag set = scheduled wake; otherwise manual wake
    // (power key / button press on USB / first power-on). M5.begin has already
    // latched HOLD high, so clearing the flag here cannot cut power.
    bool alarm_wake = M5.Rtc.getIRQstatus();
    M5.Rtc.clearIRQ();
    if (!app.begin()) {
        for (;;) delay(1000);  // error screen is showing; recovery requires USB
    }
    app.run(alarm_wake);
}
void loop() {}  // never reached
