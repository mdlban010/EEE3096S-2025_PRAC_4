/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdio.h>
#include "stm32f4xx.h"
#include "lcd_stm32f4.h"
#include <stm32f4xx_hal_tim.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// TODO: Add values for below variables

// logic behind the values:
      /* NS = 128, number of samples generated, TIM2CLK = 16MHz, given,
      Sample rate = F_signal * NS
      For the waveforms (sine, sawtooth, triangle), want to generate audible tones.
      The human hearing range is approximately 20 Hz to 20 kHz.
      Sample Rate = 44,100 Hz
      F_SIGNAL = Sample_Rate / NS = 44,100 / 128 ≈ 344.53 Hz
      Maximum Sample Rate = TIM2CLK / TIM2_Ticks_min
      assuming the minumum of TIM2_Ticks =1,
      Max Sample Rate = 16,000,000 Hz
      Max F_SIGNAL = 16,000,000 / 128 = 125,000 Hz
      */
#define NS 128        // Number of samples in LUT
#define TIM2CLK  16000000  // STM Clock frequency: 16MHz
#define F_SIGNAL 344 	// Frequency of output analog signal

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim2_ch1;

/* USER CODE BEGIN PV */
// TODO: Add code for global variables, including LUTs
uint32_t Sin_LUT[NS] = {2047, 2147, 2248, 2347, 2446, 2545, 2641, 2737,
    2831, 2922, 3012, 3100, 3185, 3267, 3346, 3422,
    3495, 3564, 3630, 3692, 3749, 3803, 3853, 3898,
    3939, 3975, 4006, 4033, 4055, 4072, 4085, 4092,
    4095, 4092, 4085, 4072, 4055, 4033, 4006, 3975,
    3939, 3898, 3853, 3803, 3749, 3692, 3630, 3564,
    3495, 3422, 3346, 3267, 3185, 3100, 3012, 2922,
    2831, 2737, 2641, 2545, 2446, 2347, 2248, 2147,
    2047, 1947, 1846, 1747, 1648, 1549, 1453, 1357,
    1263, 1172, 1082, 994, 909, 827, 748, 672,
    599, 530, 464, 402, 345, 291, 241, 196,
    155, 119, 88, 61, 39, 22, 9, 2,
    0, 2, 9, 22, 39, 61, 88, 119,
    155, 196, 241, 291, 345, 402, 464, 530,
    599, 672, 748, 827, 909, 994, 1082, 1172,
    1263, 1357, 1453, 1549, 1648, 1747, 1846, 1947};

uint32_t Saw_LUT[NS] = {0, 31, 63, 95, 127, 159, 191, 223,
    255, 287, 319, 351, 383, 415, 447, 479,
    511, 543, 575, 607, 639, 671, 703, 735,
    767, 799, 831, 863, 895, 927, 959, 991,
    1023, 1055, 1087, 1119, 1151, 1183, 1215, 1247,
    1279, 1311, 1343, 1375, 1407, 1439, 1471, 1503,
    1535, 1567, 1599, 1631, 1663, 1695, 1727, 1759,
    1791, 1823, 1855, 1887, 1919, 1951, 1983, 2015,
    2047, 2079, 2111, 2143, 2175, 2207, 2239, 2271,
    2303, 2335, 2367, 2399, 2431, 2463, 2495, 2527,
    2559, 2591, 2623, 2655, 2687, 2719, 2751, 2783,
    2815, 2847, 2879, 2911, 2943, 2975, 3007, 3039,
    3071, 3103, 3135, 3167, 3199, 3231, 3263, 3295,
    3327, 3359, 3391, 3423, 3455, 3487, 3519, 3551,
    3583, 3615, 3647, 3679, 3711, 3743, 3775, 3807,
    3839, 3871, 3903, 3935, 3967, 3999, 4031, 4063};

uint32_t Triangle_LUT[NS] = { 0, 63, 127, 191, 255, 319, 383, 447,
    511, 575, 639, 703, 767, 831, 895, 959,
    1023, 1087, 1151, 1215, 1279, 1343, 1407, 1471,
    1535, 1599, 1663, 1727, 1791, 1855, 1919, 1983,
    2047, 2111, 2175, 2239, 2303, 2367, 2431, 2495,
    2559, 2623, 2687, 2751, 2815, 2879, 2943, 3007,
    3071, 3135, 3199, 3263, 3327, 3391, 3455, 3519,
    3583, 3647, 3711, 3775, 3839, 3903, 3967, 4031,
    4095, 4031, 3967, 3903, 3839, 3775, 3711, 3647,
    3583, 3519, 3455, 3391, 3327, 3263, 3199, 3135,
    3071, 3007, 2943, 2879, 2815, 2751, 2687, 2623,
    2559, 2495, 2431, 2367, 2303, 2239, 2175, 2111,
    2047, 1983, 1919, 1855, 1791, 1727, 1663, 1599,
    1535, 1471, 1407, 1343, 1279, 1215, 1151, 1087,
    1023, 959, 895, 831, 767, 703, 639, 575,
    511, 447, 383, 319, 255, 191, 127, 63};

uint32_t Piano_LUT = {2047, 4095, 0, 4095, 4095, 4095, 0, 4095,
    4095, 4095, 4095, 1096, 0, 4095, 4095, 0,
    0, 0, 0, 4095, 0, 4095, 0, 0,
    4095, 3837, 4095, 0, 0, 0, 4095, 4095,
    0, 4095, 0, 4095, 0, 4095, 0, 0,
    0, 4095, 4095, 0, 4095, 0, 4095, 1644,
    4095, 4095, 4095, 0, 0, 0, 4095, 0,
    4095, 0, 0, 0, 0, 0, 0, 4095,
    0, 0, 4095, 4095, 0, 0, 0, 4095,
    0, 4095, 0, 4095, 4095, 0, 4095, 4095,
    4095, 4095, 0, 4095, 4095, 0, 4095, 4095,
    4095, 0, 0, 0, 0, 4095, 806, 4095,
    4095, 0, 4095, 4095, 4095, 0, 0, 0,
    0, 0, 4095, 4095, 0, 0, 0, 4095,
    0, 4095, 0, 4095, 4095, 0, 0, 0,
    0, 0, 4095, 4095, 0, 4095, 0, 2047};

uint32_t Guitar_LUT = {2047, 4095, 4095, 4095, 4095, 0, 0, 4095,
    0, 4095, 4095, 0, 4095, 4095, 0, 4095,
    0, 4095, 0, 4095, 0, 0, 4095, 0,
    0, 0, 0, 4095, 0, 0, 0, 4095,
    4095, 0, 0, 4095, 0, 4095, 0, 0,
    0, 4095, 4095, 4095, 4095, 0, 4095, 0,
    0, 0, 4095, 0, 4095, 0, 0, 4095,
    0, 4095, 4095, 0, 0, 4095, 4095, 0,
    0, 4095, 4095, 4095, 4095, 4095, 0, 4095,
    4095, 0, 4095, 4095, 4095, 0, 4095, 4095,
    4095, 4095, 0, 4095, 4095, 0, 0, 0,
    4095, 4095, 0, 0, 0, 0, 4095, 0,
    4095, 0, 4095, 0, 4095, 0, 4095, 4095,
    0, 4095, 0, 4095, 4095, 0, 4095, 0,
    0, 4095, 4095, 0, 4095, 0, 0, 0,
    4095, 0, 4095, 0, 0, 4095, 2837, 0};

uint32_t Drum_LUT = {2047, 4095, 0, 4095, 4095, 4095, 0, 0,
    4095, 4095, 0, 0, 0, 0, 0, 0,
    4095, 0, 1902, 4095, 4095, 4095, 0, 0,
    4095, 0, 4095, 4095, 4095, 0, 4095, 0,
    0, 4095, 4095, 4095, 0, 4095, 0, 709,
    4095, 0, 0, 0, 4095, 4095, 0, 0,
    4095, 4095, 0, 4095, 0, 4095, 4095, 4095,
    0, 4095, 0, 0, 4095, 4095, 4095, 4095,
    4095, 0, 4095, 0, 4095, 4095, 4095, 4095,
    0, 0, 4095, 0, 4095, 0, 1418, 4095,
    0, 4095, 0, 3740, 4095, 4095, 4095, 0,
    4095, 4095, 0, 4095, 0, 4095, 0, 0,
    0, 4095, 0, 0, 3288, 0, 4095, 0,
    0, 4095, 4095, 0, 4095, 4095, 4095, 4095,
    4095, 2160, 0, 0, 4095, 4095, 4095, 4095,
    4095, 0, 0, 0, 0, 0, 4095, 2047};

// TODO: Equation to calculate TIM2_Ticks
// TIM2_Ticks determines how often to update the PWM duty cycle
// Formula: TIM2_Ticks = TIM2CLK / (F_SIGNAL × NS)
// TIM2_Ticks = 16,000,000 / (344 × 128) = 16,000,000 / 44,032 ≈ 363
uint32_t TIM2_Ticks = TIM2CLK / (F_SIGNAL * NS); // How often to write new LUT value
uint32_t DestAddress = (uint32_t) &(TIM3->CCR3); // Write LUT TO TIM3->CCR3 to modify PWM duty cycle

// TASK5: Variables to track current waveform
volatile uint8_t current_waveform = 0; // 0=sine, 1=saw, 2=triangle, 3=piano, 4=guitar, 5=drum
volatile uint32_t last_button_press = 0; // for debouncing

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void EXTI0_IRQHandler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // TODO: Start TIM3 in PWM mode on channel 3
  init_LCD();
  HAL_Delay(10);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  // TODO: Start TIM2 in Output Compare (OC) mode on channel 1
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);
  // TODO: Start DMA in IT mode on TIM2->CH1. Source is LUT and Dest is TIM3->CCR3; start with Sine LUT
  HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t)Sin_LUT, DestAddress, NS);
  // TODO: Write current waveform to LCD(Sine is the first waveform)
  lcd_command(CLEAR);
  lcd_putstring("Wave: Sine");
  // TODO: Enable DMA (start transfer from LUT to CCR)
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = TIM2_Ticks -1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* TIM2_CH1 DMA Init */
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_tim2_ch1.Instance = DMA1_Stream5;
  hdma_tim2_ch1.Init.Channel = DMA_CHANNEL_3;         // TIM2_CH1 is on channel 3
  hdma_tim2_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH; // Memory -> TIM3->CCR3
  hdma_tim2_ch1.Init.PeriphInc = DMA_PINC_DISABLE;    // Peripheral address fixed
  hdma_tim2_ch1.Init.MemInc = DMA_MINC_ENABLE;        // Memory address increments
  hdma_tim2_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_tim2_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;            // Repeat LUT automatically
  hdma_tim2_ch1.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_tim2_ch1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_tim2_ch1) != HAL_OK)
  {
      Error_Handler();
  }

  /* Link DMA handle to TIM2 handle */
  __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_CC1], hdma_tim2_ch1);
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
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
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // -------------------------------
  // LCD pins configuration
  // -------------------------------
  // Configure PC14 (RS) and PC15 (E) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Configure PB8 (D4) and PB9 (D5) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configure PA12 (D6) and PA15 (D7) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Set all LCD pins LOW initially
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_RESET);


  // -------------------------------
  // Button0 configuration (PA0)
  // -------------------------------
  GPIO_InitStruct.Pin = Button0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // Interrupt on rising edge
  GPIO_InitStruct.Pull = GPIO_PULLUP;         // Use pull-up resistor
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Enable and set EXTI line 0 interrupt priority
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void EXTI0_IRQHandler(void){

	// TODO: Debounce using HAL_GetTick()
  uint32_t current_time = HAL_GetTick();
if (current_time - last_button_press < 200){
  HAL_GPIO_EXTI_IRQHandler(Button0_Pin); // clear interrupt flags
  return; // ignore press
}
last_button_press = current_time;
	// TODO: Disable DMA transfer and abort IT, then start DMA in IT mode with new LUT and re-enable transfer
	// HINT: Consider using C's "switch" function to handle LUT changes
    __HAL_TIM_DISABLE_DMA(&htim2, TIM_DMA_CC1);
    
    // Abort the current DMA transfer
    HAL_DMA_Abort_IT(&hdma_tim2_ch1);
    
    // Move to next waveform (cycle through 0-5)
    current_waveform = (current_waveform + 1) % 6;

    uint32_t *selected_LUT;
    
    switch(current_waveform) {
        case 0:
            selected_LUT = Sin_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Sine");
            break;
        case 1:
            selected_LUT = Saw_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Sawtooth");
            break;
        case 2:
            selected_LUT = Triangle_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Triangle");
            break;
        case 3:
            selected_LUT = Piano_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Piano");
            break;
        case 4:
            selected_LUT = Guitar_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Guitar");
            break;
        case 5:
            selected_LUT = Drum_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Drum");
            break;
        default:
            selected_LUT = Sin_LUT;
            lcd_command(CLEAR);
            lcd_putstring("Sine");
            break;
    }
    // Start DMA with new LUT
    HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t)selected_LUT, DestAddress, NS);
    
    // Re-enable DMA transfer
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);
    
    HAL_GPIO_EXTI_IRQHandler(Button0_Pin); // Clear interrupt flags


	HAL_GPIO_EXTI_IRQHandler(Button0_Pin); // Clear interrupt flags
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
#ifdef USE_FULL_ASSERT
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
