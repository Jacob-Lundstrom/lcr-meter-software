# The funciton of this file is to allow other scripts to generate
# real-time synchronized sine waves for simulation.

# SHOULD NEVER BE USED IN FINAL IMPLEMENTATION

from math import sin, pi
import time
import random

class SimulatedSineWave:
    def __init__(self, frequency: float, amplitude: float, dc_offset: float, phase_deg: float, start_time: float):
        self.frequency = frequency
        self.amplitude = amplitude
        self.dc_offset = dc_offset
        self.start_time = start_time
        self.phase_deg = phase_deg

        self.value = 0.0
        self.running = False
        self.noise = False
        self.noise_amount = 0.0

    def get_current_value(self):
        t = time.time() - self.start_time
        if self.noise:
            noise = self.noise_amount * self.amplitude * (random.random()-0.5)
        self.value = noise + self.dc_offset + self.amplitude * sin(2*pi*self.frequency * t + self.phase_deg * 2*pi / 360)
        return self.value
    
    def set_noise(self, noise_amount:float):
        self.noise = True
        self.noise_amount = noise_amount

    def remove_noise(self):
        self.noise = False
        self.noise_amount = 0
    
