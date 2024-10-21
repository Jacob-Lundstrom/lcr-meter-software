#ifndef AD9833_H
#define AD9833_H

#include "main.h" // Include necessary STM32 HAL libraries

// Define the AD9833 SPI frequency and master clock frequency
#define AD9833_SPI_FREQ 20000000  // Maximum SPI Frequency for AD9833 is 40 MHz
#define MASTER_CLK 1000000        // Master clock frequency of AD9833 (in Hz)

// Function prototypes for interacting with the AD9833
void AD9833_write(uint8_t data_high, uint8_t data_low);
void AD9833_write4(uint8_t data_l_high, uint8_t data_l_low, uint8_t data_h_high, uint8_t data_h_low);
void AD9833_set_freq(uint64_t f);
void AD9833_Init(void);

#endif /* AD9833_H */
