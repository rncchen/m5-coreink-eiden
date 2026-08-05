# Release checklist

Use this checklist before publishing source changes or flashing a deployment.

## Data rights

- [ ] Every non-fictional data source permits the intended copying,
      transformation, device use, and redistribution.
- [ ] Required attribution, notices, source links, and license text are
      recorded where the applicable license requires them.
- [ ] No real timetable or operator data is staged unless redistribution is
      explicitly permitted.
- [ ] Fictional examples are clearly identified as samples.
- [ ] Trademarked names and logos are absent unless their use is authorized.

## Privacy and secrets

- [ ] `data/wifi.json` is untracked and ignored.
- [ ] No SSID, password, token, private URL, personal email, device identifier,
      or location metadata appears in staged files.
- [ ] Photos and generated assets have been checked for visible private
      information and embedded metadata.
- [ ] Build output, caches, editor settings, and temporary files remain
      untracked.

## Data integrity

- [ ] `python scripts/validate_data.py` succeeds for deployment data.
- [ ] Every station has a matching timetable file.
- [ ] Destination and service indexes resolve to the intended labels.
- [ ] Departures are in ascending order and use valid times.
- [ ] `up` and `down` match the intended screen halves.
- [ ] Terminal stations leave the absent direction empty.
- [ ] Holiday dates and `holiday_range` match the permitted source.
- [ ] Long station, direction, and destination labels fit the display.

## Automated verification

- [ ] `python -m pytest scripts/test_validate_data.py` passes.
- [ ] `python scripts/validate_data.py --example` passes.
- [ ] `python -m platformio test -e native` passes.
- [ ] `python -m platformio run -e coreink` passes.
- [ ] Dependency and secret scans have been reviewed.

## Device verification

- [ ] LittleFS and firmware upload successfully.
- [ ] Boot loads the default station without an error screen.
- [ ] Both directions show the expected next departures.
- [ ] A departure remains visible during its scheduled minute.
- [ ] Station navigation wraps in both directions.
- [ ] A terminal uses the correct half of the display.
- [ ] Display rotation persists across sleep.
- [ ] Manual Wi-Fi/NTP synchronization succeeds or fails safely.
- [ ] Sleep, RTC wake, final-departure display, and daily cleaning behave as
      expected.

## Git review

- [ ] `git status --short` contains only intended changes.
- [ ] `git diff --check` reports no whitespace errors.
- [ ] A case-insensitive content scan finds no removed operator, station,
      service, source-site, or project-specific identifiers.
- [ ] The staged diff contains no credentials or personal metadata.
- [ ] The commit author identity is appropriate for a public repository.
- [ ] Any history rewrite or force push has separate explicit approval and
      uses a protected recovery reference or backup.

Hardware behavior cannot be fully established by compilation. Complete the
device section on an actual M5Stack CoreInk before describing a deployment as
verified.
