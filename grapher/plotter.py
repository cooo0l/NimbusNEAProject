import matplotlib.pyplot as plt

from analysis import METRIC_NAMES
from loader import SensorRecord

# Defines the supported time units for the x-axis
TIME_UNITS = {
    "ms": ("Time (ms)", 1.0),
    "seconds": ("Time (seconds)", 1000.0),
    "minutes": ("Time (minutes)", 60000.0),
}

# Prepares the raw data needed for plotting one metric
def getTimeSeries(
    records: list[SensorRecord],
    metric: str,
    time_unit: str = "minutes",
    start_ms: int | None = None,
    end_ms: int | None = None,
) -> tuple[list[float], list[float]]:
    if metric not in METRIC_NAMES:
        raise ValueError(f"Unsupported metric: {metric}")

    if time_unit not in TIME_UNITS:
        raise ValueError(f"Unsupported time unit: {time_unit}")

    divisor = TIME_UNITS[time_unit][1]
    times: list[float] = []
    values: list[float] = []

    for record in records:
        if start_ms is not None and record.time_ms < start_ms:
            continue
        if end_ms is not None and record.time_ms > end_ms:
            continue

        value = getattr(record, metric)
        if value is None:
            continue

        times.append(record.time_ms / divisor)
        values.append(float(value))

    return times, values

# Applies simple smoothing reducing noise
def smoothValues(values: list[float], window_size: int) -> list[float]:
    if window_size <= 1:
        return values.copy()

    smoothed: list[float] = []

    for index in range(len(values)):
        start_index = max(0, index - window_size + 1)
        window = values[start_index : index + 1]
        smoothed.append(sum(window) / len(window))

    return smoothed

# Plots the graph for one metric
def plotMetric(
    records: list[SensorRecord],
    metric: str,
    time_unit: str = "minutes",
    start_ms: int | None = None,
    end_ms: int | None = None,
    smoothing_window: int = 1,
):
    times, values = getTimeSeries(records, metric, time_unit, start_ms, end_ms)

    if not times:
        raise ValueError(f"No data points are available for metric '{metric}'.")

    plotted_values = smoothValues(values, smoothing_window)
    time_label = TIME_UNITS[time_unit][0]
    metric_label = METRIC_NAMES[metric]

    figure, axis = plt.subplots()
    axis.plot(times, plotted_values, marker="o")
    axis.set_title(f"{metric_label} Against Time")
    axis.set_xlabel(time_label)
    axis.set_ylabel(metric_label)
    axis.grid(True, alpha=0.3)

    return figure, axis

# Plots a graph for all metrics
def plotMetrics(
    records: list[SensorRecord],
    metrics: list[str],
    time_unit: str = "minutes",
    start_ms: int | None = None,
    end_ms: int | None = None,
):
    if not metrics:
        raise ValueError("At least one metric must be provided.")

    figure, axis = plt.subplots()
    time_label = TIME_UNITS[time_unit][0]

    for metric in metrics:
        times, values = getTimeSeries(records, metric, time_unit, start_ms, end_ms)
        if times:
            axis.plot(times, values, marker="o", label=METRIC_NAMES[metric])

    axis.set_title("Sensor Metrics Against Time")
    axis.set_xlabel(time_label)
    axis.set_ylabel("Sensor Value")
    axis.grid(True, alpha=0.3)
    axis.legend()

    return figure, axis
