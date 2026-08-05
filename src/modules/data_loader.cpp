#include "data_loader.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace railboard {

bool DataLoader::begin() {
    return LittleFS.begin();
}

static std::vector<Departure> parseArr(JsonArrayConst arr) {
    std::vector<Departure> out;
    out.reserve(arr.size());
    for (JsonVariantConst v : arr) {
        JsonArrayConst tup = v.as<JsonArrayConst>();
        out.push_back(Departure{
            static_cast<int>(tup[0].as<int>()),
            static_cast<int>(tup[1].as<int>()),
            static_cast<VehicleType>(tup[2].as<int>())
        });
    }
    return out;
}

std::optional<StationsMeta> DataLoader::loadMeta() {
    File f = LittleFS.open("/stations.json", "r");
    if (!f) return std::nullopt;

    JsonDocument doc;
    if (deserializeJson(doc, f)) { f.close(); return std::nullopt; }
    f.close();

    StationsMeta m;
    for (auto v : doc["destinations"].as<JsonArrayConst>())
        m.destinations.push_back(v.as<std::string>());
    for (const auto& id : m.destinations)
        m.dest_labels.push_back(doc["dest_labels"][id].as<std::string>());

    for (auto v : doc["vehicle_types"].as<JsonArrayConst>()) {
        JsonVariantConst badge =
            doc["vehicle_labels"][v.as<const char*>()]["badge"];
        m.vehicle_badges.push_back(badge.isNull() ? "" : badge.as<std::string>());
    }
    if (m.vehicle_badges.empty()) m.vehicle_badges = {"", "1", "2", "3"};

    for (auto v : doc["holidays"].as<JsonArrayConst>())
        m.holidays.push_back(v.as<std::string>());
    m.holiday_range = { doc["holiday_range"][0].as<std::string>(),
                        doc["holiday_range"][1].as<std::string>() };
    for (auto v : doc["nav_order"].as<JsonArrayConst>())
        m.nav_order.push_back(v.as<std::string>());
    m.default_station = doc["default_station"].as<std::string>();
    return m;
}

std::optional<StationMeta> DataLoader::loadStationMeta(const std::string& id) {
    File f = LittleFS.open("/stations.json", "r");
    if (!f) return std::nullopt;
    JsonDocument doc;
    if (deserializeJson(doc, f)) { f.close(); return std::nullopt; }
    f.close();

    JsonObjectConst s = doc["stations"][id];
    if (s.isNull()) return std::nullopt;

    StationMeta out;
    out.name_ja = s["name_ja"].as<std::string>();
    out.line = s["line"].as<std::string>();
    out.display_direction = s["display_direction"].isNull()
        ? "" : s["display_direction"].as<std::string>();
    for (auto dv : s["directions"].as<JsonArrayConst>()) {
        out.directions.push_back(Direction{
            dv["id"].as<std::string>(),
            dv["label"].as<std::string>()
        });
    }
    return out;
}

std::optional<WifiConfig> DataLoader::loadWifiConfig() {
    File f = LittleFS.open("/wifi.json", "r");
    if (!f) return std::nullopt;
    JsonDocument doc;
    if (deserializeJson(doc, f)) { f.close(); return std::nullopt; }
    f.close();

    WifiConfig w;
    w.ssid = doc["ssid"].isNull() ? "" : doc["ssid"].as<std::string>();
    w.password = doc["password"].isNull() ? "" : doc["password"].as<std::string>();
    if (w.ssid.empty()) return std::nullopt;
    return w;
}

std::optional<StationSchedule> DataLoader::loadStationSchedule(const std::string& id) {
    std::string path = "/tt/" + id + ".json";
    File f = LittleFS.open(path.c_str(), "r");
    if (!f) return std::nullopt;
    JsonDocument doc;
    if (deserializeJson(doc, f)) { f.close(); return std::nullopt; }
    f.close();

    StationSchedule s;
    s.up_wk   = parseArr(doc["up"]["wk"]);
    s.up_sa   = parseArr(doc["up"]["sa"]);
    s.up_ho   = parseArr(doc["up"]["ho"]);
    s.down_wk = parseArr(doc["down"]["wk"]);
    s.down_sa = parseArr(doc["down"]["sa"]);
    s.down_ho = parseArr(doc["down"]["ho"]);
    return s;
}

}  // namespace railboard
