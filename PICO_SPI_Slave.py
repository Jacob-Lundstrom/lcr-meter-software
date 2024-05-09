from machine import Pin 
import time


# This follows the scheme for the real SPI0 Master driver
nCS  = 1
CLK  = 2
MISO = 0
MOSI = 3

nCS_pin  = Pin(nCS, Pin.IN, None) #This can probably be replaced with an IRQ later
CLK_pin  = Pin(CLK, Pin.IN, Pin.PULL_DOWN) # Indicates no pull resistor
MISO_pin = Pin(MISO, Pin.OUT) # Leave as Hi-Z while not in use
MOSI_pin = Pin(MOSI, Pin.IN, None) # NC for now, no data in.

def send_byte(data: int):
    # the data variable will only be 8 bits long. Any additional bits will be ignored.
    send_data = data & 0xFF
    #print(f"send_data: {send_data}")
    recieve_data = 0x00
    
    # Wait for nCS to be pulled low
    nCS_state = nCS_pin.value()
    #print(f"nCS_state: {nCS_state}")
    while(nCS_state == 1):
        nCS_state = nCS_pin.value()
        #print(f"nCS_state: {nCS_state}")
        
    # print("CS de-asserted")
           
    # Here, data is being transferred.
    # With using SPI mode 0:
    MISO_pin = Pin(MISO, Pin.OUT)
    recieve_pos = 0
    while (recieve_pos < 8):
        nCS_state = nCS_pin.value()
        send_data = send_data >> recieve_pos
        # print(f"send_data: {send_data}")
        send_bit = send_data & 0x01
        # print(f"send_bit: {send_bit}, send_pos:{recieve_pos}")
        
        CLK_state = CLK_pin.value()
        while (CLK_state):
#             nCS_state = nCS_pin.value()
#             if nCS_state:
#                 recieve_pos = 8
#                 break
            CLK_state = CLK_pin.value() # Simple way to wait while the clock polarity is high
        
        MISO_pin.value(send_bit)
        
        CLK_state = CLK_pin.value()
        # print(f"CLK_state: {CLK_state}")
        while (CLK_state == 0):
#             nCS_state = nCS_pin.value()
#             if nCS_state:
#                 recieve_pos = 8
#                 break
            CLK_state = CLK_pin.value() # Simple way to wait while the clock polarity is low
            
        # At this line, the data should be sampled, as CLK_pin.value() == 1.
        recieve_data = recieve_data << recieve_pos + MOSI_pin.value()
        recieve_pos = recieve_pos + 1
        #print(f"recieve_data: {recieve_data}")
        #print(f"recieve_pos: {recieve_pos}")
            
        # When this line is reached, the cycle should repeat.
    
    # When this line is reached, we should quit driving the pin
    # Do this through changing the pin mode to input.
    MISO_pin = Pin(MISO, Pin.IN, None)
    
    while(nCS_pin.value() == 0):
        time.sleep(0)
        
if __name__ == "__main__":
    while(True):
        print("sending data...")
        send_byte(0x3F)
        print("data sent.")
        
            