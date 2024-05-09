from machine import Pin 
import time


# This follows the scheme for the real SPI0 Master driver
nCS  = 1
CLK  = 2
MISO = 0
MOSI = 3

nCS_pin  = Pin(nCS, Pin.IN, None) #This can probably be replaced with an IRQ later
CLK_pin  = Pin(CLK, Pin.IN, None) # Indicates no pull resistor
MISO_pin = Pin(MISO, Pin.OUT) # Leave as Hi-Z while not in use
MOSI_pin = Pin(MOSI, Pin.IN, None) # NC for now, no data in.

transfer_complete = True
next_position = 7
next_value = 0

global send_data

def update_MISO(pin):
    # This should be called every time the CLK signal is changed to high to follow
    # Motorola SPI mode 0.
    
    global send_data
    global next_value
    global next_position
    global transfer_complete
    global recieve_data
    
#     CLK_pin.irq(trigger=Pin.IRQ_RISING, handler=read_MOSI)
    
    
    if next_position > -1:
        next_value = (send_data >> (next_position)) & 0b1
        next_position -= 1
    
        MISO_pin.value(next_value)
#         print(next_value)
    
    if next_position == -1:
        finish_transfer()

def read_MOSI(pin=None):
    global recieve_data
    global CLK_pin
    
    recieve_data |= (MOSI_pin.value() << (next_position + 1))
    CLK_pin.irq(trigger=Pin.IRQ_FALLING, handler=update_MISO)
    
def finish_transfer(pin=None):
    # This should be called when the nCS pin is pulled high, indicates we should not send data.
    
    global transfer_complete
    global next_position
    global nCS_pin
    global CLK_pin
    
    transfer_complete = True
    nCS_pin.irq(handler=None)
    CLK_pin.irq(handler=None)
    if next_position == -1:
        print("Transfer Completed Correctly")
    else:
        print("FAILED")
        print(f"Last position to send: {next_position}")
    
def start_transfer(pin=None):
    global MISO_pin
    global CLK_pin
    global nCS_pin
    
    MISO_pin = Pin(MISO, Pin.OUT)
    update_MISO(None)
#     CLK_pin.irq(trigger=Pin.IRQ_RISING, handler=read_MOSI)
    CLK_pin.irq(trigger=Pin.IRQ_FALLING, handler=update_MISO)
    nCS_pin.irq(trigger=Pin.IRQ_RISING, handler=finish_transfer)

def send_byte(data: int) -> None:
    # Call this funciotn once per byte transfered.
    
    global send_data
    global next_value
    global next_position
    global MISO_pin
    global transfer_complete
    global recieve_data
    
    transfer_complete = False
    
    # the data variable will only be 8 bits long. Any additional bits will be ignored.
    send_data = data & 0xFF
    recieve_data = 0x00
    next_position = 7 
    
    nCS_pin.irq(trigger=Pin.IRQ_FALLING, handler=start_transfer)
    
    while(transfer_complete == False):
        time.sleep_us(1)

    # When this line is reached, we should quit driving the pin
    # Do this through changing the pin mode to input.
    
    MISO_pin = Pin(MISO, Pin.IN, None)
    
    return recieve_data
        
if __name__ == "__main__":
    byte = 0b10101010
    byte = 0b1
    while(True):
        byte = byte + 1
        rec = send_byte(byte)
        print(byte)
