# This file will be used to generate sine waves. This should
# be programmed on an RP2040, with the communcation being to a
# AD9833

# IMPORTS
import spidev
import RPi.GPIO as GPIO

import time


freq = 20_000_000 #Max SPI Frequency for the AD9833 is 40 MHz

#FROM EXTERNAL SOURCE
master_clk = 1_000_000 # Has a direct relation to the output waveform frequency



def AD9833_write(spi, data_high, data_low):
    """
    Write 2 bytes to the specified register.
    """
    
    bus = 0
    device = 1

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = freq 
    spi.mode = 2

    data_out = bytearray([data_high, data_low])
    spi.xfer(data_out)

    spi.close()

def AD9833_write4(spi, data_l_high, data_l_low, data_h_high, data_h_low):
    """
    Write 4 bytes to the specified register.
    """
    
    bus = 0
    device = 1

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = freq 
    spi.mode = 2

    data_out = bytearray([data_l_high, data_l_low, data_h_high, data_h_low])
    spi.xfer(data_out)

    spi.close()

def AD9833_set_freq(f):
    # should be used with the Master input clock frequency of 10 MHz
    global master_clk

    spi = spidev.SpiDev()
    spi.open(0, 1)  # Bus 0, Device 1
    spi.max_speed_hz = freq

    #AD9833_write(spi, 0b0010_0000, 0b0000_0000)
    
    AD9833_write( spi, 0b0010_0000, 0b1000_0000)  # Control reg, Start reset

    FRQREG = int((f) * (2**28) / (master_clk))  # Page 15
    #53687

    

    # Starting with the lower bytes
    FL = FRQREG & 0x3F_FF
    Llb = FL & 0xFF
    Lhb = ((FL & 0x3F_00) >> 8) + 0b0100_0000

    # Next, the high bytes
    FH = (FRQREG & 0xF_FF_C0_00) >> 14
    Hlb = FH & 0xFF
    Hhb = ((FH & 0x3F_00) >> 8) + 0b0100_0000

    AD9833_write4(spi,Lhb, Llb, Hhb, Hlb)

    # All the bytes being transmitted are correct

    AD9833_write( spi, 0b0010_0000, 0b0000_0000) # Control reg, finish reset and configure ic

def AD9833_Init():
    # Initialize AD9833
    spi = spidev.SpiDev()
    spi.open(0, 1)  # Bus 0, Device 1
    spi.max_speed_hz = freq

    AD9833_write(spi, 0b0001_0000, 0b0000_0000)
    AD9833_write(spi, 0b0100_0000, 0b1111_1111)

def main():
    AD9833_Init()
    # Set frequency to 100 kHz
    AD9833_set_freq(1_000)

    print("Done initializing AD9833")
    while(1):
        pass

if __name__ == "__main__":
    main()