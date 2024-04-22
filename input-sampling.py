# This file is going to be used in emulation of the sampling that will occur 
# in the final setup, with the inputs recieved from a DAC module. (Yet to be implemented)

import time
from math import pi, sqrt, atan2
import numpy as np
import scipy.optimize as curve_fit
import matplotlib.pyplot as plt
# Simulation imports
from sinesimulation import SimulatedSineWave

def simulate_sample(show_detail: bool=False) -> list:
    # Define settings as a dictionary
    simulation_settings = {
        'injection_frequency': 10, # in Hz
        'injection_voltage_min': -10, # Volts
        'injection_voltage_max': 10, # Volts

        'shunt_resistance': 1000, # Ohms
        'load_value_nano': 50, # nF or nH
        'load_is_capacitor': True,
        'load_is_inductor': False,

        'sampling_multiplier': 200, # How many samples per period will be recorded
        'simulated_load_noise_percentage': 0.01,
        'simulated_shunt_noise_percentage': 0.01,
    }

    # Create time-varying sine waves for simulation

    # Injection voltage, put across the shunt resistor and the load in series.
    injection_voltage_amplitude = (simulation_settings['injection_voltage_max']-simulation_settings['injection_voltage_min'])/2
    injection_dc_offset = (simulation_settings['injection_voltage_max']+simulation_settings['injection_voltage_min'])/2
    start_time = time.time()

    injection_voltage = SimulatedSineWave(simulation_settings['injection_frequency'], 
                                    injection_voltage_amplitude, injection_dc_offset, 0,
                                    start_time)

    # Example for testing: 1k shunt resistor, with a 100 nF Capacitor Load
    # V_load = V_inj * Z_load / (Z_load + Z_shunt)

    omega = 2*pi*simulation_settings['injection_frequency']
    R_shunt = simulation_settings['shunt_resistance']

    if simulation_settings['load_is_capacitor']:
        X_load = - 1/(omega * simulation_settings['load_value_nano'] * 1e-9)
        load_voltage_offset = injection_dc_offset
    elif simulation_settings['load_is_inductor']:
        X_load = (omega * simulation_settings['load_value_nano'] * 1e-9)
        load_voltage_offset = 0

    load_voltage_amplitude = injection_voltage_amplitude * abs(X_load) / sqrt(X_load**2 + R_shunt**2)
    load_voltage_phase = 0 + 90 * X_load / abs(X_load) - atan2(X_load, R_shunt)*180/(pi)

    # Voltage across load
    load_voltage_phasor = SimulatedSineWave(simulation_settings['injection_frequency'], 
                                    load_voltage_amplitude, load_voltage_offset, load_voltage_phase,
                                    start_time)
    load_voltage_phasor.set_noise(simulation_settings['simulated_load_noise_percentage'])

    shunt_voltage_amplitude = injection_voltage_amplitude * (R_shunt / sqrt(X_load**2 + R_shunt**2))
    shunt_voltage_phase = 0 + 0 - atan2(X_load, R_shunt) * 180/pi

    # Voltage across 1k Shunt resistor
    shunt_voltage_phasor = SimulatedSineWave(simulation_settings['injection_frequency'], 
                                    shunt_voltage_amplitude, (injection_dc_offset-load_voltage_offset)/1000, shunt_voltage_phase,
                                    start_time)
    shunt_voltage_phasor.set_noise(simulation_settings['simulated_shunt_noise_percentage'])
    # print(f"load_voltage: {load_voltage_amplitude}, load_phase: {load_voltage_phase}")
    # print(f"shunt_voltage: {shunt_voltage_amplitude}, shunt_phase: {shunt_voltage_phase}")

    sampling_rate = simulation_settings['sampling_multiplier'] * simulation_settings['injection_frequency']
    sampling_period = 2 * 1 / simulation_settings['injection_frequency']
    # sampling_period = 1

    cols = 3 # One for time, one for load voltage, one for shunt voltage
    total_samples = rows = int(sampling_rate*sampling_period)
    data = np.zeros((rows, cols))

    for i in range(0, total_samples):
        vload = load_voltage_phasor.get_current_value()
        vshunt = shunt_voltage_phasor.get_current_value()
        t = time.time()-start_time
        if show_detail:
            print(f"{t}, {vload}, {vshunt}")
        data[i, 0] = t
        data[i, 1] = vload
        data[i, 2] = vshunt
        # The minimum sleep time seems to be about 1ms, which means with a 
        time.sleep(1 / sampling_rate)

    # At this point, the whole sample should be collected. 
    # we now need to compute the least-squares for the 
    # phase angle, then furthermore the amplitude of an
    # ideal sine wave. Doing so should yield a very
    # good approximation of the real sine wave.
    timestamps = data[:, 0]
    start = timestamps[0]  # Start value will be the first value of the column
    stop = timestamps[-1]  # Stop value will be the last value of the column
    num = len(timestamps)  # Number of samples will be the length of the column
    result = np.linspace(start, stop, num)
    # I have no clue why this works but it does

    load_sample = data[:, 1]
    start = load_sample[0]  # Start value will be the first value of the column
    stop = load_sample[-1]  # Stop value will be the last value of the column
    num = len(load_sample)  # Number of samples will be the length of the column
    result = np.linspace(start, stop, num)

    shunt_sample = data[:, 2]
    start = shunt_sample[0]  # Start value will be the first value of the column
    stop = shunt_sample[-1]  # Stop value will be the last value of the column
    num = len(shunt_sample)  # Number of samples will be the length of the column
    result = np.linspace(start, stop, num)

    def model(x, A , phi):
        return A * np.sin(2*pi*simulation_settings['injection_frequency'] * x + phi)

    initial_guess = (1, 0)
    popt, pcov = curve_fit.curve_fit(model, timestamps, load_sample, p0=initial_guess)

    A_load_optimal, phi_load_optimal = popt
    if show_detail:
        print("Load Voltage:")
        print(f"Fitted: {A_load_optimal}, {phi_load_optimal * 180/pi}")
        print(f"Actual: {load_voltage_amplitude}, {load_voltage_phase}")

    initial_guess = (1, 0)
    popt, pcov = curve_fit.curve_fit(model, timestamps, shunt_sample, p0=initial_guess)

    A_shunt_optimal, phi_shunt_optimal = popt
    if show_detail:
        print("Shunt Voltage:")
        print(f"Fitted: {A_shunt_optimal}, {phi_shunt_optimal * 180/pi}")
        print(f"Actual: {shunt_voltage_amplitude}, {shunt_voltage_phase}")

        load_fit = model(timestamps, A_load_optimal, phi_load_optimal)
        shunt_fit = model(timestamps, A_shunt_optimal, phi_shunt_optimal)
        plt.figure()
        plt.plot(timestamps, load_sample, 'o', label='Original Data (Load)')
        plt.plot(timestamps, load_fit, '-', label='Fitted Curve (Load)')
        plt.plot(timestamps, shunt_sample, 'o', label='Original Data (Shunt)')
        plt.plot(timestamps, shunt_fit, '-', label='Fitted Curve (Shunt)')
        plt.xlabel('Time')
        plt.ylabel('Voltage')
        plt.title('Least Squares Fit of Sine Wave')
        plt.legend()
        plt.show()

    # Regardless, at this point we have pretty good guesses for our amplitude and phase.
    # From these guesses, we can calculate the estimated load impedance

    V_load = A_load_optimal
    ang_V_Load = phi_load_optimal
    I_load = A_shunt_optimal / simulation_settings['shunt_resistance']
    ang_I_Load = phi_shunt_optimal

    # Z = V / I
    Z_load_mag = V_load / I_load
    Z_load_ang = ang_V_Load - ang_I_Load

    
    # print(f"{Z_load_mag}, {Z_load_ang}")
    real_load = Z_load_mag * np.cos(Z_load_ang)
    imag_load = Z_load_mag * np.sin(Z_load_ang)
    if show_detail:
        print(f"{real_load}, j({imag_load})")
        print(f"Actual load Reactance: {X_load}")

    if imag_load < 0:
        # The load is assumed to be capacitive
        # X = -1/(omega * C)
        load_val_nano_guess = (- 1 / (omega * imag_load)) * 1e9

    if show_detail:
        print(f'Estimated capacitance: {load_val_nano_guess}')
    return ((real_load, load_val_nano_guess), simulation_settings['load_is_capacitor'])


if __name__ == "__main__":
    total = 0
    sample_count = 10

    for i in range(0, sample_count):
        total += simulate_sample(False)[0][1]

    total /= sample_count

    print(f'Estimated after multiple samples: {total} nF')
    ideal_capacitance = 50
    print(f"Error: {100 * (total-ideal_capacitance)/ideal_capacitance}%")