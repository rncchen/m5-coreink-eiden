"""Validate user-authored station metadata and timetable JSON files."""

from __future__ import annotations

import argparse
import json
import re
import sys
from datetime import date
from pathlib import Path
from typing import Any

ID_PATTERN = re.compile(r"^[a-z0-9]+(?:-[a-z0-9]+)*$")
DAYS = ("wk", "sa", "ho")
DIRECTIONS = ("up", "down")


def _load_json(path: Path, errors: list[str]) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        errors.append(f"{path}: file not found")
    except json.JSONDecodeError as exc:
        errors.append(f"{path}:{exc.lineno}:{exc.colno}: {exc.msg}")
    return None


def _is_identifier(value: Any) -> bool:
    return isinstance(value, str) and bool(ID_PATTERN.fullmatch(value))


def _valid_date(value: Any) -> bool:
    if not isinstance(value, str):
        return False
    try:
        date.fromisoformat(value)
    except ValueError:
        return False
    return True


def validate_meta(meta: Any, path: Path) -> list[str]:
    errors: list[str] = []
    if not isinstance(meta, dict):
        return [f"{path}: root must be an object"]

    for field in ("version", "generated_at"):
        if not isinstance(meta.get(field), str) or not meta[field].strip():
            errors.append(f"{path}: {field} must be a non-empty string")

    destinations = meta.get("destinations")
    if not isinstance(destinations, list) or not destinations:
        errors.append(f"{path}: destinations must be a non-empty array")
        destinations = []
    elif any(not _is_identifier(value) for value in destinations):
        errors.append(f"{path}: every destination must use lowercase letters, digits, or hyphens")
    elif len(set(destinations)) != len(destinations):
        errors.append(f"{path}: destinations must be unique")

    labels = meta.get("dest_labels")
    if not isinstance(labels, dict):
        errors.append(f"{path}: dest_labels must be an object")
        labels = {}
    for destination in destinations:
        if not isinstance(labels.get(destination), str) or not labels[destination].strip():
            errors.append(f"{path}: dest_labels.{destination} must be a non-empty string")

    vehicle_types = meta.get("vehicle_types")
    if not isinstance(vehicle_types, list) or not vehicle_types:
        errors.append(f"{path}: vehicle_types must be a non-empty array")
        vehicle_types = []
    else:
        if vehicle_types[0] != "regular":
            errors.append(f"{path}: vehicle_types[0] must be regular")
        if len(vehicle_types) > 256:
            errors.append(f"{path}: vehicle_types cannot contain more than 256 entries")
        if any(not _is_identifier(value) for value in vehicle_types):
            errors.append(f"{path}: every vehicle type must be a valid identifier")
        elif len(set(vehicle_types)) != len(vehicle_types):
            errors.append(f"{path}: vehicle_types must be unique")

    vehicle_labels = meta.get("vehicle_labels", {})
    if not isinstance(vehicle_labels, dict):
        errors.append(f"{path}: vehicle_labels must be an object")
        vehicle_labels = {}
    for vehicle_type in vehicle_types[1:]:
        label = vehicle_labels.get(vehicle_type)
        if not isinstance(label, dict):
            errors.append(f"{path}: vehicle_labels.{vehicle_type} must be an object")
            continue
        badge = label.get("badge")
        if not isinstance(badge, str) or len(badge) != 1:
            errors.append(f"{path}: vehicle_labels.{vehicle_type}.badge must be one character")

    holidays = meta.get("holidays")
    if not isinstance(holidays, list) or any(not _valid_date(value) for value in holidays):
        errors.append(f"{path}: holidays must contain only YYYY-MM-DD dates")

    holiday_range = meta.get("holiday_range")
    if not isinstance(holiday_range, list) or len(holiday_range) != 2:
        errors.append(f"{path}: holiday_range must contain two strings")
    elif holiday_range != ["", ""]:
        if not all(_valid_date(value) for value in holiday_range):
            errors.append(f"{path}: holiday_range must be blank or contain two YYYY-MM-DD dates")
        elif holiday_range[0] > holiday_range[1]:
            errors.append(f"{path}: holiday_range start must not be after its end")

    stations = meta.get("stations")
    if not isinstance(stations, dict) or not stations:
        errors.append(f"{path}: stations must be a non-empty object")
        stations = {}
    for station_id, station in stations.items():
        prefix = f"{path}: stations.{station_id}"
        if not _is_identifier(station_id):
            errors.append(f"{prefix}: station id is invalid")
        if not isinstance(station, dict):
            errors.append(f"{prefix} must be an object")
            continue
        if not isinstance(station.get("name_ja"), str) or not station["name_ja"].strip():
            errors.append(f"{prefix}.name_ja must be a non-empty string")
        if not isinstance(station.get("line"), str) or not station["line"].strip():
            errors.append(f"{prefix}.line must be a non-empty string")
        if station.get("display_direction", "") not in ("", "up", "down"):
            errors.append(f"{prefix}.display_direction must be up or down")
        directions = station.get("directions")
        if not isinstance(directions, list) or len(directions) not in (1, 2):
            errors.append(f"{prefix}.directions must contain one or two entries")
            continue
        ids: list[Any] = []
        for index, direction in enumerate(directions):
            if not isinstance(direction, dict):
                errors.append(f"{prefix}.directions[{index}] must be an object")
                continue
            ids.append(direction.get("id"))
            if not isinstance(direction.get("label"), str) or not direction["label"].strip():
                errors.append(f"{prefix}.directions[{index}].label must be a non-empty string")
        expected = ["up", "down"] if len(directions) == 2 else None
        if expected and ids != expected:
            errors.append(f"{prefix}.directions must be ordered as up, down")
        elif not expected and (len(ids) != 1 or ids[0] not in DIRECTIONS):
            errors.append(f"{prefix}.directions[0].id must be up or down")

    nav_order = meta.get("nav_order")
    if not isinstance(nav_order, list):
        errors.append(f"{path}: nav_order must be an array")
    elif any(not _is_identifier(value) for value in nav_order):
        errors.append(f"{path}: every nav_order entry must be a valid station id")
    elif len(nav_order) != len(set(nav_order)):
        errors.append(f"{path}: nav_order must not contain duplicates")
    elif set(nav_order) != set(stations):
        errors.append(f"{path}: nav_order must contain every station exactly once")

    if meta.get("default_station") not in stations:
        errors.append(f"{path}: default_station must reference a station")
    return errors


def _validate_departures(
    rows: Any,
    context: str,
    destination_count: int,
    vehicle_count: int,
) -> list[str]:
    errors: list[str] = []
    if not isinstance(rows, list):
        return [f"{context} must be an array"]
    previous = -1
    for index, row in enumerate(rows):
        item = f"{context}[{index}]"
        if (
            not isinstance(row, list)
            or len(row) != 3
            or any(isinstance(value, bool) or not isinstance(value, int) for value in row)
        ):
            errors.append(f"{item} must be [HHMM, destination_index, vehicle_type_index]")
            continue
        hhmm, destination_index, vehicle_index = row
        hour, minute = divmod(hhmm, 100)
        if not 0 <= hour <= 23 or not 0 <= minute <= 59:
            errors.append(f"{item}: HHMM is invalid")
        minutes = hour * 60 + minute
        if minutes <= previous:
            errors.append(f"{item}: departures must be in ascending time order")
        previous = minutes
        if not 0 <= destination_index < destination_count:
            errors.append(f"{item}: destination index is out of range")
        if not 0 <= vehicle_index < vehicle_count:
            errors.append(f"{item}: vehicle type index is out of range")
    return errors


def validate_timetable(
    data: Any,
    path: Path,
    station_id: str,
    direction_ids: set[str],
    destination_count: int,
    vehicle_count: int,
) -> list[str]:
    errors: list[str] = []
    if not isinstance(data, dict):
        return [f"{path}: root must be an object"]
    if data.get("station") != station_id:
        errors.append(f"{path}: station must match {station_id}")
    for direction in DIRECTIONS:
        schedules = data.get(direction)
        if not isinstance(schedules, dict):
            errors.append(f"{path}: {direction} must be an object")
            continue
        for day in DAYS:
            rows = schedules.get(day)
            context = f"{path}: {direction}.{day}"
            errors.extend(
                _validate_departures(rows, context, destination_count, vehicle_count)
            )
            if direction not in direction_ids and rows:
                errors.append(f"{context} must be empty because the station has no {direction} direction")
    return errors


def validate_dataset(meta_path: Path, timetable_dir: Path, suffix: str = ".json") -> list[str]:
    errors: list[str] = []
    meta = _load_json(meta_path, errors)
    if meta is None:
        return errors
    errors.extend(validate_meta(meta, meta_path))
    if errors:
        return errors

    for station_id in meta["nav_order"]:
        path = timetable_dir / f"{station_id}{suffix}"
        data = _load_json(path, errors)
        if data is None:
            continue
        direction_ids = {
            direction["id"] for direction in meta["stations"][station_id]["directions"]
        }
        errors.extend(
            validate_timetable(
                data,
                path,
                station_id,
                direction_ids,
                len(meta["destinations"]),
                len(meta["vehicle_types"]),
            )
        )
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--meta", type=Path, default=Path("data/stations.json"))
    parser.add_argument("--timetable-dir", type=Path, default=Path("data/tt"))
    parser.add_argument(
        "--example",
        action="store_true",
        help="validate the tracked fictional example files",
    )
    args = parser.parse_args()
    suffix = ".json"
    if args.example:
        args.meta = Path("data/stations.example.json")
        suffix = ".example.json"
    errors = validate_dataset(args.meta, args.timetable_dir, suffix)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1
    print(f"OK: validated {args.meta} and {len(json.loads(args.meta.read_text(encoding='utf-8'))['nav_order'])} timetable files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
