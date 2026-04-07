from pathlib import Path
# tkinter is responsible for handling GUI
from tkinter import Tk, filedialog
import matplotlib.pyplot as plt
# Importing the appropriate classes and functions
from loader import LoaderError, loadData
from analysis import getValidPercentages, summariseAllMetrics
from plotter import plotMetrics
from validate import validateRecords

# This opens a file dialog for the user
def choose_file() -> Path | None:
    root = Tk()
    root.withdraw()

    file_path = filedialog.askopenfilename(
        title="Select a sensor export file",
        filetypes=[
            ("Sensor exports", "*.csv *.json *.txt"),
            ("All files", "*.*"),
        ],
    )

    root.destroy()

    if not file_path:
        return None

    return Path(file_path)


def main() -> None:
    selected_file = choose_file()

    if selected_file is None:
        print("No file was selected.")
        return

    try:
        records = loadData(selected_file)
    except FileNotFoundError:
        print(f"File not found: {selected_file}")
        return
    except LoaderError as error:
        print(f"Import failed: {error}")
        return

    print(f"Loaded {len(records)} record(s) from {selected_file.name}.")

    validation_issues = validateRecords(records)
    if validation_issues:
        print("Validation found the following issue(s):")
        for issue in validation_issues:
            print(f"- {issue}")
    else:
        print("Validation passed.")

    percentages = getValidPercentages(records)
    print(f"DHT valid: {percentages['dht_valid_percentage']:.1f}%")
    print(f"CCS valid: {percentages['ccs_valid_percentage']:.1f}%")
    print("Metric summaries:")

    for summary in summariseAllMetrics(records):
        print(f"- {summary.label}")
        print(
            f"  count={summary.count}, min={summary.minimum}, max={summary.maximum}, "
            f"mean={summary.mean:.2f}, median={summary.median:.2f}, range={summary.range:.2f}"
        )

    try:
        plotMetrics(records, ["co2_ppm", "tvoc_ppb","temp_c","humidity_pct"], time_unit="minutes")
        plt.show()
    except ValueError as error:
        print(f"Plotting skipped: {error}")


if __name__ == "__main__":
    main()
