# This file is going to be used in emulation of the sampling that will occur 
# in the final setup, with the inputs recieved from a DAC module. (Yet to be implemented)

import time
from math import pi, sqrt, atan2

# Simulation imports
from sinesimulation import SimulatedSineWave


# Define settings as a dictionary
simulation_settings = {
    'injection_frequency': 10, # in Hz
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
load_voltage_phasor.set_noise(0.1)

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
while (time.time()-start_time) < 0.1:
    val = load_voltage_phasor.get_current_value()
    # The minimum sleep time seems to be about 1ms
    time.sleep(1 / sampling_rate)
    print(f"{time.time()-start_time}, {val}, {shunt_voltage_phasor.get_current_value()}")