#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t myQueue;

void ProducerTask(void *argument)
{
    int number = 0;

    while (1)
    {
        number++;

        // Send number to the queue
        xQueueSend(myQueue, &number, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ConsumerTask(void *argument)
{
    int receivedNumber;

    while (1)
    {
        // Wait until data is available
        xQueueReceive(myQueue, &receivedNumber, portMAX_DELAY);

        // Toggle STM32F446RE LED on PA5
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // Queue can store five integers
    myQueue = xQueueCreate(5, sizeof(int));

    // Create producer and consumer tasks
    xTaskCreate(ProducerTask, "Producer", 128, NULL, 2, NULL);
    xTaskCreate(ConsumerTask, "Consumer", 128, NULL, 1, NULL);

    // Start FreeRTOS
    vTaskStartScheduler();

    while (1)
    {
    }
}