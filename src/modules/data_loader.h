#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "types.h"

namespace railboard {

struct StationsMeta {
    std::vector<std::string> destinations;       // index -> id
    std::vector<std::string> dest_labels;        // parallel to destinations
    std::vector<std::string> vehicle_badges;     // index -> configurable badge; index 0 is ""
    std::vector<std::string> holidays;
    std::pair<std::string, std::string> holiday_range;
    std::vector<std::string> nav_order;
    std::string default_station;
};

struct StationMeta {
    std::string name_ja;
    std::string line;
    std::string display_direction;
    std::vector<Direction> directions;
};

struct StationSchedule {
    std::vector<Departure> up_wk, up_sa, up_ho;
    std::vector<Departure> down_wk, down_sa, down_ho;
};

// Wi-Fi credentials live in /wifi.json on LittleFS (flashed via uploadfs),
// so the firmware binary and the repo stay free of secrets.
struct WifiConfig {
    std::string ssid;
    std::string password;
};

class DataLoader {
public:
    bool begin();
    std::optional<StationsMeta> loadMeta();
    std::optional<StationMeta> loadStationMeta(const std::string& id);
    std::optional<StationSchedule> loadStationSchedule(const std::string& id);
    // nullopt when /wifi.json is missing, unparsable, or has an empty ssid
    std::optional<WifiConfig> loadWifiConfig();
};

}  // namespace railboard
