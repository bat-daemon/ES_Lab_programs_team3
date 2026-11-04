
#include <stdint.h>
#include <stdio.h>

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

ADC_Handle_t hadc = {0};
USART_Handle_t huart = {0};

volatile uint8_t button_event = 0U;
volatile uint8_t threshold_mode = 0U;

/* =====================================================
   PA0 - Potentiometer ADC input
   ===================================================== */
static void Pot_Init(void)
{
    GPIO_Handle_t gpio = {0};

    gpio.pGPIOx = GPIOA;
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&gpio);
}

/* =====================================================
   PA8 - LED output
   ===================================================== */
static void LED_Init(void)
{
    GPIO_Handle_t gpio = {0};

    gpio.pGPIOx = GPIOA;
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&gpio);

    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET);
}

/* =====================================================
   PA2 - USART2 TX, Alternate Function 7
   ===================================================== */
static void UART_GPIO_Init(void)
{
    GPIO_Handle_t gpio = {0};

    gpio.pGPIOx = GPIOA;
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 7;
    gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&gpio);
}

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
    USART_PeripheralControl(USART2, ENABLE);
}

/* =====================================================
   ADC1 - PA0, single conversion, 12-bit default
   ===================================================== */
static void ADC_Init_Pot(void)
{
    hadc.pADCx = ADC1;

    /*
       ADC_ConvMode = 0 assumes the driver's zero value
       represents single conversion mode.

       The STM32 ADC reset default is 12-bit resolution.
       Verify the ADC_ConvMode definition in your driver
       header before relying on this setting.
    */
    hadc.ADC_Config.ADC_ConvMode = 0U;
    hadc.ADC_Config.ADC_DataAlignment = ADC_ALIGN_RIGHT;
    hadc.ADC_Config.ADC_NumOfConversion = 1U;

    ADC_PeriClockControl(ADC1, ENABLE);
    ADC_Init(&hadc);
    ADC_PeripheralControl(ADC1, ENABLE);
}

/* =====================================================
   PC13 - User button, rising-edge interrupt
   Nucleo-F446RE button is active HIGH
   ===================================================== */
static void Button_Init(void)
{
    GPIO_Handle_t gpio = {0};

    gpio.pGPIOx = GPIOC;
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_RT;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOC, ENABLE);
    GPIO_Init(&gpio);

    /* Clear a pending interrupt before enabling it */
    GPIO_IRQHandling(GPIO_PIN_NO_13);

    /* Your driver requires all three arguments */
    GPIO_IRQConfig(
        IRQ_NO_EXTI15_10,
        NVIC_IRQ_PRI15,
        ENABLE
    );
}

/* =====================================================
   EXTI interrupt handler for PC13
   Button toggles the threshold selection
   ===================================================== */
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & (1U << 13U)) != 0U)
    {
        GPIO_IRQHandling(GPIO_PIN_NO_13);

        threshold_mode ^= 1U;
        button_event = 1U;
    }
}

/* =====================================================
   Main application
   Mode 0: LED ON when voltage > 1.0 V
   Mode 1: LED ON when voltage > 2.0 V
   ===================================================== */
int main(void)
{
    uint16_t adc_value;
    uint32_t millivolts;
    uint32_t threshold;
    uint8_t current_mode;
    uint8_t report_event;

    char text[100];
    int len;

    Pot_Init();
    LED_Init();
    UART_GPIO_Init();
    UART_Init_App();
    ADC_Init_Pot();
    Button_Init();

    while (1)
    {
        /* Read the potentiometer */
        adc_value = ADC_ReadChannel(&hadc, ADC_CHANNEL_0);

        /* Convert ADC reading to millivolts */
        millivolts =
            ((uint32_t)adc_value * 3300U) / 4095U;

        /* Select the voltage threshold */
        current_mode = threshold_mode;

        if (current_mode == 0U)
        {
            threshold = 1000U;  /* 1.0 V */
        }
        else
        {
            threshold = 2000U;  /* 2.0 V */
        }

        /* Control LED according to the threshold */
        if (millivolts > threshold)
        {
            GPIO_WriteToOutputPin(
                GPIOA,
                GPIO_PIN_NO_8,
                GPIO_PIN_SET
            );
        }
        else
        {
            GPIO_WriteToOutputPin(
                GPIOA,
                GPIO_PIN_NO_8,
                GPIO_PIN_RESET
            );
        }

        /* Capture and clear the button event */
        report_event = button_event;
        button_event = 0U;

        /* Report threshold change */
        if (report_event != 0U)
        {
            len = sprintf(
                text,
                "\r\nThreshold changed to %lu mV\r\n",
                (unsigned long)threshold
            );

            USART_SendData(
                &huart,
                (uint8_t *)text,
                (uint32_t)len
            );
        }

        /* Report ADC value and LED status */
        len = sprintf(
            text,
            "ADC=%u Voltage=%lu mV Threshold=%lu mV LED=%s\r\n",
            adc_value,
            (unsigned long)millivolts,
            (unsigned long)threshold,
            (millivolts > threshold) ? "ON" : "OFF"
        );

        USART_SendData(
            &huart,
            (uint8_t *)text,
            (uint32_t)len
        );

        /* Small delay between reports */
        for (volatile uint32_t i = 0U; i < 100000U; i++)
        {
            /* Wait */
        }
    }
}
