/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <stdbool.h>
#include "AD9833_STM32.h"
#include "MCP3202_STM32.h"

#include "matrix_operations.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"


#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t frequency = 1000;
int FREQ_UPDATE_NEEDED = 1; // Flag to tell the system that it needs to update the test frequency.
// All the data must be floating point, as the nucleo board I'm using doesn't have enough memory for double.

#define MAX_POINTS 2000
uint16_t ch0_data[MAX_POINTS];
uint16_t ch1_data[MAX_POINTS];

bool CH0_DONE_SAMPLING = false;
bool CH1_DONE_SAMPLING = false;

float Get_Time_us(void) {
	float count = __HAL_TIM_GET_COUNTER(&htim2);
	float FRQ = 2e6;
	return count * 1e6 / FRQ; // Read timer
}

void wait_us(uint32_t w) {
  uint32_t s = Get_Time_us();
  while(Get_Time_us() - s < w);
}

void ADC_START() {
	HAL_TIM_Base_Stop(&htim3);// Make sure that the timer is stopped, so we don't start ADC conversions in the middle of processing

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ch1_data, MAX_POINTS); // This is required for all ADC conversion types
	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)ch0_data, MAX_POINTS);

	HAL_Delay(10); // Should be enough time for calibration?

	__HAL_TIM_SET_COUNTER(&htim2, 0);
	HAL_TIM_Base_Start(&htim3);

	CH0_DONE_SAMPLING = false;
	CH1_DONE_SAMPLING = false;
}

void convert_to_double(const uint16_t *input_array, double *output_array, size_t length) {
    const double scale_factor = 3.3 / 4095.0;
    for (int i = 0; i < length; i++) {
    	double o = input_array[i] * scale_factor;
        output_array[i] = o;
    }
}

int get_range() {
	// This function figures out what range is currently being used.
	// Returns the number corresponding to the range.
	// If there is a problem, it will return -1.
	int r0 = HAL_GPIO_ReadPin(GPIOC, RANGE_0_Pin); // 100 Ohms
	int r1 = HAL_GPIO_ReadPin(GPIOC, RANGE_1_Pin); // 1000 Ohms
	int r2 = HAL_GPIO_ReadPin(GPIOC, RANGE_2_Pin); // 10000 Ohms
	int r3 = HAL_GPIO_ReadPin(GPIOC, RANGE_3_Pin); // 100000 Ohms

	int n = (r0*1) + (r1*10) + (r2*100) + (r3*1000);
	if (n == 0) {
		return -1;
	}
	if (n == 1) {
		return 0;
	}
	if (n == 10) {
		return 1;
	}
	if (n == 100) {
		return 2;
	}
	if (n == 1000){
		return 3;
	}
	return -1;
}

int set_range(int range) {
	if (range < 0) {
		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 0); // 100 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 0); // 1000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 0); // 10000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 0); // 100000 Ohms

		return -1;
	}
	if (range > 3) {
		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 0); // 100 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 0); // 1000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 0); // 10000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 0); // 100000 Ohms

		return -1;
	}

	if (range == 0) {
		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 0); // 1000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 0); // 10000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 0); // 100000 Ohms

		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 1); // 100 Ohms
		HAL_Delay(100);
		return 1;
	} else if(range == 1) {
		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 0); // 100 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 0); // 10000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 0); // 100000 Ohms

		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 1); // 1000 Ohms
		HAL_Delay(100);
		return 1;
	} else if (range == 2) {
		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 0); // 100 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 0); // 1000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 0); // 100000 Ohms

		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 1); // 10000 Ohms
		HAL_Delay(100);
		return 1;
	} else {
		HAL_GPIO_WritePin(GPIOC, RANGE_0_Pin, 0); // 100 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_1_Pin, 0); // 1000 Ohms
		HAL_GPIO_WritePin(GPIOC, RANGE_2_Pin, 0); // 10000 Ohms

		HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin, 1); // 100000 Ohms
		HAL_Delay(100);
		return 1;
	}

}

double get_shunt_resistance() {

	int range = get_range();

	double shunt_resistance;
	if (range == 0) {
		shunt_resistance = 102.10;
	} else if (range == 1){
		shunt_resistance = 1002.8;
	} else if (range == 2){
		shunt_resistance = 10012;
	} else { // No fault handling for now. Should never be a fault :)
		shunt_resistance = 107250;
	}
	return shunt_resistance;
}

double get_amp_calibration() {
	// This needs to be done for each meter/setup
	double ACF0 = 19.959 / 20;
	double ACF1 = 19.887 / 20;
	double ACF2 = 19.826 / 20;
	double ACF3 = 15.856 / 20;

	int range = get_range();

	if (frequency == 1000) {
		if (range == 0) {
			return ACF0;
		} else if (range == 1) {
			return ACF1;
		} else if (range == 2) {
			return ACF2;
		} else if (range == 3) {
			return ACF3;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}

double get_phase_calibration() {
	// This needs to be done for each meter/setup
	double PCF0 = -0.0014542840503944804;
	double PCF1 = -0.003935525496759973;
	double PCF2 = -0.030454848010975977;
	double PCF3 = -0.034066954299771218;

	int range = get_range();
	if (frequency == 1000) {
		if (range == 0) {
			return PCF0;
		} else if (range == 1) {
			return PCF1;
		} else if (range == 2) {
			return PCF2;
		} else if (range == 3) {
			return PCF3;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int auto_range(float magnitude) {
	// This function changes the current range resistor.
	// The return value is 0 if the range is unchanged, or 1 if the range has successfully changed.
	int current_range = get_range();

	if (magnitude > 5.5 * (100 * pow(10,current_range))){
		if (current_range != 2) {
			set_range(current_range + 1);
			return 1;
		}
		return 0;
	}
	if (magnitude < 5.5 * (100 * pow(10,current_range - 1))) {
		if (current_range != 0) {
			set_range(current_range - 1);
			return 1;
		}
		return 0;
	}
	return 0;
}

void Show_Stats(double reactance, double resistance, double frequency){
//	ILI9341_Fill_Screen(BLACK);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);

	double X = reactance;
	double R = resistance;

	// Make sure in project properties to add the linker flag -u _printf_float

    char buffer[50]; // Buffer to hold the formatted string
	if (X < 0){
		float capacitance = -1e9 / (2 * M_PI * frequency * X);
		ILI9341_Draw_Text("                        ", 10, 40, WHITE, 2, BLACK);
		snprintf(buffer, sizeof(buffer), "Cs: %.3f nF", capacitance);
		ILI9341_Draw_Text(buffer, 10, 40, WHITE, 2, BLACK);


        char impedance[50];
        snprintf(impedance, sizeof(impedance), "Z: %.2f - j %.2f", R, -X);

		ILI9341_Draw_Text("                        ", 10, 120, WHITE, 2, BLACK);
		ILI9341_Draw_Text(impedance, 10, 120, WHITE, 2, BLACK);

	} else {
        float inductance = X * 1e6 / (2 * M_PI * frequency);
		ILI9341_Draw_Text("                        ", 10, 40, WHITE, 2, BLACK);
        snprintf(buffer, sizeof(buffer), "Ls: %.3f uH", inductance);
        ILI9341_Draw_Text(buffer, 10, 40, WHITE, 2, BLACK);

        char impedance[50];
        snprintf(impedance, sizeof(impedance), "Z: %.3f + j %.3f", R, X);

		ILI9341_Draw_Text("                        ", 10, 120, WHITE, 2, BLACK);
		ILI9341_Draw_Text(impedance, 10, 120, WHITE, 2, BLACK);
	}

	ILI9341_Draw_Text("                        ", 10, 80, WHITE, 2, BLACK);
	snprintf(buffer, sizeof(buffer), "ESR: %.2f Ohms", R);
	ILI9341_Draw_Text(buffer, 10, 80, WHITE, 2, BLACK);

	ILI9341_Draw_Text("                        ", 10, 160, WHITE, 2, BLACK);
	snprintf(buffer, sizeof(buffer), "Freq: %.2f kHz", frequency / 1000);
	ILI9341_Draw_Text(buffer, 10, 160, WHITE, 2, BLACK);


	ILI9341_Draw_Text("                        ", 10, 200, WHITE, 2, BLACK);
	snprintf(buffer, sizeof(buffer), "Range: %i", get_range());
	ILI9341_Draw_Text(buffer, 10, 200, WHITE, 2, BLACK);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */



  memset(ch0_data, 0, MAX_POINTS);  // Set all bytes in the array to 0
  memset(ch1_data, 0, MAX_POINTS);  // Set all bytes in the array to 0

	__HAL_TIM_SET_COUNTER(&htim2, 0);
	HAL_TIM_Base_Start(&htim2);

	double load_amplitude = 0;
	double load_phase = 0;
	double load_offset = 0;

	double shunt_amplitude = 0;
	double shunt_phase = 0;
	double shunt_offset = 0;


	ILI9341_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if (FREQ_UPDATE_NEEDED > 0) {
		  AD9833_set_freq(frequency);
		  AD9833_set_freq(frequency);
		  // I have to do this twice because I'm using two SPI devices on this bus which
		  // use different modes. After an ADC reading, the clock idles low. This
		  // for some reason can't be changed by just reconfiguring the SPI bus (as I am doing in the
		  // AD9833 driver) but rather actually requires an spi transfer to initiate the correct
		  // Polarity clock for the start of the next transfer. This is required because the AD9833
		  // Requires the clock to idle HIGH before the CS/FSYNC pin goes low. I'm too lazy to figure
		  // out how to fix this easily. This works, so I'm keeping it.
		  FREQ_UPDATE_NEEDED = 0; // clear flag
		  HAL_Delay(1000); // allow for the frequency to stabilize before sampling
	  }


	  double sampleF = frequency; // samples and hold the frequency to ensure it won't change while in the middle of gathering smaples
	  double sampleRate = 1.4e6;
	  double avgMag = 0;
	  double avgPhase = 0;
	  double avgR = 0;
	  double avgX = 0;
	  int cycles = 5;

	  for (int i = 0; i < cycles; i++) {

		  ADC_START();
		  while(!CH0_DONE_SAMPLING || !CH1_DONE_SAMPLING); // Wait for samples to be  collected

		  uint32_t collected_points = MAX_POINTS; // Testing new method with all the available points

		  double DBL_BUF[collected_points];

		  convert_to_double(ch1_data, DBL_BUF, collected_points);
		  least_squares_sine(collected_points, sampleF, sampleRate, 0, DBL_BUF, &shunt_amplitude, &shunt_phase, &shunt_offset);

		  convert_to_double(ch0_data, DBL_BUF, collected_points);
		  least_squares_sine(collected_points, sampleF, sampleRate, 0, DBL_BUF, &load_amplitude, &load_phase, &load_offset);

		  double shunt_resistance = get_shunt_resistance();

		  double impedance_magnitude = (load_amplitude / (shunt_amplitude / shunt_resistance));
		  double impedance_angle = (load_phase - shunt_phase);
		  if (impedance_angle > M_PI) {
			  impedance_angle -= 2 * M_PI;
		  }
		  avgMag += impedance_magnitude;
		  avgPhase += impedance_angle;

		  __HAL_TIM_SET_COUNTER(&htim2, 0);

	  }
	  double ACF = get_amp_calibration(); // Calibration factor, range 2
	  double PCF = get_phase_calibration();
	  avgMag = (avgMag / cycles) * ACF;

	  // Frequency 1000
	  avgPhase = (avgPhase / cycles) + PCF; // Range 3

	  avgX = avgMag * sin(avgPhase);
	  avgR = avgMag * cos(avgPhase);

	  Show_Stats(avgX, avgR, sampleF);


	  auto_range(sqrt(avgX * avgX + avgR * avgR));

//	  set_range(3);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 41;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_NCS_Pin|LCD_RST_Pin|LCD_DC_Pin|LCD_LED_Pin
                          |RANGE_2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RANGE_3_Pin|RANGE_1_Pin|RANGE_0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, AD9833_NCS_Pin|MCP3202_NCS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LCD_NCS_Pin LCD_RST_Pin LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_NCS_Pin|LCD_RST_Pin|LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_LED_Pin RANGE_3_Pin RANGE_2_Pin RANGE_1_Pin
                           RANGE_0_Pin */
  GPIO_InitStruct.Pin = LCD_LED_Pin|RANGE_3_Pin|RANGE_2_Pin|RANGE_1_Pin
                          |RANGE_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Button_0_Pin Button_1_Pin */
  GPIO_InitStruct.Pin = Button_0_Pin|Button_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : AD9833_NCS_Pin MCP3202_NCS_Pin */
  GPIO_InitStruct.Pin = AD9833_NCS_Pin|MCP3202_NCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//	frequency = frequency + 1000;
	frequency = frequency * 10;
	if (frequency > 100000) {
		frequency = 100;
	}
	FREQ_UPDATE_NEEDED = 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	// WE're done collecting samples
	if (hadc == &hadc1) {
		CH1_DONE_SAMPLING = true;
	}
	else if (hadc == &hadc2) {
		CH0_DONE_SAMPLING = true;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Check which version of the timer triggered this callback and toggle LED
	if (htim == &htim3 )
	{
		HAL_TIM_Base_Stop(&htim3);// Make sure that the timer is stopped, so we don't start ADC conversions in the middle of processing
	}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
