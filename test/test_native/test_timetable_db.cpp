#include "doctest.h"
#include "modules/timetable_db.h"

using namespace railboard;

static Departure dep(int hhmm) {
    return Departure{hhmm, 0, VehicleType::Regular};
}

TEST_CASE("nextDepartures returns upcoming trains") {
    std::vector<Departure> schedule{dep(500), dep(515), dep(530), dep(545)};
    Time now{2026, 7, 8, 3, 5, 10};  // 05:10

    auto result = nextDepartures(schedule, now, 2);

    REQUIRE(result.size() == 2);
    CHECK(result[0].hhmm == 515);
    CHECK(result[1].hhmm == 530);
}

TEST_CASE("nextDepartures skips already-departed") {
    std::vector<Departure> schedule{dep(500), dep(515), dep(530), dep(545)};
    Time now{2026, 7, 8, 3, 5, 30};  // 05:30 exactly

    auto result = nextDepartures(schedule, now, 2);

    REQUIRE(result.size() == 2);
    CHECK(result[0].hhmm == 530);  // include equal time
    CHECK(result[1].hhmm == 545);
}

TEST_CASE("nextDepartures returns fewer if not enough left") {
    std::vector<Departure> schedule{dep(500), dep(515)};
    Time now{2026, 7, 8, 3, 5, 10};

    auto result = nextDepartures(schedule, now, 5);

    CHECK(result.size() == 1);
    CHECK(result[0].hhmm == 515);
}

TEST_CASE("nextDepartures empty when after last train") {
    std::vector<Departure> schedule{dep(500), dep(515)};
    Time now{2026, 7, 8, 3, 23, 30};

    auto result = nextDepartures(schedule, now, 2);

    CHECK(result.empty());
}

TEST_CASE("nextDepartures empty when schedule empty") {
    std::vector<Departure> schedule{};
    Time now{2026, 7, 8, 3, 12, 0};

    auto result = nextDepartures(schedule, now, 2);

    CHECK(result.empty());
}

TEST_CASE("lastDeparture returns latest train of day") {
    std::vector<Departure> schedule{dep(500), dep(2345), dep(515)};

    auto last = lastDeparture(schedule);

    REQUIRE(last.has_value());
    CHECK(last->hhmm == 2345);
}

TEST_CASE("lastDeparture empty when schedule empty") {
    std::vector<Departure> schedule{};

    CHECK(!lastDeparture(schedule).has_value());
}

TEST_CASE("nextWakeMinute is minute after earliest upcoming departure") {
    std::vector<Departure> up{dep(510), dep(530)};
    std::vector<Departure> down{dep(515)};
    Time now{2026, 7, 8, 3, 5, 0};  // 05:00

    auto wake = nextWakeMinute(up, down, now);

    REQUIRE(wake.has_value());
    CHECK(*wake == 5 * 60 + 10 + 1);  // 05:11, right after the 05:10 departure
}

TEST_CASE("nextWakeMinute at exact departure minute waits for next minute") {
    std::vector<Departure> up{dep(510)};
    std::vector<Departure> down{};
    Time now{2026, 7, 8, 3, 5, 10};  // during minute 05:10, that train is still shown

    auto wake = nextWakeMinute(up, down, now);

    REQUIRE(wake.has_value());
    CHECK(*wake == 5 * 60 + 11);
}

TEST_CASE("nextWakeMinute uses remaining direction when other ended") {
    std::vector<Departure> up{};
    std::vector<Departure> down{dep(2330)};
    Time now{2026, 7, 8, 3, 23, 0};

    auto wake = nextWakeMinute(up, down, now);

    REQUIRE(wake.has_value());
    CHECK(*wake == 23 * 60 + 30 + 1);
}

TEST_CASE("nextWakeMinute empty after last train in both directions") {
    std::vector<Departure> up{dep(510)};
    std::vector<Departure> down{dep(520)};
    Time now{2026, 7, 8, 3, 23, 50};

    CHECK(!nextWakeMinute(up, down, now).has_value());
}

TEST_CASE("nextWakeMinute tolerates unsorted schedule") {
    std::vector<Departure> up{dep(700), dep(650)};
    std::vector<Departure> down{};
    Time now{2026, 7, 8, 3, 6, 0};

    auto wake = nextWakeMinute(up, down, now);

    REQUIRE(wake.has_value());
    CHECK(*wake == 6 * 60 + 50 + 1);
}

TEST_CASE("nextWakeMinute may return 1440 for last train at 23:59") {
    std::vector<Departure> up{dep(2359)};
    std::vector<Departure> down{};
    Time now{2026, 7, 8, 3, 23, 59};

    auto wake = nextWakeMinute(up, down, now);

    REQUIRE(wake.has_value());
    CHECK(*wake == 1440);  // caller takes modulo -> next day 00:00
}
