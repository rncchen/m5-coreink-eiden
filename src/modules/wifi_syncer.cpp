#include "wifi_syncer.h"
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

namespace railboard {

SyncResult WifiSyncer::sync(RtcClock& rtc, const std::string& ssid,
                            const std::string& password) {
    SyncResult r{false, "", {}};
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 15000) {
            r.error = "Wi-Fi timeout";
            WiFi.disconnect(true, true); WiFi.mode(WIFI_OFF);
            return r;
        }
        delay(200);
    }
    // Force the system clock to zero first. M5.begin() has already used
    // setSystemTimeFromRtc to write the PCF8563's JST value into time() with
    // GMT semantics, so getLocalTime would pass its tm_year check with
    // "PCF8563 + 9 hours" instead of waiting for real SNTP — making every
    // sync jump ahead by 9 hours.
    struct timeval tv0 = {0, 0};
    settimeofday(&tv0, nullptr);
    configTzTime("JST-9", "ntp.nict.jp", "time.google.com");
    struct tm ti;
    start = millis();
    while (!getLocalTime(&ti, 200)) {
        if (millis() - start > 15000) {  // SNTP cold start needs several seconds; 5 s is too tight
            r.error = "NTP timeout";
            WiFi.disconnect(true, true); WiFi.mode(WIFI_OFF);
            return r;
        }
    }
    Time t{ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
           ti.tm_wday, ti.tm_hour, ti.tm_min};
    rtc.set(t);
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    r.ok = true;
    r.synced = t;
    return r;
}

}  // namespace railboard
