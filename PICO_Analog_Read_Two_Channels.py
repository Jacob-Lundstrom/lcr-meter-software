from machine import ADC
from time import sleep_ms, ticks_ms, ticks_us
import uarray


a2d0 = machine.ADC(0)
a2d1 = machine.ADC(1)


k = 3.284 / (65535)

start_time = (ticks_us() / 1000000.0)
current_time = (ticks_us() / 1000000.0)

record_time = 20000.0
sample_rate = 10.0

timestamps = uarray.array('f')
shunt_voltage = uarray.array('f')
load_voltage = uarray.array('f')

row = 0
while(current_time < start_time + record_time / 1000000.0):
    Vs = 0
    Vl = 0
    samples = 1
    for i in range(samples):
        a0 = a2d0.read_u16()
        a1 = a2d1.read_u16()
        
        Vs += k*a0
        Vl += k*a1
        
    Vs /= samples
    Vl /= samples
    Vscal = (Vs - 0.0039) / (1 + 0.0065)
    Vlcal = (Vl - 0.0039) / (1 + 0.0065)
    current_time = (ticks_us() / 1000000.0)
    timestamps.append(current_time - start_time)
    shunt_voltage.append(Vscal)
    load_voltage.append(Vlcal)
    row = row + 1
    #sleep_ms(int(sample_rate))

for i in range(row):
    print(f"{timestamps[i]}, {shunt_voltage[i]}, {load_voltage[i]}")