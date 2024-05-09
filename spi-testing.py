import spidev
import time

def test_transfer() -> None:
    bus = 0
    device = 0

    spi = spidev.SpiDev()
    spi.open(bus,device)
    spi.max_speed_hz = 7+000 # 7 kHz clk speed
    spi.mode = 0

    # data_out = [0xFF,0x02,0xFF,0x04,0xFF]
    data_out = [0x3f]

    data_in = (spi.xfer2(data_out))

    print("Recieved data: ", data_in)

    spi.close()

if __name__ == "__main__":
    while(True):
        test_transfer()
        time.sleep(2)