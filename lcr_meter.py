import MCP3202
import AD9833

import csv
from datetime import datetime

# Function to create a new CSV file with today's date and time
def create_csv_file():
    # Get the current date and time
    now = datetime.now()
    filename = now.strftime("data_%Y-%m-%d_%H-%M-%S.csv")
    
    # Create a new CSV file
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write the header row (optional)
        writer.writerow(['Timestamp','Value1', 'Value2'])
    
    return filename

# Function to append values to the CSV file
def append_to_csv(filename, t, value1, value2):
    with open(filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        # Append a new row with the given values
        writer.writerow([t, value1, value2])


def main():
    start_time = datetime.now()

    AD9833.AD9833_Init()
    AD9833.AD9833_set_freq(1_000)

    # Create a new CSV file
    csv_filename = create_csv_file()

    data = []

    for i in range(10000):
        current_time = (datetime.now() - start_time).total_seconds() * 1e3
        data.append([current_time, MCP3202.ADC_Channel0(), MCP3202.ADC_Channel1()])

    for i in range(len(data)):
        append_to_csv(csv_filename, data[i][0], data[i][1], data[i][2])
    print(f"Data has been written to {csv_filename}")

if __name__ == "__main__":
    main()
