#include "doctest.h"
#include "modules/station_nav.h"

using namespace railboard;

TEST_CASE("starts at default station") {
    StationNav nav({"a", "b", "c"}, "b");
    CHECK(nav.current() == "b");
}

TEST_CASE("next advances forward") {
    StationNav nav({"a", "b", "c"}, "a");
    nav.next();
    CHECK(nav.current() == "b");
}

TEST_CASE("next wraps around at end") {
    StationNav nav({"a", "b", "c"}, "c");
    nav.next();
    CHECK(nav.current() == "a");
}

TEST_CASE("prev wraps around at start") {
    StationNav nav({"a", "b", "c"}, "a");
    nav.prev();
    CHECK(nav.current() == "c");
}

TEST_CASE("jumpTo existing station") {
    StationNav nav({"a", "b", "c"}, "a");
    CHECK(nav.jumpTo("c") == true);
    CHECK(nav.current() == "c");
}

TEST_CASE("jumpTo unknown station returns false") {
    StationNav nav({"a", "b", "c"}, "a");
    CHECK(nav.jumpTo("zzz") == false);
    CHECK(nav.current() == "a");
}
