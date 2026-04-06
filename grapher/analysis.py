from dataclasses import dataclass

from loader import SensorRecord

# Dictionary linking names of fields/keys
# to a more humanly readable format
METRIC_NAMES = {
    "temp_c": "Temperature (C)",
    "humidity_pct": "Humidity (%)",
    "co2_ppm": "CO2 (ppm)",
    "tvoc_ppb": "TVOC (ppb)",
}

# Stores all the information for one metric
@dataclass
class MetricSummary:
    metric: str
    label: str
    count: int
    minimum: float
    maximum: float
    mean: float
    median: float
    range: float

# Gets all the values for a specific metric and
# appends them to a list
def getMetricValues(records: list[SensorRecord], metric: str) -> list[float]:
    if metric not in METRIC_NAMES:
        raise ValueError(f"Unsupported metric: {metric}")

    values: list[float] = []

    for record in records:
        value = getattr(record, metric)
        if value is not None:
            values.append(float(value))

    return values

# Calculates all the statistics for a metric
def summariseMetric(records: list[SensorRecord], metric: str) -> MetricSummary | None:
    values = getMetricValues(records, metric)

    if not values:
        return None
    
    sorted_values=sorted(values)
    middle_index= len(sorted_values) // 2

    if len(sorted_values) % 2 == 0:
        median = (sorted_values[middle_index-1] + sorted_values[middle_index]) / 2
    else:
        median = sorted_values[middle_index]

    return MetricSummary(
        metric=metric,
        label=METRIC_NAMES[metric],
        count=len(values),
        minimum=min(values),
        maximum=max(values),
        mean=sum(values) / len(values),
        median=median,
        range=max(values)-min(values)
    )

# Calulates the all summaries for all metrics
def summariseAllMetrics(records: list[SensorRecord]) -> list[MetricSummary]:
    summaries: list[MetricSummary] = []

    for metric in METRIC_NAMES:
        summary = summariseMetric(records, metric)
        if summary is not None:
            summaries.append(summary)

    return summaries
# Calculates the proportion of records that are usable data
def getValidPercentages(records: list[SensorRecord]) -> dict[str, float]:
    if not records:
        return {
            "total_records": 0,
            "dht_valid_percentage": 0.0,
            "ccs_valid_percentage": 0.0,
        }

    dht_valid_count = sum(1 for record in records if record.dht_valid)
    ccs_valid_count = sum(1 for record in records if record.ccs_valid)
    total_records = len(records)

    return {
        "total_records": total_records,
        "dht_valid_percentage": (dht_valid_count / total_records) * 100,
        "ccs_valid_percentage": (ccs_valid_count / total_records) * 100,
    }
