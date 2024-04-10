# This file is going to be used in emulation of the sampling that will occur 
# in the final setup, with the inputs recieved from a DAC module. (Yet to be implemented)

import time
from math import pi, sqrt, atan2
import numpy as np
import scipy.optimize as curve_fit
import matplotlib.pyplot as plt
# Simulation imports
from sinesimulation import SimulatedSineWave


# Define settings as a dictionary
simulation_settings = {
    'injection_frequency': 100, # in Hz
    'injection_voltage_min': -1, # Volts
    'injection_voltage_max': 1, # Volts

    'shunt_resistance': 1000, # Ohms
    'load_value_nano': 100, # nF or nH
    'load_is_capacitor': True,
    'load_is_inductor': False
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
load_voltage_phasor.set_noise(0.01)

shunt_voltage_amplitude = injection_voltage_amplitude * (R_shunt / sqrt(X_load**2 + R_shunt**2))
shunt_voltage_phase = 0 + 0 - atan2(X_load, R_shunt) * 180/pi

# Voltage across 1k Shunt resistor
shunt_voltage_phasor = SimulatedSineWave(simulation_settings['injection_frequency'], 
                                  0.1, (injection_dc_offset-load_voltage_offset)/1000, 45,
                                  start_time)
shunt_voltage_phasor.set_noise(0.1)
# print(f"load_voltage: {load_voltage_amplitude}, load_phase: {load_voltage_phase}")
# print(f"shunt_voltage: {shunt_voltage_amplitude}, shunt_phase: {shunt_voltage_phase}")

sampling_rate = 200 * simulation_settings['injection_frequency']
sampling_period = 2 * 1 / simulation_settings['injection_frequency']

cols = 3
total_samples = rows = int(sampling_rate*sampling_period)
data = np.zeros((rows, cols))

for i in range(0, total_samples):
    vload = load_voltage_phasor.get_current_value()
    vshunt = shunt_voltage_phasor.get_current_value()
    t = time.time()-start_time
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

def model(x, A , phi):
    return A * np.sin(2*pi*simulation_settings['injection_frequency'] * x + phi)

initial_guess = (1, 0)
popt, pcov = curve_fit.curve_fit(model, timestamps, load_sample, p0=initial_guess)

A_optimal, phi_optimal = popt
print(f"Fitted: {A_optimal}, {phi_optimal * 180/pi}")
print(f"Actual: {load_voltage_amplitude}, {load_voltage_phase}")

data_fit = model(timestamps, A_optimal, phi_optimal)

plt.figure()
plt.plot(timestamps, load_sample, 'bo', label='Original Data')
plt.plot(timestamps, data_fit, 'r-', label='Fitted Curve')
plt.xlabel('Time')
plt.ylabel('Voltage')
plt.title('Least Squares Fit of Sine Wave')
plt.legend()
plt.show()

# Regardless, at this point we have pretty good guesses for our amplitude and phase.
# From these guesses, we can calculate the estimated load impedance