#include "MCP3202_STM32.h"


// MaKE SURE THIS IS DEFINED IN THE main.c FILE
extern SPI_HandleTypeDef hspi1; // SPI handle (Assuming SPI1 is used)

// Function to read from ADC Channel 0 (CH0)
float ADC_Channel0(void) {
    uint8_t data_out[2] = { MCP3202_CH0, 0x00 };
    uint8_t data_in[2] = { 0x00, 0x00 };


    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;   // CPOL = 0 (Clock idle Low)
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // CPHA = 0 (Sample on the falling edge)
    HAL_SPI_Init(&hspi1); // Restart the SPI module

    // Transmit and receive data via SPI
    HAL_GPIO_WritePin(MCP3202_NCS_GPIO_Port, MCP3202_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, data_out, data_in, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(MCP3202_NCS_GPIO_Port, MCP3202_NCS_Pin, GPIO_PIN_SET);

    // Combine the received bytes to form the 12-bit ADC value
    uint16_t adc_value = ((data_in[0] & 0x0F) << 8) | data_in[1];

    // Calculate the voltage based on the 3.3V reference and 12-bit ADC resolution
    float voltage = 3.3f * adc_value / (float)(1 << 11); // 12-bit resolution (2^12)

    return voltage;
}

// Function to read from ADC Channel 1 (CH1)
float ADC_Channel1(void) {
    uint8_t data_out[2] = { MCP3202_CH1, 0x00 };
    uint8_t data_in[2] = { 0x00, 0x00 };


    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;   // CPOL = 0 (Clock idle Low)
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // CPHA = 0 (Sample on the falling edge)
    HAL_SPI_Init(&hspi1); // Restart the SPI module

    // Transmit and receive data via SPI
    HAL_GPIO_WritePin(MCP3202_NCS_GPIO_Port, MCP3202_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, data_out, data_in, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(MCP3202_NCS_GPIO_Port, MCP3202_NCS_Pin, GPIO_PIN_SET);

    // Combine the received bytes to form the 12-bit ADC value
    uint16_t adc_value = ((data_in[0] & 0x0F) << 8) | data_in[1];

    // Calculate the voltage based on the 3.3V reference and 12-bit ADC resolution
    float voltage = 3.3f * adc_value / (float)(1 << 11); // 12-bit resolution (2^12)

    return voltage;
}
