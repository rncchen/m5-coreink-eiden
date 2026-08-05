#pragma once
#include <memory>
#include <optional>
#include <string>
#include "modules/button_input.h"
#include "modules/data_loader.h"
#include "modules/persist.h"
#include "modules/power_manager.h"
#include "modules/rtc_clock.h"
#include "modules/station_nav.h"
#include "modules/ui_renderer.h"

namespace railboard {

class App {
public:
    bool begin();
    // Alarm wake: redraw then sleep. Manual wake: sleep after the interactive
    // window times out. Never returns either way.
    [[noreturn]] void run(bool alarm_wake);

private:
    bool loadStation();
    RenderContext buildContext(const Time& now);
    void renderNow();
    void renderIfChanged(const Time& now);
    bool runWifiSync(bool quiet);
    void switchStation(int delta);
    [[noreturn]] void finishAndSleep();
    [[noreturn]] void interactiveLoop();
    [[noreturn]] void needsSyncLoop();

    DataLoader loader_;
    RtcClock rtc_;
    UiRenderer ui_;
    ButtonInput input_;
    PowerManager power_;
    Persist persist_;
    PersistState saved_;
    StationsMeta meta_;
    std::unique_ptr<StationNav> nav_;
    StationMeta current_station_meta_;
    StationSchedule current_schedule_;
    std::optional<WifiConfig> wifi_;
    // Signature of what this boot has drawn; not kept across sleep
    // (every wake redraws at least once).
    std::string last_render_sig_;
    bool flipped_ = false;
    bool rtc_needs_sync_ = false;
};

}  // namespace railboard
