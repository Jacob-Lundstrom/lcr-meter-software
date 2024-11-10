/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_NCS_Pin GPIO_PIN_0
#define LCD_NCS_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_2
#define LCD_DC_GPIO_Port GPIOC
#define LCD_LED_Pin GPIO_PIN_3
#define LCD_LED_GPIO_Port GPIOC
#define ADC_CH0_Pin GPIO_PIN_0
#define ADC_CH0_GPIO_Port GPIOA
#define ADC_CH1_Pin GPIO_PIN_1
#define ADC_CH1_GPIO_Port GPIOA
#define RANGE_3_Pin GPIO_PIN_6
#define RANGE_3_GPIO_Port GPIOC
#define RANGE_2_Pin GPIO_PIN_7
#define RANGE_2_GPIO_Port GPIOC
#define RANGE_1_Pin GPIO_PIN_8
#define RANGE_1_GPIO_Port GPIOC
#define RANGE_0_Pin GPIO_PIN_9
#define RANGE_0_GPIO_Port GPIOC
#define AD9833_NCS_Pin GPIO_PIN_8
#define AD9833_NCS_GPIO_Port GPIOA
#define MCP3202_NCS_Pin GPIO_PIN_9
#define MCP3202_NCS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
