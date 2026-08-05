#pragma once
#include <string>

namespace railboard {

// A battery-mode wake is a full power-off reboot: neither RAM nor RTC slow
// memory survives, so any state that must cross sleep goes into NVS.
// Write only on change to preserve flash endurance.
struct PersistState {
    std::string station_id;   // empty string = never saved
    bool flipped = false;
    int last_sync_ymd = 0;    // last successful time sync (YYYYMMDD)
    int night_maint_ymd = 0;  // last night maintenance (YYYYMMDD)
};

class Persist {
public:
    PersistState load();
    void saveStation(const std::string& id);
    void saveFlipped(bool flipped);
    void saveLastSyncYmd(int ymd);
    void saveNightMaintYmd(int ymd);
};

}  // namespace railboard
