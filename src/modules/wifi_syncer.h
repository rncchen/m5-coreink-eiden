#pragma once
#include <string>
#include "rtc_clock.h"
#include "types.h"

namespace railboard {

struct SyncResult {
    bool ok;
    std::string error;
    Time synced;
};

class WifiSyncer {
public:
    SyncResult sync(RtcClock& rtc, const std::string& ssid,
                    const std::string& password);
};

}  // namespace railboard
