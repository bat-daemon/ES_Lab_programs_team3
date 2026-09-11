/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Selection Sort with USART2 output
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define ARRAY_SIZE 8U

volatile int32_t numbers[ARRAY_SIZE] = {64, 25, 12, 22, 11, 90, 5, 38};
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
static void SelectionSort(volatile int32_t array[], uint32_t size);
static void PrintArray(const char *title,
                       const volatile int32_t array[],
                       uint32_t size);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

static void SelectionSort(volatile int32_t array[], uint32_t size)
{
  for (uint32_t i = 0; i + 1 < size; i++)
  {
    uint32_t min_idx = i;

    for (uint32_t j = i + 1; j < size; j++)
    {
      if (array[j] < array[min_idx])
      {
        min_idx = j;
      }
    }

    if (min_idx != i)
    {
      int32_t temp = array[i];
      array[i] = array[min_idx];
      array[min_idx] = temp;
    }
  }
}

static void PrintArray(const char *title,
                       const volatile int32_t array[],
                       uint32_t size)
{
  char message[128];
  uint32_t i;
  int length;

  length = snprintf(message, sizeof(message), "%s", title);
  HAL_UART_Transmit(&huart2, (uint8_t *)message,
                    (uint16_t)length, HAL_MAX_DELAY);

  for (i = 0U; i < size; i++)
  {
    length = snprintf(message, sizeof(message), "%ld ",
                      (long)array[i]);

    HAL_UART_Transmit(&huart2, (uint8_t *)message,
                      (uint16_t)length, HAL_MAX_DELAY);
  }

  HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2U, HAL_MAX_DELAY);
}

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  PrintArray("Unsorted array: ", numbers, ARRAY_SIZE);

  SelectionSort(numbers, ARRAY_SIZE);

  PrintArray("Sorted array:   ", numbers, ARRAY_SIZE);

  /* USER CODE END 2 */

  while (1)
  {
    __NOP();
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

void Error_Handler(void)
{
  __disable_irq();

  while (1)
  {
  }
}
