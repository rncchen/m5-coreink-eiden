# Railway Departure Board Template for M5Stack CoreInk

[繁體中文](README.zh-TW.md) | [日本語](README.ja.md)

An offline railway departure board template for the M5Stack CoreInk. It shows
the next two departures in each direction, supports configurable service
badges, keeps time with the RTC, and can periodically synchronize through
Wi-Fi and NTP.

This repository intentionally contains no real operator, station, or timetable
data. The tracked JSON files are fictional examples. Each user supplies data
they are legally permitted to use.

## Features

- Data-driven station names, destinations, directions, service badges, and
  weekday/Saturday/holiday timetables.
- Two departures per direction on the 200 x 200 e-paper display.
- Station selection, 180-degree display rotation, and manual time sync.
- Deep sleep between departure changes and a daily display-cleaning cycle.
- Local JSON validation before uploading LittleFS.
- Wi-Fi credentials and user-authored railway data excluded from Git.

## Requirements

- M5Stack CoreInk.
- Python 3.10 or later.
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/index.html).
- A USB connection suitable for flashing the device.

Install the validation dependency:

```powershell
python -m pip install -r scripts/requirements.txt
```

## Quick start

1. Copy the fictional examples to the ignored working files:

   ```powershell
   Copy-Item data/stations.example.json data/stations.json
   Copy-Item data/tt/sample-central.example.json data/tt/sample-central.json
   Copy-Item data/tt/sample-north.example.json data/tt/sample-north.json
   ```

2. Replace the sample station IDs, labels, destinations, dates, and departures
   with your own data. Rename each timetable file to match its station ID.

3. Validate the data:

   ```powershell
   python scripts/validate_data.py
   ```

4. Optionally create a local Wi-Fi configuration:

   ```powershell
   Copy-Item data/wifi.example.json data/wifi.json
   ```

5. Build and upload the filesystem and firmware:

   ```powershell
   python -m platformio run -e coreink
   python -m platformio run -e coreink -t uploadfs
   python -m platformio run -e coreink -t upload
   ```

The tracked examples can be checked without creating local data:

```powershell
python scripts/validate_data.py --example
```

## Controls

| Control | Action |
| --- | --- |
| Stick up | Previous station |
| Stick down | Next station |
| Stick press | Rotate the display 180 degrees |
| Top button | Synchronize time through Wi-Fi and NTP |

The upper half represents `down`; the lower half represents `up`. A terminal
station may define only one of those directions, and the unused half displays
`終着駅`.

## Data format

`data/stations.json` contains shared metadata and station definitions.
`data/tt/<station-id>.json` contains six schedules for each station:
`up`/`down` combined with `wk`/`sa`/`ho`.

Each departure is a compact array:

```json
[815, 1, 0]
```

The values are `[HHMM, destination_index, vehicle_type_index]`. Indexes refer
to the ordered arrays in `stations.json`. See
[Data authoring](docs/data-authoring.md) for the complete contract.

## Data rights and responsibility

Publicly accessible information is not automatically licensed for copying,
automated extraction, modification, or redistribution. Before adding data,
review the operator's terms, database rights, copyright rules, and any API or
open-data license that applies in your jurisdiction.

Keep source records and attribution locally when required. Do not commit data
unless its license permits repository redistribution. The project authors do
not provide railway data or legal clearance.

## Documentation

- [Architecture](docs/architecture.md)
- [Data authoring](docs/data-authoring.md)
- [Release checklist](docs/release-checklist.md)

## Testing

```powershell
python -m pytest scripts/test_validate_data.py
python -m platformio test -e native
python -m platformio run -e coreink
```

## License

The source code and fictional templates are available under the [MIT License](LICENSE).
Vendored dependency notices are listed in
[Third-party notices](THIRD_PARTY_NOTICES.md). Railway data created by users
remains subject to its own source terms and license.
