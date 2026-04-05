from pathlib import Path
from tkinter import Tk, filedialog

from loader import LoaderError, loadData


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


if __name__ == "__main__":
    main()
