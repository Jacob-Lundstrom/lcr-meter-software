# This file will be used to generate sine waves. This should
# be programmed on an RP2040, with the communcation being to a
# AD9833

# IMPORTS
from machine import Pin, SPI, Timer, PWM
import time
import math

# SPI COMMUNICATION INTERFACE PINS
nCS = 17
MISO = 16 # can be left NC, not used for the DAC
MOSI = 19
SCK = 18
freq = 1_000_000 #Max SPI Frequency for the AD9833 is 40 MHz

master_clk = 2_000_000 # Has a direct relation to the output waveform frequency

nCS_pin = Pin(nCS, Pin.OUT)
nCS_pin.value(1)

spi = SPI(0,
          baudrate=freq,
          sck=Pin(SCK),
          mosi=Pin(MOSI),
          miso=Pin(MISO))

def AD9833_write(spi, cs, data_high, data_low):
    """
    Write 2 bytes to the specified register.
    """
    msg1 = bytearray()
    msg1.append(data_high)
    
    msg2 = bytearray()
    msg2.append(data_low)
    
    msg = msg1 + msg2
    # Send out SPI message
    cs.value(0)
    spi.write(msg)
    cs.value(1)

CLK = PWM(Pin(20))
CLK.freq(master_clk)
CLK.duty_u16(2**15)

# From datasheet, page 15, Table 11
AD9833_write(spi, nCS_pin, 0b0001_0000, 0b0000_0000)
AD9833_write(spi, nCS_pin, 0b0100_0000, 0b1111_1111)

def AD9833_set_freq(frequency):
    # should be used with the Master input clock frequency of 10 MHz
    global master_clk
    global spi
    
    FRQREG = int((frequency) * (2**28) / (master_clk)) # Page 15
    FRQREG = FRQREG // 4 # For some reason I have to do this adjustment
    
    lb = FRQREG & 0b1111_1111
    hb = ((FRQREG & 0b1111_0000_0000) >> 8) + 0b0100_0000
    AD9833_write(spi, nCS_pin, 0b0001_0000, 0b0000_0000)
    AD9833_write(spi, nCS_pin, hb, lb)

AD9833_set_freq(100_000)

while(1):
    pass
    
    
