# Data authoring

## Responsibility

Create runtime data only from sources you are allowed to use. Public access
alone does not grant permission to scrape, copy, transform, publish, or
redistribute a timetable. Check source terms, copyright and database rights,
API conditions, attribution duties, and local law.

Keep evidence of the source and applicable license outside this repository when
the data cannot be redistributed. The repository ignores runtime railway data
by default.

## Prepare working files

Copy the fictional examples:

```powershell
Copy-Item data/stations.example.json data/stations.json
Copy-Item data/tt/sample-central.example.json data/tt/sample-central.json
Copy-Item data/tt/sample-north.example.json data/tt/sample-north.json
```

Add, rename, or remove timetable files as station IDs change. A station ID uses
lowercase ASCII letters, digits, and single hyphens, for example
`sample-central`.

## Station metadata

`data/stations.json` contains:

| Field | Contract |
| --- | --- |
| `version` | Maintainer-defined data version |
| `generated_at` | Maintainer-defined generation or review date |
| `destinations` | Ordered unique destination IDs |
| `dest_labels` | Display label for every destination ID |
| `vehicle_types` | Ordered unique type IDs; index 0 must be `regular` |
| `vehicle_labels` | Badge metadata for each non-regular type |
| `holidays` | Individual `YYYY-MM-DD` dates that use `ho` |
| `holiday_range` | Two inclusive dates, or `["", ""]` when unused |
| `nav_order` | Every station ID exactly once, in button-navigation order |
| `default_station` | Station selected when no saved choice exists |
| `stations` | Display metadata keyed by station ID |

Each non-regular vehicle type needs a one-character `badge`. The optional
`ja` value is descriptive and is not rendered by the current firmware.

Each station contains:

```json
{
  "name_ja": "サンプル中央",
  "line": "sample-line",
  "directions": [
    {"id": "up", "label": "サンプル南方面"},
    {"id": "down", "label": "サンプル北方面"}
  ]
}
```

A two-direction station must list `up` first and `down` second. A terminal
may contain only one entry whose ID is either `up` or `down`.
`display_direction` may optionally be set to `up` or `down` when a
terminal's schedule storage direction must appear in the opposite screen half.

## Timetable files

Create one `data/tt/<station-id>.json` file for every station in
`nav_order`.

```json
{
  "station": "sample-central",
  "up": {
    "wk": [[730, 0, 0], [800, 0, 1]],
    "sa": [[800, 0, 0]],
    "ho": [[800, 0, 0]]
  },
  "down": {
    "wk": [[745, 1, 0], [815, 1, 1]],
    "sa": [[815, 1, 0]],
    "ho": [[815, 1, 0]]
  }
}
```

Day keys are:

| Key | Meaning |
| --- | --- |
| `wk` | Weekday |
| `sa` | Saturday |
| `ho` | Sunday and configured holidays |

A departure row is `[HHMM, destination_index, vehicle_type_index]`:

- `HHMM` is an integer from `0` through `2359` with a valid minute.
- `destination_index` points into `destinations`.
- `vehicle_type_index` points into `vehicle_types`.
- Rows must be strictly ascending by time.

For a terminal with no `down` direction, all three `down` arrays must be
empty. Apply the same rule to an absent `up` direction.

## Validate

Run the validator after each data change:

```powershell
python scripts/validate_data.py
```

It checks JSON syntax, required relationships, date formats, direction layout,
departure ordering, indexes, and missing station files. It validates structure,
not factual timetable accuracy.

To verify the tracked fictional examples:

```powershell
python scripts/validate_data.py --example
```

## Review before upload

1. Compare every departure and destination against the permitted source.
2. Confirm the service calendar, special dates, and terminal directions.
3. Record the source, retrieval date, license, and required attribution.
4. Run the validator.
5. Inspect `git status --short` and confirm runtime data remains ignored.
6. Upload LittleFS only after the review is complete.
