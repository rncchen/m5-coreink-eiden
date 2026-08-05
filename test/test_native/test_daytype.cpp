#include "doctest.h"
#include "modules/daytype.h"

using namespace railboard;

TEST_CASE("weekday returns Weekday") {
    Time t{2026, 7, 8, 3, 12, 0};  // 2026-07-08 (Wed)
    CHECK(classifyDay(t, {}, {"", ""}) == DayType::Weekday);
}

TEST_CASE("Saturday returns Saturday") {
    Time t{2026, 7, 11, 6, 12, 0};
    CHECK(classifyDay(t, {}, {"", ""}) == DayType::Saturday);
}

TEST_CASE("Sunday returns Holiday") {
    Time t{2026, 7, 12, 0, 12, 0};
    CHECK(classifyDay(t, {}, {"", ""}) == DayType::Holiday);
}

TEST_CASE("national holiday overrides weekday") {
    Time t{2026, 1, 1, 4, 12, 0};   // New Year's Day, Thursday
    CHECK(classifyDay(t, {"2026-01-01"}, {"", ""}) == DayType::Holiday);
}

TEST_CASE("holiday range overrides weekday") {
    Time t{2026, 8, 14, 5, 12, 0};  // Friday inside the configured range
    CHECK(classifyDay(t, {}, {"2026-08-13", "2026-08-16"}) == DayType::Holiday);
}

TEST_CASE("outside holiday range unaffected") {
    Time t{2026, 8, 17, 1, 12, 0};  // Monday outside the configured range
    CHECK(classifyDay(t, {}, {"2026-08-13", "2026-08-16"}) == DayType::Weekday);
}
