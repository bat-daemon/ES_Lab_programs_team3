/*
 * 007freertos_task_creation.c
 *
 *  Created on: Aug 30, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Minimal FreeRTOS example - create two tasks and start the
 *            scheduler. One task toggles the on-board LED (LD2, PA5),
 *            the other prints a heartbeat message over semihosting.
 *
 *  This uses the NATIVE FreeRTOS API (xTaskCreate, vTaskDelay, etc.),
 *  which is what you get if you add FreeRTOS as Middleware in
 *  STM32CubeMX/CubeIDE WITHOUT selecting the CMSIS-RTOS v2 wrapper.
 *  If your project uses CMSIS-RTOS v2 instead, the equivalent calls are
 *  osThreadNew(), osDelay(), etc. - noted inline where relevant.
 *
 *  Project setup (STM32CubeIDE):
 *      1. Add FreeRTOS as Middleware (Device Configuration Tool -> Middleware).
 *      2. Interface: choose CMSIS_V1 if you want the native API shown here,
 *         or leave CMSIS_V2 and translate the calls as noted in comments.
 *      3. This pulls in FreeRTOSConfig.h, and the CMSIS_RTOS/FreeRTOS
 *         sources, and wires SysTick to the RTOS tick automatically.
 *      4. GPIO driver calls below assume your existing GPIO_Init etc.
 *         are still linked in; swap for HAL_GPIO_* if your project uses
 *         HAL instead of your own driver.
 */

#include <stdio.h>                    // for printf, routed via semihosting
#include "stm32f446xx.h"              // your GPIO driver header

#include "FreeRTOS.h"                 // must be included before task.h
#include "task.h"                     // xTaskCreate, vTaskDelay, vTaskStartScheduler

extern void initialise_monitor_handles(void);   // semihosting init, same as earlier labs

/*
 * Task handles.
 * Not required for this simple example, but keeping them lets you later
 * suspend, resume or delete a task by name instead of guessing.
 */
TaskHandle_t xLedTaskHandle   = NULL;
TaskHandle_t xPrintTaskHandle = NULL;

/*
 * Stack sizes are in WORDS, not bytes, on an ARM Cortex-M port.
 * configMINIMAL_STACK_SIZE (from FreeRTOSConfig.h) is the smallest sane
 * value; give a task more if it calls printf or uses local arrays.
 */
#define LED_TASK_STACK_SIZE     configMINIMAL_STACK_SIZE
#define PRINT_TASK_STACK_SIZE   (configMINIMAL_STACK_SIZE * 2U)   // printf needs extra stack

/*
 * Priorities: higher NUMBER = higher priority in FreeRTOS, the opposite
 * of NVIC interrupt priority where lower number is more urgent.
 * tskIDLE_PRIORITY (0) is reserved for the idle task - never use it here.
 */
#define LED_TASK_PRIORITY       (tskIDLE_PRIORITY + 2U)
#define PRINT_TASK_PRIORITY     (tskIDLE_PRIORITY + 1U)


/*********************************************************************
 * @fn                - GPIO_LedInit
 * @brief             - Configures PA5 as a push-pull output for LD2
 * @return            - None
 * @Note              - Ordinary driver call; FreeRTOS has no opinion about
 *                      how a peripheral is initialised, only about how
 *                      tasks are scheduled once the system is running.
 *********************************************************************/
void GPIO_LedInit(void)
{
    GPIO_Handle_t led;

    led.pGPIOx = GPIOA;

    led.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;
    led.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    led.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOA, ENABLE);

    GPIO_Init(&led);
}


/*********************************************************************
 * @fn                - vLedTask
 * @brief             - Toggles LD2 every 500 ms, forever
 *
 * @param[in]         - pvParameters : unused here, but every FreeRTOS task
 *                      function must accept this signature exactly
 *
 * @return            - None. A task function must never return - it either
 *                      loops forever, as here, or calls vTaskDelete(NULL)
 *                      to remove itself.
 *
 * @Note              - vTaskDelay() BLOCKS this task only. The scheduler is
 *                      free to run other tasks - including vPrintTask below -
 *                      during that time. This is the entire point of an RTOS:
 *                      the CPU is never idle-waiting inside one function the
 *                      way your earlier bare-metal delay() loops made it wait.
 *********************************************************************/
void vLedTask(void *pvParameters)
{
    (void)pvParameters;                                          // silence an unused-parameter warning

    for(;;)                                                       // idiomatic FreeRTOS task loop
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);               // flip the LED state

        vTaskDelay(pdMS_TO_TICKS(500));                           // sleep 500 ms; other tasks run meanwhile
                                                                    // CMSIS-RTOS v2 equivalent: osDelay(500)
    }
}


/*********************************************************************
 * @fn                - vPrintTask
 * @brief             - Prints a heartbeat message every 1 second, forever
 *
 * @param[in]         - pvParameters : unused
 *
 * @return            - None, never returns
 *
 * @Note              - Runs independently of vLedTask. Because the two
 *                      tasks have different periods (500 ms and 1000 ms),
 *                      you will see the LED toggle roughly twice for every
 *                      one message printed - proof both are running
 *                      concurrently rather than one after the other.
 *********************************************************************/
void vPrintTask(void *pvParameters)
{
    (void)pvParameters;

    uint32_t beat = 0;

    for(;;)
    {
        printf("Heartbeat #%lu\r\n", (unsigned long)beat);       // semihosting output to the IDE console
        beat++;

        vTaskDelay(pdMS_TO_TICKS(1000));                          // sleep 1 second
    }
}


int main(void)
{
    initialise_monitor_handles();                                 // must run before any printf

    printf("=== FreeRTOS task creation demo ===\r\n");
    printf("Creating tasks...\r\n");

    GPIO_LedInit();                                                // bring up the LED pin before the scheduler starts

    /*
     * xTaskCreate() arguments, in order:
     *   1. Task function            - the function implementing the task
     *   2. Task name                - for debuggers/trace tools, max
     *                                  configMAX_TASK_NAME_LEN characters
     *   3. Stack depth, in words    - not bytes
     *   4. Parameter to pass in     - becomes pvParameters inside the task
     *   5. Priority                 - 0 to configMAX_PRIORITIES - 1
     *   6. Task handle output       - pass NULL if you never need it
     *
     * Returns pdPASS on success, or an error code (commonly
     * errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) if the heap could not supply
     * the requested stack and TCB. ALWAYS check the return value - a
     * failed task creation is silent otherwise.
     */
    if(xTaskCreate(vLedTask,
                   "LED_Task",
                   LED_TASK_STACK_SIZE,
                   NULL,
                   LED_TASK_PRIORITY,
                   &xLedTaskHandle) != pdPASS)
    {
        printf("ERROR: LED task creation failed\r\n");
        while(1);                                                  // stop here; something is badly wrong
    }

    if(xTaskCreate(vPrintTask,
                   "Print_Task",
                   PRINT_TASK_STACK_SIZE,
                   NULL,
                   PRINT_TASK_PRIORITY,
                   &xPrintTaskHandle) != pdPASS)
    {
        printf("ERROR: Print task creation failed\r\n");
        while(1);
    }

    printf("Tasks created. Starting scheduler...\r\n");

    vTaskStartScheduler();                                         // hands control to FreeRTOS - does not return
                                                                     // CMSIS-RTOS v2 equivalent: osKernelStart()

    /*
     * Execution reaches here ONLY if the scheduler could not start -
     * almost always because the heap (configTOTAL_HEAP_SIZE in
     * FreeRTOSConfig.h) was too small to allocate the idle task and
     * timer task. It is not reached in normal operation.
     */
    printf("ERROR: scheduler returned - increase configTOTAL_HEAP_SIZE\r\n");

    while(1);                                                       // trap here; nothing further can run correctly

    return 0;                                                       // never reached
}
