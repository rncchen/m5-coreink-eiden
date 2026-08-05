#pragma once

namespace railboard {

class PowerManager {
public:
    // Sleep until the next occurrence of hh:mm (BM8563 daily alarm; crossing
    // midnight works naturally).
    // Battery: pull HOLD low to power off the whole board (~2.5 uA); the RTC
    // powers it back on at the alarm. USB: power cannot be cut, so fall back
    // to light sleep and esp_restart on wake. Both paths rerun setup().
    [[noreturn]] void shutdownUntil(int minute_of_day);
};

}  // namespace railboard
