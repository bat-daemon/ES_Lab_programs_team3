
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#define LED_PIN             GPIO_PIN_NO_8
#define BUTTON_PIN          GPIO_PIN_NO_13

#define ADC_MAX_VALUE       4095U
#define ADC_HALF_SCALE      2048U

#define FAST_DELAY_MS       50U
#define MIN_DELAY_MS        50U
#define MAX_DELAY_MS        500U

volatile uint8_t button_event = 0U;
volatile uint8_t mode = 0U;

ADC_Handle_t hadc = {0};
USART_Handle_t huart = {0};

/* Approximate software delay */
static void delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    volatile uint32_t j;

    for (i = 0U; i < ms; i++)
    {
        for (j = 0U; j < 4000U; j++)
        {
            /* Wait */
        }
    }
}

/* PA8: LED output */
static void LED_Init(void)
{
    GPIO_Handle_t led = {0};

    led.pGPIOx = GPIOA;
    led.GPIO_PinConfig.GPIO_PinNumber = LED_PIN;
    led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&led);

    GPIO_WriteToOutputPin(GPIOA, LED_PIN, GPIO_PIN_RESET);
}

/* PC13: button interrupt */
static void Button_Init(void)
{
    GPIO_Handle_t button = {0};

    button.pGPIOx = GPIOC;
    button.GPIO_PinConfig.GPIO_PinNumber = BUTTON_PIN;
    button.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_RT;
    button.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    button.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOC, ENABLE);
    GPIO_Init(&button);

    GPIO_IRQHandling(BUTTON_PIN);
    GPIO_IRQConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15, ENABLE);
}

/* PA2: USART2 TX, AF7 */
static void UART_GPIO_Init(void)
{
    GPIO_Handle_t uart_pin = {0};

    uart_pin.pGPIOx = GPIOA;
    uart_pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    uart_pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    uart_pin.GPIO_PinConfig.GPIO_PinAltFunMode = 7U;
    uart_pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    uart_pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    uart_pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&uart_pin);
}

/* USART2 initialization */
static void UART_Init_App(void)
{
    huart.pUSARTx = USART2;

    huart.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    huart.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
    huart.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    huart.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
    huart.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    huart.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_PeriClockControl(USART2, ENABLE);
    USART_Init(&huart);
}

/*
 * ADC configuration fields below are based on the names
 * reported by your compiler. Verify ADC_ConvMode = 0U
 * and ADC_ReadChannel() against your ADC driver source.
 */
static void ADC_Init_App(void)
{
    hadc.pADCx = ADC1;

    hadc.ADC_Config.ADC_ConvMode = 0U;
    hadc.ADC_Config.ADC_DataAlignment = ADC_ALIGN_RIGHT;
    hadc.ADC_Config.ADC_NumOfConversion = 1U;

    ADC_PeriClockControl(ADC1, ENABLE);
    ADC_Init(&hadc);
    ADC_PeripheralControl(ADC1, ENABLE);
}

static uint32_t Read_Potentiometer(void)
{
    return ADC_ReadChannel(&hadc, ADC_CHANNEL_0);
}

/* Shared handler for EXTI lines 10–15 */
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & (1U << 13U)) != 0U)
    {
        GPIO_IRQHandling(BUTTON_PIN);
        button_event = 1U;
    }
}

int main(void)
{
    uint32_t adc_value;
    uint32_t delay_time;
    char message[100];

    LED_Init();
    Button_Init();
    UART_GPIO_Init();
    UART_Init_App();
    ADC_Init_App();

    while (1)
    {
        /* Button interrupt switches the mode */
        if (button_event != 0U)
        {
            button_event = 0U;
            mode ^= 1U;

            sprintf(message, "\r\nMode changed to %u\r\n",
                    (unsigned int)mode);

            /* Correct: your USART driver takes 3 arguments */
            USART_SendData(&huart, (uint8_t *)message,
                           (uint32_t)strlen(message));
        }

        adc_value = Read_Potentiometer();

        if (mode == 0U)
        {
            /* Mode 0: ADC controls blink speed */
            if (adc_value > ADC_MAX_VALUE)
            {
                adc_value = ADC_MAX_VALUE;
            }

            delay_time = MAX_DELAY_MS -
                ((adc_value * (MAX_DELAY_MS - MIN_DELAY_MS))
                 / ADC_MAX_VALUE);

            GPIO_ToggleOutputPin(GPIOA, LED_PIN);
            delay_ms(delay_time);
        }
        else
        {
            /* Mode 1: blink only above half-scale */
            if (adc_value > ADC_HALF_SCALE)
            {
                GPIO_ToggleOutputPin(GPIOA, LED_PIN);
                delay_ms(FAST_DELAY_MS);
            }
            else
            {
                GPIO_WriteToOutputPin(GPIOA, LED_PIN,
                                      GPIO_PIN_RESET);
                delay_ms(50U);
            }
        }

        sprintf(message, "ADC: %lu | Mode: %u\r\n",
                (unsigned long)adc_value,
                (unsigned int)mode);

        /* Correct: no USART_MAX_DELAY argument */
        USART_SendData(&huart, (uint8_t *)message,
                       (uint32_t)strlen(message));
    }
}
