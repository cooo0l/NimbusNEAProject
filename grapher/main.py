import tkinter as tk

from ui import GrapherApp

# Create the root window
def main() -> None:
    root = tk.Tk()
    GrapherApp(root)
    root.mainloop()

# The entire program runs only when main.py is run
if __name__ == "__main__":
    main()
