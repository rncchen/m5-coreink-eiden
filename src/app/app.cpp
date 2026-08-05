#include "app.h"
#include "config.h"
#include "modules/daytype.h"
#include "modules/timetable_db.h"
#include "modules/wifi_syncer.h"
#include <cstdio>
#include <Arduino.h>

namespace railboard {

static int packYmd(const Time& t) {
    return t.year * 10000 + t.month * 100 + t.day;
}

// Civil-day algorithm (Howard Hinnant); day distance between two YYYYMMDD
// values drives the periodic time-sync decision.
static long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468;
}

static long ymdToDays(int ymd) {
    return daysFromCivil(ymd / 10000, (ymd / 100) % 100, ymd % 100);
}

// Screen-content signature: station id plus both directions' departures,
// including the last-train row after service end. An unchanged signature means
// redrawing would produce the identical screen, so the flashy refresh is skipped.
static std::string renderSig(const std::string& station_id,
                             const RenderContext& ctx) {
    std::string s = station_id;
    auto add = [&s](const Departure& d) {
        char b[20];
        std::snprintf(b, sizeof(b), "|%d.%d.%u", d.hhmm, d.dest_index,
                      static_cast<unsigned>(d.vehicle));
        s += b;
    };
    for (const auto& d : ctx.up_next) add(d);
    s += "/";
    for (const auto& d : ctx.down_next) add(d);
    s += "/";
    if (ctx.up_next.empty() && ctx.up_last) add(*ctx.up_last);
    if (ctx.down_next.empty() && ctx.down_last) add(*ctx.down_last);
    return s;
}

static const std::vector<Departure>& scheduleFor(const StationSchedule& s,
                                                 int dir_idx, DayType dt) {
    if (dir_idx == 0) {
        if (dt == DayType::Weekday)  return s.up_wk;
        if (dt == DayType::Saturday) return s.up_sa;
        return s.up_ho;
    } else {
        if (dt == DayType::Weekday)  return s.down_wk;
        if (dt == DayType::Saturday) return s.down_sa;
        return s.down_ho;
    }
}

bool App::begin() {
    if (!loader_.begin()) {
        ui_.begin();
        ui_.renderMessage("データ読込失敗", "LittleFS");
        return false;
    }
    ui_.begin();
    rtc_.begin();
    auto m = loader_.loadMeta();
    if (!m) { ui_.renderMessage("stations.json 不正", ""); return false; }
    meta_ = *m;
    wifi_ = loader_.loadWifiConfig();
    saved_ = persist_.load();
    flipped_ = saved_.flipped;
    ui_.setFlipped(flipped_);
    nav_ = std::make_unique<StationNav>(meta_.nav_order, meta_.default_station);
    if (!saved_.station_id.empty()) nav_->jumpTo(saved_.station_id);
    if (!loadStation()) {
        ui_.renderMessage("駅データ不明", nav_->current());
        return false;
    }
    rtc_needs_sync_ = !rtc_.isSet();
    return true;
}

bool App::loadStation() {
    auto sm = loader_.loadStationMeta(nav_->current());
    auto sc = loader_.loadStationSchedule(nav_->current());
    if (!sm || !sc) return false;
    current_station_meta_ = *sm;
    current_schedule_ = *sc;
    return true;
}

void App::run(bool alarm_wake) {
    if (rtc_needs_sync_) needsSyncLoop();
    if (alarm_wake) finishAndSleep();
    renderNow();
    interactiveLoop();
}

RenderContext App::buildContext(const Time& now) {
    DayType dt = classifyDay(now, meta_.holidays, meta_.holiday_range);
    const auto& up   = scheduleFor(current_schedule_, 0, dt);
    const auto& down = scheduleFor(current_schedule_, 1, dt);
    return RenderContext{
        &current_station_meta_, &meta_, now,
        nextDepartures(up, now, DEPARTURES_PER_DIRECTION),
        nextDepartures(down, now, DEPARTURES_PER_DIRECTION),
        lastDeparture(up),
        lastDeparture(down)
    };
}

void App::renderNow() {
    Time now = rtc_.now();
    RenderContext ctx = buildContext(now);
    last_render_sig_ = renderSig(nav_->current(), ctx);
    ui_.renderMain(ctx);
}

void App::renderIfChanged(const Time& now) {
    RenderContext ctx = buildContext(now);
    std::string sig = renderSig(nav_->current(), ctx);
    if (sig != last_render_sig_) {
        last_render_sig_ = sig;
        ui_.renderMain(ctx);
    }
}

void App::switchStation(int delta) {
    if (delta > 0) nav_->next(); else nav_->prev();
    if (!loadStation()) {
        ui_.renderMessage("データなし", nav_->current());
        return;
    }
    saved_.station_id = nav_->current();
    persist_.saveStation(saved_.station_id);
    renderNow();
}

bool App::runWifiSync(bool quiet) {
    if (!wifi_) {
        if (!quiet) {
            ui_.renderMessage("Wi-Fi 設定なし", "wifi.json を確認");
            delay(2000);
        }
        return false;
    }
    if (!quiet) ui_.renderMessage("Wi-Fi 接続中…", wifi_->ssid);
    WifiSyncer syncer;
    auto r = syncer.sync(rtc_, wifi_->ssid, wifi_->password);
    if (r.ok) {
        saved_.last_sync_ymd = packYmd(r.synced);
        persist_.saveLastSyncYmd(saved_.last_sync_ymd);
        if (!quiet) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%02d:%02d 同期完了",
                          r.synced.hour, r.synced.minute);
            ui_.renderMessage("時計同期完了", buf);
            delay(2000);
        }
        return true;
    }
    if (!quiet) {
        ui_.renderMessage("同期失敗", r.error + " TOPで再試行");
        delay(2000);
    }
    return false;
}

void App::finishAndSleep() {
    Time now = rtc_.now();
    DayType dt = classifyDay(now, meta_.holidays, meta_.holiday_range);
    auto wake = nextWakeMinute(scheduleFor(current_schedule_, 0, dt),
                               scheduleFor(current_schedule_, 1, dt), now);
    if (wake) {
        renderIfChanged(now);
        power_.shutdownUntil(*wake % (24 * 60));
    }
    // After the last train: once-per-day night maintenance = periodic time
    // sync + deep clean + redraw of the last-train screen, then sleep until
    // the day-rollover minute to draw the next day's first departures.
    int today = packYmd(now);
    if (saved_.night_maint_ymd != today) {
        bool sync_due = saved_.last_sync_ymd == 0 ||
            ymdToDays(today) - ymdToDays(saved_.last_sync_ymd) >= SYNC_INTERVAL_DAYS;
        if (sync_due) runWifiSync(true);
        ui_.deepClean();
        renderNow();
        saved_.night_maint_ymd = today;
        persist_.saveNightMaintYmd(today);
    } else {
        renderIfChanged(now);
    }
    power_.shutdownUntil(DAY_ROLLOVER_WAKE_MINUTE);
}

void App::interactiveLoop() {
    uint32_t idle_start = millis();
    int last_minute = rtc_.now().minute;
    while (millis() - idle_start < INTERACTIVE_TIMEOUT_S * 1000UL) {
        ButtonEvent ev = input_.poll();
        switch (ev) {
            case ButtonEvent::Up:
                switchStation(-1);
                idle_start = millis();
                break;
            case ButtonEvent::Down:
                switchStation(+1);
                idle_start = millis();
                break;
            case ButtonEvent::Press:
                flipped_ = !flipped_;
                ui_.setFlipped(flipped_);
                persist_.saveFlipped(flipped_);
                renderNow();
                idle_start = millis();
                break;
            case ButtonEvent::Top:
                runWifiSync(false);
                renderNow();
                idle_start = millis();
                break;
            default:
                break;
        }
        Time now = rtc_.now();
        if (now.minute != last_minute) {
            last_minute = now.minute;
            renderIfChanged(now);
        }
        delay(30);
    }
    finishAndSleep();
}

void App::needsSyncLoop() {
    // Without a valid RTC no wake can be scheduled; stay here until the user
    // syncs manually (this state normally happens while on USB power).
    ui_.renderMessage("時計未設定", "TOP キーで校時");
    for (;;) {
        if (input_.poll() == ButtonEvent::Top) {
            if (runWifiSync(false)) {
                rtc_needs_sync_ = false;
                renderNow();
                interactiveLoop();
            }
            ui_.renderMessage("時計未設定", "TOP キーで校時");
        }
        delay(30);
    }
}

}  // namespace railboard
