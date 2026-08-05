#include "rtc_clock.h"
#include <M5Unified.h>

namespace railboard {

void RtcClock::begin() {
    M5.Rtc.begin();
}

Time RtcClock::now() {
    auto d = M5.Rtc.getDateTime();
    Time t;
    t.year    = d.date.year;
    t.month   = d.date.month;
    t.day     = d.date.date;
    t.weekday = d.date.weekDay;
    t.hour    = d.time.hours;
    t.minute  = d.time.minutes;
    return t;
}

void RtcClock::set(const Time& t) {
    m5::rtc_datetime_t d;
    d.date.year    = t.year;
    d.date.month   = t.month;
    d.date.date    = t.day;
    d.date.weekDay = t.weekday;
    d.time.hours   = t.hour;
    d.time.minutes = t.minute;
    d.time.seconds = 0;
    M5.Rtc.setDateTime(d);
}

bool RtcClock::isSet() {
    return now().year >= 2024;
}

}  // namespace railboard
