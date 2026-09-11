/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Jump Search example
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN PV */
#define ARRAY_SIZE 10U

/* Array must be sorted for Jump Search. */
volatile int32_t numbers[ARRAY_SIZE] = {2, 5, 8, 12, 16, 23, 31, 42, 56, 78};

volatile int32_t search_key = 31;
volatile int32_t search_result = -1;
/* USER CODE END PV */

void SystemClock_Config(void);

static int32_t JumpSearch(const volatile int32_t array[],
                          uint32_t size,
                          int32_t key);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  /* USER CODE BEGIN 2 */

  /* search_result becomes 6 because numbers[6] is 31. */
  search_result = JumpSearch(numbers, ARRAY_SIZE, search_key);

  /* USER CODE END 2 */

  while (1)
  {
    /* Keeps a real instruction here for a breakpoint. */
    __NOP();
  }
}

/**
  * @brief  Searches a sorted array using Jump Search.
  * @param  array: sorted integer array
  * @param  size: number of elements
  * @param  key: value to find
  * @retval Index of key, or -1 if not found
  */
static int32_t JumpSearch(const volatile int32_t array[],
                          uint32_t size,
                          int32_t key)
{
  uint32_t jump = 1U;
  uint32_t start = 0U;
  uint32_t end;
  uint32_t i;

  /* Find jump size: ceil(sqrt(size)), without math.h. */
  while ((jump * jump) < size)
  {
    jump++;
  }

  end = jump;

  /* Jump between blocks. */
  while ((start < size) &&
         (array[((end < size) ? end : size) - 1U] < key))
  {
    start = end;
    end += jump;

    if (start >= size)
    {
      return -1;
    }
  }

  /* Search each value inside the matching block. */
  for (i = start; (i < end) && (i < size); i++)
  {
    if (array[i] == key)
    {
      return (int32_t)i;
    }
  }

  return -1;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
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

void Error_Handler(void)
{
  __disable_irq();

  while (1)
  {
  }
}
