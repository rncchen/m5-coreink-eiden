#include "doctest.h"
#include "modules/types.h"

using namespace railboard;

TEST_CASE("hhmmToMinutes") {
    CHECK(hhmmToMinutes(0) == 0);
    CHECK(hhmmToMinutes(512) == 312);        // 05:12
    CHECK(hhmmToMinutes(2345) == 1425);      // 23:45
    CHECK(hhmmToMinutes(2359) == 1439);
}

TEST_CASE("minutesSinceMidnight") {
    CHECK(minutesSinceMidnight(0, 0) == 0);
    CHECK(minutesSinceMidnight(5, 12) == 312);
    CHECK(minutesSinceMidnight(23, 59) == 1439);
}
