from pathlib import Path
import tkinter as tk
from tkinter import filedialog, ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

from analysis import METRIC_NAMES, getValidPercentages, summariseAllMetrics
from loader import LoaderError, SensorRecord, loadData
from plotter import TIME_UNITS, plotMetric
from validate import validateRecords

# Represents the whole application window and its logic
class GrapherApp:
    # Constructor, runs when the app starts
    def __init__(self, root: tk.Tk):
        # Configures and stores the main window
        self.root = root
        self.root.title("Nimbus Grapher")
        self.root.geometry("1100x700")
        # Stores the currently loaded sensor data
        self.records: list[SensorRecord] = []
        self.selected_file: Path | None = None
        # Stores the current graph
        self.current_canvas: FigureCanvasTkAgg | None = None

        self.metric_var = tk.StringVar(value="co2_ppm")
        self.time_unit_var = tk.StringVar(value="minutes")
        self.smoothing_var = tk.IntVar(value=1)
        self.status_var = tk.StringVar(value="Choose a file to begin.")
        self.file_var = tk.StringVar(value="No file selected")
        # Method call to build the interface
        self._buildLayout()
    # Builds the whole interface
    def _buildLayout(self) -> None:
        self.root.columnconfigure(1, weight=1) 
        self.root.rowconfigure(0, weight=1)

        controls = ttk.Frame(self.root, padding=12)
        controls.grid(row=0, column=0, sticky="ns")
        # Creates the area where the graph is
        graph_area = ttk.Frame(self.root, padding=12)
        graph_area.grid(row=0, column=1, sticky="nsew")
        graph_area.columnconfigure(0, weight=1)
        graph_area.rowconfigure(1, weight=1)
        # Creates an import file button
        ttk.Button(controls, text="Import File", command=self.importFile).grid(
            row=0, column=0, sticky="ew", pady=(0, 8)
        )
        #  Shows the selected file name
        ttk.Label(controls, textvariable=self.file_var, wraplength=260).grid(
            row=1, column=0, sticky="w", pady=(0, 12)
        )
        # Creates a dropdown metric selector
        ttk.Label(controls, text="Metric").grid(row=2, column=0, sticky="w")
        ttk.Combobox(
            controls,
            textvariable=self.metric_var,
            values=list(METRIC_NAMES.keys()),
            state="readonly",
        ).grid(row=3, column=0, sticky="ew", pady=(0, 10))
        #  Creates another dropdown selector but for time
        ttk.Label(controls, text="Time unit").grid(row=4, column=0, sticky="w")
        ttk.Combobox(
            controls,
            textvariable=self.time_unit_var,
            values=list(TIME_UNITS.keys()),
            state="readonly",
        ).grid(row=5, column=0, sticky="ew", pady=(0, 10))
        # Creates a numeric control from 1 to 10 for smoothing
        ttk.Label(controls, text="Smoothing window").grid(row=6, column=0, sticky="w")
        ttk.Spinbox(
            controls,
            from_=1,
            to=10,
            textvariable=self.smoothing_var,
            width=8,
        ).grid(row=7, column=0, sticky="w", pady=(0, 12))
        # Button for updating the graph
        ttk.Button(controls, text="Update Plot", command=self.updatePlot).grid(
            row=8, column=0, sticky="ew", pady=(0, 12)
        )
        # Shows status messages
        ttk.Label(controls, text="Status").grid(row=9, column=0, sticky="w")
        ttk.Label(
            controls,
            textvariable=self.status_var,
            wraplength=260,
            justify="left",
        ).grid(row=10, column=0, sticky="w", pady=(0, 12))
        # Labels the summary section and creates an area for validation
        # results and summaries
        ttk.Label(controls, text="Summaries").grid(row=11, column=0, sticky="w")
        self.summary_text = tk.Text(controls, width=36, height=20, wrap="word")
        self.summary_text.grid(row=12, column=0, sticky="nsew")
        # Labels and creates a frame around the graph area
        ttk.Label(graph_area, text="Graph").grid(row=0, column=0, sticky="w")
        self.canvas_frame = ttk.Frame(graph_area)
        self.canvas_frame.grid(row=1, column=0, sticky="nsew")
    # Runs when the user clicks Import File
    def importFile(self) -> None:
        file_path = filedialog.askopenfilename(
            title="Select a sensor export file",
            filetypes=[
                ("Sensor exports", "*.csv *.json *.txt"),
                ("All files", "*.*"),
            ],
        )

        if not file_path:
            self.status_var.set("No file was selected.")
            return

        self.selected_file = Path(file_path)
        self.file_var.set(self.selected_file.name)

        try:
            self.records = loadData(self.selected_file)
        except FileNotFoundError:
            self.records = []
            self.status_var.set(f"File not found: {self.selected_file}")
            return
        except LoaderError as error:
            self.records = []
            self.status_var.set(f"Import failed: {error}")
            self._setSummaryText("")
            return

        self._updateSummaryPanel()
        self.updatePlot()
    # Runs when user presses update graph
    def updatePlot(self) -> None:
        if not self.records:
            self.status_var.set("Import a file before plotting.")
            return

        metric = self.metric_var.get()
        time_unit = self.time_unit_var.get()
        smoothing_window = self.smoothing_var.get()

        try:
            figure, _axis = plotMetric(
                self.records,
                metric,
                time_unit=time_unit,
                smoothing_window=smoothing_window,
            )
        except ValueError as error:
            self.status_var.set(f"Plotting failed: {error}")
            return

        if self.current_canvas is not None:
            self.current_canvas.get_tk_widget().destroy()

        self.current_canvas = FigureCanvasTkAgg(figure, master=self.canvas_frame)
        self.current_canvas.draw()
        self.current_canvas.get_tk_widget().pack(fill="both", expand=True)

        label = METRIC_NAMES[metric]
        self.status_var.set(f"Showing {label} using {time_unit}.")
    # Upon import, the summary panel section is updated using this
    def _updateSummaryPanel(self) -> None:
        if not self.records:
            self._setSummaryText("")
            return

        validation_issues = validateRecords(self.records)
        percentages = getValidPercentages(self.records)

        lines = [
            f"Records loaded: {len(self.records)}",
            f"DHT valid: {percentages['dht_valid_percentage']:.1f}%",
            f"CCS valid: {percentages['ccs_valid_percentage']:.1f}%",
            "",
        ]

        if validation_issues:
            lines.append("Validation issues:")
            for issue in validation_issues:
                lines.append(f"- {issue}")
            lines.append("")
        else:
            lines.append("Validation passed.")
            lines.append("")

        for summary in summariseAllMetrics(self.records):
            lines.append(summary.label)
            lines.append(f"Count: {summary.count}")
            lines.append(f"Min: {summary.minimum}")
            lines.append(f"Max: {summary.maximum}")
            lines.append(f"Mean: {summary.mean:.2f}")
            lines.append(f"Median: {summary.median:.2f}")
            lines.append(f"Range: {summary.range:.2f}")
            lines.append("")

        self._setSummaryText("\n".join(lines).strip())
    # Replaces the contents of the summary text box
    def _setSummaryText(self, text: str) -> None:
        self.summary_text.delete("1.0", tk.END)
        self.summary_text.insert("1.0", text)
