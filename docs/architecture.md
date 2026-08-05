# Architecture

## Scope

The project is a data-driven departure board for the M5Stack CoreInk. Firmware
logic is independent of any railway operator. Station metadata and timetables
are local deployment inputs stored in LittleFS.

The repository boundary is deliberate:

- Tracked: firmware, fictional JSON examples, validator, tests, and
  documentation.
- Ignored: user-authored railway data, Wi-Fi credentials, caches, and build
  outputs.

## Runtime flow

```text
LittleFS JSON
    |
    v
DataLoader -> StationNav -> App -> UiRenderer -> CoreInk display
                  |          |
                  |          +-> RtcClock / WifiSyncer
                  |
                  +-> Persist -> ESP32 NVS

StationSchedule -> timetable_db -> next departures / next wake time
StationsMeta    -> daytype      -> weekday / Saturday / holiday selection
```

At startup, `App` mounts LittleFS, loads `stations.json`, restores local
preferences, and loads the selected station's timetable. Invalid or missing
runtime data produces an on-device error message.

The app selects one of the six schedules using the current day:

1. A date listed in `holidays` uses `ho`.
2. A date inside the optional `holiday_range` uses `ho`.
3. Sunday uses `ho`.
4. Saturday uses `sa`.
5. Every other day uses `wk`.

The screen is redrawn when the visible departures change. The device then
sleeps until one minute after the next departure, so a departure remains
visible throughout its scheduled minute. After the final departure, the app
performs its daily time-sync and display-cleaning work, then sleeps until the
configured day-rollover minute.

## Components

| Component | Responsibility |
| --- | --- |
| `App` | Coordinates startup, rendering, input, synchronization, persistence, and sleep |
| `DataLoader` | Reads metadata, timetable, and local Wi-Fi JSON from LittleFS |
| `daytype` | Selects weekday, Saturday, or holiday service |
| `timetable_db` | Finds visible departures, final departures, and wake times |
| `StationNav` | Navigates the configured station order |
| `UiRenderer` | Draws the two-direction departure board using M5GFX Japanese fonts |
| `Persist` | Stores station, rotation, sync, and maintenance state in NVS |
| `WifiSyncer` | Performs bounded Wi-Fi and NTP synchronization |
| `PowerManager` | Schedules RTC wake-up and enters shutdown |

Pure timetable, calendar, and navigation logic is compiled in the native test
environment. Hardware-specific behavior is compiled in the `coreink`
environment.

## Data model

The firmware expects these runtime files:

| Path | Purpose |
| --- | --- |
| `/stations.json` | Destinations, badges, calendar overrides, station order, and display metadata |
| `/tt/<station-id>.json` | Six ordered schedules for one station |
| `/wifi.json` | Optional local SSID and password |

The service type stored in each departure is an unsigned 8-bit index. Index
`0` is regular service and has no badge. Other indexes resolve through the
ordered `vehicle_types` array and `vehicle_labels`.

Direction IDs are a layout contract:

- `down` appears in the upper half with a downward arrow.
- `up` appears in the lower half with an upward arrow.
- A station with both directions lists them in `up`, `down` order.
- A terminal may define only one direction; the other schedule stays empty.
- A terminal may set `display_direction` to place that schedule in the
  opposite screen half without moving its timetable arrays.

## Persistent state

The NVS namespace `railboard` stores:

- Selected station ID.
- Display rotation.
- Last successful time-sync date.
- Last daily maintenance date.

Wi-Fi credentials and timetable data remain in LittleFS and are not compiled
into the firmware image.

## Constraints

- The display is 200 x 200 pixels. Long labels are reduced through the linked
  24, 16, and 12 pixel Japanese fonts; very long labels can still overflow.
- The calendar supports one weekday, Saturday, and holiday schedule plus one
  continuous holiday date range. Operator rules based on individual weekdays
  or exceptions require explicit entries in `holidays` or a model change.
- The firmware trusts data only after parsing. The desktop validator performs
  stricter referential, range, direction, and sorting checks before upload.
- The partition table favors application flash because the Japanese fonts use
  most of the available firmware space.
