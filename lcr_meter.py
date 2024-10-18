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
def append_to_csv(filename, t1, value1, t2, value2):
    with open(filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        # Append a new row with the given values
        writer.writerow([t1, value1, t2, value2])

def measure_impedance(points: int, test_frequency: float, plot: bool):
    start_time = datetime.now()
    omega = 2 * pi * test_frequency
    shunt_resistance = 67.7

    AD9833.AD9833_set_freq(test_frequency)

    data = np.zeros((points, 4))

    for i in range(points):
        T1 = (datetime.now() - start_time).total_seconds()
        V1 = MCP3202.ADC_Channel0()
        T2 = (datetime.now() - start_time).total_seconds()
        V2 = MCP3202.ADC_Channel1()
        data[i] = ([T1, V1, T2, V2])

    T1 = data[:, 0]

    load_sample = data[:, 1]

    T2 = data[:,2]

    shunt_sample = data[:, 3]

    def model_shunt(x, A , phi, y0):
        return A * np.sin(2*pi* test_frequency * x + phi) + y0
        

    def model_load(x, A , phi, y0):
        return A * np.sin(2*pi*test_frequency * x + phi) + y0

    initial_guess = (0.3, 0, 0.3)
    popt, pcov = curve_fit.curve_fit(model_load, T1, load_sample, p0=initial_guess)

    A_load_optimal, phi_load_optimal, y0_load_optimal = popt
    # print("Load Voltage:")
    # print(f"Fitted: {A_load_optimal}, {phi_load_optimal * 180/pi}")

    initial_guess = (1, 0, 1.8)
    popt, pcov = curve_fit.curve_fit(model_shunt, T2, shunt_sample, p0=initial_guess)

    A_shunt_optimal, phi_shunt_optimal, y0_shunt_optimal = popt
    # print("Shunt Voltage:")
    # print(f"Fitted: {A_shunt_optimal}, {phi_shunt_optimal * 180/pi}")

    load_fit = model_load(T1, A_load_optimal, phi_load_optimal, y0_load_optimal)
    shunt_fit = model_shunt(T2, A_shunt_optimal, phi_shunt_optimal, y0_shunt_optimal) 
    if plot:
        plt.figure()
        plt.plot(T1, load_sample, 'o', label='Original Data (Load)')
        plt.plot(T1, load_fit, '-', label='Fitted Curve (Load)')
        plt.plot(T2, shunt_sample, 'o', label='Original Data (Shunt)')
        plt.plot(T2, shunt_fit, '-', label='Fitted Curve (Shunt)')
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
    #print(f"Calculated net impedance: {real_load}, j({imag_load})")

    return([real_load - shunt_resistance,imag_load])

    # if imag_load < 0:
    #     # The load is assumed to be capacitive
    #     # X = -1/(omega * C)
    #     load_val_nano_guess = (- 1 / (omega * imag_load)) * 1e9
    #     print(f'Estimated capacitance: {load_val_nano_guess}')
    # elif imag_load > 0:
    #     # The load impedance is inductive
    #     # X = omega * L
    #     load_val_nano_guess = imag_load * 1e9 / omega
    #     print(f'Estimeated Inductance: {load_val_nano_guess}')

def main():
    samples = 100
    freq = 100          
    avg = [0.0,0.0]
    for i in range(samples):
        z = measure_impedance(points=10000, test_frequency=freq, plot=False)
        avg[0] = avg[0] + z[0]
        avg[1] = avg[1] + z[1]
    
    avg[0] = avg[0]/samples
    avg[1] = avg[1]/samples
    
    print(f"Measured net impedance: {avg[0]}, j({avg[1]})")

    if avg[1] < 0:
        # The load is assumed to be capacitive
        # X = -1/(omega * C)
        load_val_nano_guess = (- 1 / (2*pi*freq * avg[1])) * 1e9
        print(f'Estimated capacitance: {load_val_nano_guess} nF')
        print(f'ESR: {avg[0]} Ohms')

    elif avg[1] > 0:
        # The load impedance is inductive
        # X = omega * L
        load_val_micro_guess = avg[1] * 1e6 / (2*pi*freq)
        print(f'Estimeated Inductance: {load_val_micro_guess} uH')
        print(f'ESR: {avg[0]} Ohms')

'''
    # Here in case I need to log the collected data for debugging
    # Create a new CSV file
    csv_filename = create_csv_file()

    for i in range(len(data)):
        append_to_csv(csv_filename, data[i][0], data[i][1], data[i][2])
    print(f"Data has been written to {csv_filename}")
'''

if __name__ == "__main__":
    main()
