#include "power_manager.h"
#include <M5Unified.h>
#include <driver/gpio.h>

namespace railboard {

void PowerManager::shutdownUntil(int minute_of_day) {
    // On USB power, timerSleep falls back to light sleep internally (power
    // cannot be cut), so first allow all four buttons (stick up/middle/down +
    // TOP) to wake — any key wakes the device while plugged in. On battery the
    // whole board powers off right after, so these settings simply become moot
    // (only the side power key can wake).
    for (gpio_num_t pin : {GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_5}) {
        gpio_wakeup_enable(pin, gpio_int_type_t::GPIO_INTR_LOW_LEVEL);
    }
    m5::rtc_time_t t;
    t.hours   = static_cast<std::int8_t>(minute_of_day / 60);
    t.minutes = static_cast<std::int8_t>(minute_of_day % 60);
    t.seconds = 0;
    M5.Power.timerSleep(t);
    for (;;) { delay(1000); }  // never reached; power-off or esp_restart already happened
}

}  // namespace railboard
