#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t buttonSemaphore;

void ButtonTask(void *argument)
{
    while (1)
    {
        /* Wait for button interrupt */
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE)
        {
            /* Toggle onboard LED: PA5 */
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
}

/* Called when the button EXTI interrupt occurs */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_13)       // User button: PC13
    {
        xSemaphoreGiveFromISR(
            buttonSemaphore,
            &higherPriorityTaskWoken
        );

        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* Create an empty binary semaphore */
    buttonSemaphore = xSemaphoreCreateBinary();

    configASSERT(buttonSemaphore != NULL);

    /* Create button-handling task */
    xTaskCreate(
        ButtonTask,
        "ButtonTask",
        128,
        NULL,
        2,
        NULL
    );

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    while (1)
    {
    }
}