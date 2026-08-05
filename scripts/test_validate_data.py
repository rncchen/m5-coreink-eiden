import copy
import json
from pathlib import Path

from scripts.validate_data import validate_dataset, validate_meta, validate_timetable


ROOT = Path(__file__).resolve().parent.parent
EXAMPLE_META = ROOT / "data" / "stations.example.json"


def _meta():
    return json.loads(EXAMPLE_META.read_text(encoding="utf-8"))


def test_tracked_examples_are_valid():
    errors = validate_dataset(EXAMPLE_META, ROOT / "data" / "tt", ".example.json")
    assert errors == []


def test_meta_rejects_missing_destination_label():
    meta = _meta()
    del meta["dest_labels"]["sample-north"]
    errors = validate_meta(meta, Path("stations.json"))
    assert any("dest_labels.sample-north" in error for error in errors)


def test_meta_reports_invalid_direction_without_crashing():
    meta = _meta()
    meta["stations"]["sample-north"]["directions"] = [None]
    errors = validate_meta(meta, Path("stations.json"))
    assert any("directions[0] must be an object" in error for error in errors)
    assert any("directions[0].id must be up or down" in error for error in errors)


def test_meta_accepts_terminal_display_direction_override():
    meta = _meta()
    meta["stations"]["sample-north"]["display_direction"] = "down"
    assert validate_meta(meta, Path("stations.json")) == []


def test_timetable_rejects_invalid_indices_and_order():
    meta = _meta()
    data = json.loads(
        (ROOT / "data" / "tt" / "sample-central.example.json").read_text(
            encoding="utf-8"
        )
    )
    data = copy.deepcopy(data)
    data["up"]["wk"] = [[800, 99, 0], [730, 0, 99]]
    errors = validate_timetable(
        data,
        Path("sample-central.json"),
        "sample-central",
        {"up", "down"},
        len(meta["destinations"]),
        len(meta["vehicle_types"]),
    )
    assert any("ascending time order" in error for error in errors)
    assert any("destination index is out of range" in error for error in errors)
    assert any("vehicle type index is out of range" in error for error in errors)


def test_terminal_rejects_departures_in_missing_direction():
    meta = _meta()
    data = json.loads(
        (ROOT / "data" / "tt" / "sample-north.example.json").read_text(
            encoding="utf-8"
        )
    )
    data["down"]["wk"] = [[900, 0, 0]]
    errors = validate_timetable(
        data,
        Path("sample-north.json"),
        "sample-north",
        {"up"},
        len(meta["destinations"]),
        len(meta["vehicle_types"]),
    )
    assert any("station has no down direction" in error for error in errors)
