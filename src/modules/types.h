#pragma once
#include <cstdint>
#include <string>

namespace railboard {

enum class DayType { Weekday, Saturday, Holiday };
enum class VehicleType : uint8_t { Regular = 0 };

struct Time {
    int year;
    int month;
    int day;
    int weekday;  // 0=Sun, 1=Mon, ..., 6=Sat
    int hour;
    int minute;
};

struct Departure {
    int hhmm;                 // 512 = 05:12; 2345 = 23:45
    int dest_index;           // index into destinations in stations.json
    VehicleType vehicle;
};

struct Direction {
    std::string id;           // "up" / "down"
    std::string label;        // user-facing direction label
};

// Convert HHMM (512) to minutes of day (5*60+12 = 312)
inline int hhmmToMinutes(int hhmm) {
    return (hhmm / 100) * 60 + (hhmm % 100);
}

inline int minutesSinceMidnight(int hour, int minute) {
    return hour * 60 + minute;
}

}  // namespace railboard
