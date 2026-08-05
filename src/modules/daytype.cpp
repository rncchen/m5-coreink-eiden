#include "daytype.h"
#include <cstdio>

namespace railboard {

static std::string ymd(const Time& t) {
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.year, t.month, t.day);
    return std::string(buf);
}

DayType classifyDay(const Time& t,
                    const std::vector<std::string>& holidays,
                    const std::pair<std::string, std::string>& holiday_range) {
    const std::string today = ymd(t);

    for (const auto& h : holidays) {
        if (h == today) return DayType::Holiday;
    }

    if (!holiday_range.first.empty() &&
        today >= holiday_range.first && today <= holiday_range.second) {
        return DayType::Holiday;
    }

    if (t.weekday == 0) return DayType::Holiday;
    if (t.weekday == 6) return DayType::Saturday;
    return DayType::Weekday;
}

}  // namespace railboard
