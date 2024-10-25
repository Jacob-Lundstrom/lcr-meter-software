// Converted from the original Python file

#include "AD9833_STM32.h"

#define AD9833_SPI_FREQ 20000000 // Max SPI Frequency for AD9833
#define MASTER_CLK 25000000       // Relation to the output waveform frequency

// MaKE SURE THIS IS DEFINED IN THE main.c FILE
extern SPI_HandleTypeDef hspi1; // SPI handle (Assuming SPI1 is used)


void AD9833_write(uint8_t data_high, uint8_t data_low) {
    uint8_t data_out[2] = {data_high, data_low};
    

    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;  // CPOL = 1 (Clock idle high)
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // CPHA = 0 (Sample on the falling edge)
    HAL_SPI_Init(&hspi1); // Restart the SPI module

    HAL_GPIO_WritePin(AD9833_NCS_GPIO_Port, AD9833_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, data_out, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(AD9833_NCS_GPIO_Port, AD9833_NCS_Pin, GPIO_PIN_SET);
}

void AD9833_write4(uint8_t data_l_high, uint8_t data_l_low, uint8_t data_h_high, uint8_t data_h_low) {
    uint8_t data_out[4] = {data_l_high, data_l_low, data_h_high, data_h_low};

    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;  // CPOL = 1 (Clock idle high)
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // CPHA = 0 (Sample on the falling edge)
    HAL_SPI_Init(&hspi1); // Restart the SPI module

    HAL_GPIO_WritePin(AD9833_NCS_GPIO_Port, AD9833_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, data_out, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(AD9833_NCS_GPIO_Port, AD9833_NCS_Pin, GPIO_PIN_SET);
}

void AD9833_set_freq(float f) {
    AD9833_write(0b00100000, 0b10000000); // Control reg, start reset

    uint64_t FRQREG = (uint64_t)((f * (1 << 28)) / MASTER_CLK);

    // Lower bytes
    uint16_t FL = FRQREG & 0x3FFF;
    uint8_t Llb = FL & 0xFF;
    uint8_t Lhb = ((FL >> 8) & 0x3F) | 0b01000000;

    // Higher bytes
    uint16_t FH = (FRQREG >> 14) & 0x3FFF;
    uint8_t Hlb = FH & 0xFF;
    uint8_t Hhb = ((FH >> 8) & 0x3F) | 0b01000000;

    AD9833_write4(Lhb, Llb, Hhb, Hlb);

    AD9833_write(0b00100000, 0b00000000); // Control reg, finish reset and configure IC
}

void AD9833_Init() {
    AD9833_write(0b00010000, 0b00000000); // Reset
    AD9833_write(0b01000000, 0b11111111); // Set frequency register
}
