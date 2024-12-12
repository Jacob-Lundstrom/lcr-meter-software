#ifndef MCP3202_H
#define MCP3202_H

#include "main.h"  // Ensure you include the main header where system-wide includes and SPI are defined
#include <stdint.h>

// Command bytes for MCP3202 channels
#define MCP3202_CH0  0b11010000  // Command for channel 0
#define MCP3202_CH1  0b11110000  // Command for channel 1

// Function declarations
float ADC_Channel0(void);
float ADC_Channel1(void);

#endif // MCP3202_H
