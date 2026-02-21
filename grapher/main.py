# For handling CSV files
import csv
# For formatting dates
from datetime import datetime
# For plotting a graph
import matplotlib.pyplot as plt

# Two lists for storing the results of the CSV file to plot
timestamps = []
co2_values = []

# Open the csv file using the file path
with open(r"C:\Users\kazbe\Documents\PlatformIO\Projects\NimbusNEAProject\grapher\testData.csv", newline="") as file:
    # Extract data from the CSV
    reader = csv.DictReader(file)

    # Iterate through each header, if it's CO2 append
    # the CO2 data and the formatted timestamp to lists
    for row in reader:
        if row["sensor"] == "CO2":
            timestamps.append(datetime.strptime(row["timestamp"], "%Y-%m-%d %H:%M:%S"))
            co2_values.append(float(row["value"]))
# Labeling and plotting

plt.plot(timestamps, co2_values)
plt.xlabel("Time")
plt.ylabel("CO2 (ppm)")
plt.title("CO2 Levels Over Time")
plt.show()
