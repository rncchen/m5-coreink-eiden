#include "persist.h"
#include "config.h"
#include <Preferences.h>

namespace railboard {

static constexpr const char* kNamespace = RAILBOARD_NVS_NAMESPACE;

PersistState Persist::load() {
    PersistState s;
    Preferences p;
    // On first boot the namespace does not exist yet; if begin fails, every
    // field keeps its default value
    if (p.begin(kNamespace, true)) {
        s.station_id     = p.getString("station", "").c_str();
        s.flipped        = p.getBool("flipped", false);
        s.last_sync_ymd  = p.getInt("sync_ymd", 0);
        s.night_maint_ymd = p.getInt("maint_ymd", 0);
        p.end();
    }
    return s;
}

void Persist::saveStation(const std::string& id) {
    Preferences p;
    if (p.begin(kNamespace, false)) {
        p.putString("station", id.c_str());
        p.end();
    }
}

void Persist::saveFlipped(bool flipped) {
    Preferences p;
    if (p.begin(kNamespace, false)) {
        p.putBool("flipped", flipped);
        p.end();
    }
}

void Persist::saveLastSyncYmd(int ymd) {
    Preferences p;
    if (p.begin(kNamespace, false)) {
        p.putInt("sync_ymd", ymd);
        p.end();
    }
}

void Persist::saveNightMaintYmd(int ymd) {
    Preferences p;
    if (p.begin(kNamespace, false)) {
        p.putInt("maint_ymd", ymd);
        p.end();
    }
}

}  // namespace railboard
