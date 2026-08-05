#include "timetable_db.h"

namespace railboard {

std::vector<Departure> nextDepartures(const std::vector<Departure>& day_schedule,
                                      const Time& now,
                                      int count) {
    const int now_min = minutesSinceMidnight(now.hour, now.minute);
    std::vector<Departure> out;
    out.reserve(count);
    for (const auto& d : day_schedule) {
        if (hhmmToMinutes(d.hhmm) >= now_min) {
            out.push_back(d);
            if (static_cast<int>(out.size()) >= count) break;
        }
    }
    return out;
}

std::optional<int> nextWakeMinute(const std::vector<Departure>& up_schedule,
                                  const std::vector<Departure>& down_schedule,
                                  const Time& now) {
    const int now_min = minutesSinceMidnight(now.hour, now.minute);
    std::optional<int> next;
    auto consider = [&](const std::vector<Departure>& sched) {
        for (const auto& d : sched) {
            int m = hhmmToMinutes(d.hhmm);
            if (m >= now_min && (!next || m + 1 < *next)) next = m + 1;
        }
    };
    consider(up_schedule);
    consider(down_schedule);
    return next;
}

std::optional<Departure> lastDeparture(const std::vector<Departure>& day_schedule) {
    std::optional<Departure> last;
    for (const auto& d : day_schedule) {
        if (!last || d.hhmm > last->hhmm) last = d;
    }
    return last;
}

}  // namespace railboard
