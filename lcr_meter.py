import MCP3202
import AD9833

import csv
from datetime import datetime
import numpy as np
import scipy.optimize as curve_fit
import matplotlib.pyplot as plt
from math import pi, sqrt, atan2

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
    injection_freq = 10000
    omega = 2 * pi * injection_freq
    shunt_resistance = 99.6
    samples = 1000

    #AD9833.AD9833_Init()
    AD9833.AD9833_set_freq(injection_freq)


    data = np.zeros((samples, 3))

    for i in range(samples):
        current_time = (datetime.now() - start_time).total_seconds()
        data[i] = ([current_time, MCP3202.ADC_Channel0(), MCP3202.ADC_Channel1()])

    timestamps = data[:, 0]
    # I have no clue why this works but it does

    load_sample = data[:, 1]

    shunt_sample = data[:, 2]

    def model_shunt(x, A , phi):
        return A * np.sin(2*pi* injection_freq * x + phi) + np.mean(shunt_sample)
        

    def model_load(x, A , phi):
        return A * np.sin(2*pi*injection_freq * x + phi) + np.mean(load_sample)

    initial_guess = (0.5, 0)
    popt, pcov = curve_fit.curve_fit(model_load, timestamps, load_sample, p0=initial_guess)

    A_load_optimal, phi_load_optimal = popt
    print("Load Voltage:")
    print(f"Fitted: {A_load_optimal}, {phi_load_optimal * 180/pi}")

    initial_guess = (1, 0)
    popt, pcov = curve_fit.curve_fit(model_shunt, timestamps, shunt_sample, p0=initial_guess)

    A_shunt_optimal, phi_shunt_optimal = popt
    print("Shunt Voltage:")
    print(f"Fitted: {A_shunt_optimal}, {phi_shunt_optimal * 180/pi}")

    load_fit = model_load(timestamps, A_load_optimal, phi_load_optimal)
    shunt_fit = model_shunt(timestamps, A_shunt_optimal, phi_shunt_optimal) 
    plt.figure()
    plt.plot(timestamps, load_sample, 'o', label='Original Data (Load)')
    plt.plot(timestamps, load_fit, '-', label='Fitted Curve (Load)')
    plt.plot(timestamps, shunt_sample, 'o', label='Original Data (Shunt)')
    plt.plot(timestamps, shunt_fit, '-', label='Fitted Curve (Shunt)')
    plt.xlabel('Time')
    plt.ylabel('Voltage')
    plt.title('Best Fit of Sine Wave')
    plt.legend()
    plt.show()

    # Regardless, at this point we have pretty good guesses for our amplitude and phase.
    # From these guesses, we can calculate the estimated load impedance

    V_load = A_load_optimal
    ang_V_Load = phi_load_optimal
    I_load = A_shunt_optimal / shunt_resistance
    ang_I_Load = phi_shunt_optimal

    # Z = V / I
    Z_load_mag = V_load / I_load
    Z_load_ang = ang_V_Load - ang_I_Load

    
    # print(f"{Z_load_mag}, {Z_load_ang}")
    real_load = Z_load_mag * np.cos(Z_load_ang)
    imag_load = Z_load_mag * np.sin(Z_load_ang)
    print(f"Calculated net impedance: {real_load}, j({imag_load})")

    if imag_load < 0:
        # The load is assumed to be capacitive
        # X = -1/(omega * C)
        load_val_nano_guess = (- 1 / (omega * imag_load)) * 1e9

    print(f'Estimated capacitance: {load_val_nano_guess}')

'''
    # Create a new CSV file
    csv_filename = create_csv_file()

    for i in range(len(data)):
        append_to_csv(csv_filename, data[i][0], data[i][1], data[i][2])
    print(f"Data has been written to {csv_filename}")
'''

if __name__ == "__main__":
    main()
