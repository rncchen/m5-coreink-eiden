#pragma once

#if __has_include("local_config.h")
#include "local_config.h"
#endif

#ifndef RAILBOARD_NVS_NAMESPACE
#define RAILBOARD_NVS_NAMESPACE "railboard"
#endif

// Departures shown per direction
constexpr int DEPARTURES_PER_DIRECTION = 2;

// Time-sync period: during night maintenance, connect to Wi-Fi only when the
// last successful sync is at least this many days old
constexpr int SYNC_INTERVAL_DAYS = 7;

// Interactive window after a manual wake (seconds); on timeout, return to
// scheduled sleep
constexpr int INTERACTIVE_TIMEOUT_S = 60;

// After the last train, sleep until this minute of day to redraw the next
// day's first departures; avoids the 00:00 boundary
constexpr int DAY_ROLLOVER_WAKE_MINUTE = 5;  // 00:05
