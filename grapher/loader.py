# Used to read CSV files
import csv
# Used to read JSON objects
import json
# Provides dataclasses, reduces repeated code
from dataclasses import dataclass
# Makes it easier to work with file paths
from pathlib import Path

# Stores the column names expected in the csv file
CSV_FIELDS = (
    "time_ms",
    "temp_c",
    "humidity_pct",
    "co2_ppm",
    "tvoc_ppb",
    "dht_valid",
    "ccs_valid",
)
# Stores the keys expected in the json export
JSON_FIELDS = (
    "time_ms",
    "temperature",
    "humidity",
    "co2",
    "tvoc",
    "dhtValid",
    "ccsValid",
)

# Raised when an input file cannot be parsed into the expected structure
class LoaderError(ValueError):
    pass
    
#  Generates __init__, __repr__ and __eq__ automatically
@dataclass
# Defines how data will be stored internally after importing
class SensorRecord:
    # Values may be missing so none is possible
    time_ms: int
    temp_c: float | None 
    humidity_pct: float | None
    co2_ppm: int | None
    tvoc_ppb: int | None
    dht_valid: bool
    ccs_valid: bool
    # Converts milleseconds to seconds
    @property
    def timeSeconds(self) -> float:
        return self.time_ms / 1000.0
    # Converts milleseconds to minutes
    @property
    def timeMinutes(self) -> float:
        return self.time_ms / 60000.0
    # Converts a SensorRecord object into a normal dictionary
    def asDict(self) -> dict[str, int | float | bool | None]:
        return {
            "time_ms": self.time_ms,
            "time_seconds": self.timeSeconds,
            "time_minutes": self.timeMinutes,
            "temp_c": self.temp_c,
            "humidity_pct": self.humidity_pct,
            "co2_ppm": self.co2_ppm,
            "tvoc_ppb": self.tvoc_ppb,
            "dht_valid": self.dht_valid,
            "ccs_valid": self.ccs_valid,
        }

# Decides how to load the file given, JSON or CSV
def loadData(path: str | Path) -> list[SensorRecord]:
    file_path = Path(path)
    # Gets the file extension in lower case
    suffix = file_path.suffix.lower()

    if suffix == ".csv":
        return loadCsv(file_path)
    if suffix in [".json", ".txt"]:
        return loadJson(file_path)

    raise LoaderError(f"Unsupported file type '{file_path.suffix}'. Use a CSV, JSON, or TXT export.")

# Loads and parses a CSV file, takes a path as input and outputs
# a list of records in the format required by SensorRecord 
def loadCsv(path: str | Path) -> list[SensorRecord]:
    file_path = Path(path)

    with file_path.open("r", newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)

        if reader.fieldnames is None:
            raise LoaderError("CSV file is empty or does not contain a header row.")

        _requireFields(reader.fieldnames, CSV_FIELDS, "CSV")
        records = [_recordFromCsvRow(row, row_number) for row_number, row in enumerate(reader, start=2)]

    if not records:
        raise LoaderError("CSV file contains a header but no sensor readings.")

    return records

# Loads and parses a JSON file (or txt), takes a path as input and outputs
# a list of records in the format required by SensorRecord 

def loadJson(path: str | Path) -> list[SensorRecord]:
    file_path = Path(path)
    records: list[SensorRecord] = []

    with file_path.open("r", encoding="utf-8") as json_file:
        for line_number, raw_line in enumerate(json_file, start=1):
            line = raw_line.strip()
            if not line:
                continue

            try:
                payload = json.loads(line)
            except json.JSONDecodeError as exc:
                raise LoaderError(f"Invalid JSON on line {line_number}: {exc.msg}.") from exc

            if not isinstance(payload, dict):
                raise LoaderError(f"Expected a JSON object on line {line_number}.")

            _requireFields(payload.keys(), JSON_FIELDS, f"JSON line {line_number}")
            records.append(_recordFromJsonRow(payload, line_number))

    if not records:
        raise LoaderError("JSON file does not contain any sensor readings.")

    return records

# Converts loaded records into dictionaries for easier plotting or display
def toRows(records: list[SensorRecord]) -> list[dict[str, int | float | bool | None]]:
    return [record.asDict() for record in records]

# Converts one csv row dictionary into one SensorRecord
def _recordFromCsvRow(row: dict[str, str | None], row_number: int) -> SensorRecord:
    dht_valid = _parseBool(row.get("dht_valid"), f"CSV row {row_number} dht_valid")
    ccs_valid = _parseBool(row.get("ccs_valid"), f"CSV row {row_number} ccs_valid")

    return SensorRecord(
        time_ms=_parseInt(row.get("time_ms"), f"CSV row {row_number} time_ms"),
        temp_c=_parseOptionalFloat(row.get("temp_c"), f"CSV row {row_number} temp_c"),
        humidity_pct=_parseOptionalFloat(
            row.get("humidity_pct"),
            f"CSV row {row_number} humidity_pct",
        ),
        co2_ppm=_parseOptionalInt(row.get("co2_ppm"), f"CSV row {row_number} co2_ppm"),
        tvoc_ppb=_parseOptionalInt(row.get("tvoc_ppb"), f"CSV row {row_number} tvoc_ppb"),
        dht_valid=dht_valid,
        ccs_valid=ccs_valid,
    )

# Converts one JSON row dictionary into one SensorRecord
def _recordFromJsonRow(row: dict[str, object], line_number: int) -> SensorRecord:
    return SensorRecord(
        time_ms=_parseInt(row.get("time_ms"), f"JSON line {line_number} time_ms"),
        temp_c=_parseOptionalFloat(row.get("temperature"), f"JSON line {line_number} temperature"),
        humidity_pct=_parseOptionalFloat(row.get("humidity"), f"JSON line {line_number} humidity"),
        co2_ppm=_parseOptionalInt(row.get("co2"), f"JSON line {line_number} co2"),
        tvoc_ppb=_parseOptionalInt(row.get("tvoc"), f"JSON line {line_number} tvoc"),
        dht_valid=_parseBool(row.get("dhtValid"), f"JSON line {line_number} dhtValid"),
        ccs_valid=_parseBool(row.get("ccsValid"), f"JSON line {line_number} ccsValid"),
    )

# Checks whether a file has all required collumns/keys
def _requireFields(found_fields, expected_fields: tuple[str, ...], context: str) -> None:
    missing = [field for field in expected_fields if field not in found_fields]
    if missing:
        joined = ", ".join(missing)
        raise LoaderError(f"{context} is missing required field(s): {joined}.")

# Converts boolean style inputs and turns them into python booleans
# i.e false: str returns False: bool

def _parseBool(value: object, label: str) -> bool:
    if isinstance(value, bool):
        return value
    if isinstance(value, int):
        if value in (0, 1):
            return bool(value)
    if isinstance(value, str):
        cleaned = value.strip().lower()
        if cleaned in {"0", "false"}:
            return False
        if cleaned in {"1", "true"}:
            return True

    raise LoaderError(f"{label} must be a boolean or 0/1 value.")

# Converts a required value into an integer

def _parseInt(value: object, label: str) -> int:
    if value is None or value == "":
        raise LoaderError(f"{label} is required.")

    try:
        return int(value)
    except (TypeError, ValueError) as exc:
        raise LoaderError(f"{label} must be an integer.") from exc

# Same as _parseInt() but allows blank values

def _parseOptionalInt(value: object, label: str) -> int | None:
    if value is None or value == "":
        return None

    try:
        return int(value)
    except (TypeError, ValueError) as exc:
        raise LoaderError(f"{label} must be an integer or blank.") from exc

# Same as _parseOptionalInt() but for decimal values

def _parseOptionalFloat(value: object, label: str) -> float | None:
    if value is None or value == "":
        return None

    try:
        return float(value)
    except (TypeError, ValueError) as exc:
        raise LoaderError(f"{label} must be a number or blank.") from exc
