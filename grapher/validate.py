from loader import SensorRecord

# Takes records and finds any logical issues
def validateRecords(records: list[SensorRecord]) -> list[str]:
    # List to store all found issues
    issues: list[str] = []
    # Checks empty records
    if not records:
        issues.append("No sensor records were loaded.")
        return issues
    # Loops through every record keeping track of the index
    for index, record in enumerate(records, start=1):
        # Checks if time is negative
        if record.time_ms < 0:
            issues.append(f"Record {index}: time_ms cannot be negative.")
        # Checks if the time is moving backwards
        if index > 1 and record.time_ms < records[index - 2].time_ms:
            issues.append(
                f"Record {index}: time_ms moved backwards from the previous reading."
            )
        # Checks if all data exists when it's supposed to
        if record.dht_valid:
            if record.temp_c is None:
                issues.append(f"Record {index}: dht_valid is true but temp_c is missing.")
            if record.humidity_pct is None:
                issues.append(
                    f"Record {index}: dht_valid is true but humidity_pct is missing."
                )
        else:
            if record.temp_c is not None or record.humidity_pct is not None:
                issues.append(
                    f"Record {index}: DHT values are present even though dht_valid is false."
                )

        if record.ccs_valid:
            if record.co2_ppm is None:
                issues.append(f"Record {index}: ccs_valid is true but co2_ppm is missing.")
            if record.tvoc_ppb is None:
                issues.append(
                    f"Record {index}: ccs_valid is true but tvoc_ppb is missing."
                )
        else:
            if record.co2_ppm is not None or record.tvoc_ppb is not None:
                issues.append(
                    f"Record {index}: CCS values are present even though ccs_valid is false."
                )

    return issues
