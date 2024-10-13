# This file is used to demo the collection of sample points from an MCP3202 2-channel ADC.
# Channel0 will be used for measuring the voltage across the load, and Channel1 will be used
# to sample the (offset) voltage across the current sense resistor.

# This specific ADC has a maximum sample rate of 100 ksps, which uses a 5V supply. Since we need to 
# use a 3.3V supply in order to communicate with it, the supply has to be lowered to 3.3V. This 
# limits the maximum SPI clock frequency to 0.9 MHz. The equation that the datasheet gives to 
# calculate the spi clock frequency is fspi=18*fsample. This then limits me to 50 ksps. Since I need to use
# both channels, this then again limits the effective sample frequency to 25 ksps.

# I can simply use two ADCs for this purpose, however I'm still limited to 50 ksps total. This would not be sufficient to 
# go to higher frequencies, as I want to be able to reach 100khz. By the nyquist limit, I should need 200ksps minimum, 
# however I am not confident that I'll be able to determine phase angles using so few relative samples.

# I will continue with this ADC for the time being, I'll limit the input frequency to 1kHz


import spidev
import time
import gpiozero

def ADC_Channel0() -> float:
    bus = 0
    device = 0

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = 900_000 
    spi.mode = 0

    data_out = [0b11010000, 0x00] # Should indicate that we are using channel 0, with the MSB first.
    data_in = (spi.xfer(data_out))

    spi.close()
    voltage = 3.3 * ((data_in[0] << 8) + data_in[1]) / 2**11
    return voltage

def ADC_Channel1() -> float:
    bus = 0
    device = 0

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = 900_000 
    spi.mode = 0

    data_out = [0b11110000, 0x00] # Should indicate that we are using channel 0, with the MSB first.
    data_in = (spi.xfer(data_out))

    spi.close()
    voltage = 3.3 * ((data_in[0] << 8) + data_in[1]) / 2**11
    return voltage


def main():

    while(True):
        V0 = ADC_Channel0()
        V1 = ADC_Channel1()
        print(f"Read Data. CH0: {V0}, CH1: {V1}")
    

if __name__ == "__main__":
    main()