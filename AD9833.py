# This file will be used to generate sine waves. This should
# be programmed on an RP2040, with the communcation being to a
# AD9833

# IMPORTS
import spidev
import RPi.GPIO as GPIO

import time


freq = 1_000_000 #Max SPI Frequency for the AD9833 is 40 MHz

master_clk = 2_000_000 # Has a direct relation to the output waveform frequency

GPIO.setmode(GPIO.BCM)		#set pin numbering system
GPIO.setup(12,GPIO.OUT)
pi_pwm = GPIO.PWM(12,master_clk)		#create PWM instance with frequency
pi_pwm.start(0.5)	

# Setup SPI
spi = spidev.SpiDev()
spi.open(0, 1)  # Bus 0, Device 1
spi.max_speed_hz = freq

def AD9833_write(spi, data_high, data_low):
    """
    Write 2 bytes to the specified register.
    """
    
    bus = 0
    device = 1

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = 900_000 
    spi.mode = 0

    data_out = bytearray([data_high, data_low])
    spi.xfer(data_out)

    spi.close()



def AD9833_set_freq(frequency):
    # should be used with the Master input clock frequency of 10 MHz
    global master_clk
    global spi
    
    FRQREG = int((frequency) * (2**28) / (master_clk))  # Page 15
    FRQREG = FRQREG // 4  # For some reason I have to do this adjustment
    
    lb = FRQREG & 0b11111111
    hb = ((FRQREG & 0b111100000000) >> 8) + 0b01000000
    AD9833_write(spi, 0b0001_0000, 0b0000_0000)
    AD9833_write(spi, hb, lb)

def AD9833_Init():
    # Initialize AD9833
    AD9833_write(spi, 0b0001_0000, 0b0000_0000)
    AD9833_write(spi, 0b0100_0000, 0b1111_1111)

def main():
    AD9833_Init()
    # Set frequency to 100 kHz
    AD9833_set_freq(100_000)

    print("Done initializing AD9833")
    while(1):
        pass

if __name__ == "__main__":
    main()