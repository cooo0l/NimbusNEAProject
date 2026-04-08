import tkinter as tk

from ui import GrapherApp


def main() -> None:
    root = tk.Tk()
    GrapherApp(root)
    root.mainloop()

main()
