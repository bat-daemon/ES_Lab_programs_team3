#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#include <stdint.h>
#include <stdio.h>

ADC_Handle_t hadc = {0};
USART_Handle_t huart = {0};

volatile uint8_t mode = 0U;
volatile uint8_t button_event = 0U;


/* PA0: Potentiometer ADC input */
void Pot_Init(void)
{
    GPIO_Handle_t pin = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    pin.pGPIOx = GPIOA;
    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&pin);
}


/* PA8: LED output */
void LED_Init(void)
{
    GPIO_Handle_t pin = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    pin.pGPIOx = GPIOA;
    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&pin);

    GPIO_WriteToOutputPin(
        GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET
    );
}


/* USART2 TX: PA2, AF7 */
void UART_GPIO_Init(void)
{
    GPIO_Handle_t pin = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    pin.pGPIOx = GPIOA;
    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode = 7U;

    GPIO_Init(&pin);
}


/* USART2 initialization */
void UART_Init(void)
{
    huart.pUSARTx = USART2;

    huart.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    huart.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
    huart.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
    huart.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    huart.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    huart.USART_Config.USART_HWFlowControl =
        USART_HW_FLOW_CTRL_NONE;

    USART_Init(&huart);
    USART_PeripheralControl(USART2, ENABLE);
}


/* ADC1: PA0, channel 0 */
void ADC_Init_Pot(void)
{
    hadc.pADCx = ADC1;

    hadc.ADC_Config.ADC_ClkPreScaler = ADC_CLK_DIV2;
    hadc.ADC_Config.ADC_Resolution = ADC_RES_12BITS;
    hadc.ADC_Config.ADC_ScanMode = ADC_SCAN_DISABLE;
    hadc.ADC_Config.ADC_DataAlignment = ADC_ALIGN_RIGHT;
    hadc.ADC_Config.ADC_ConvMode = ADC_CONV_SINGLE;
    hadc.ADC_Config.ADC_NumOfConversion = 1U;
    hadc.ADC_Config.ADC_SamplingTime = ADC_SAMPLE_480CYCLES;

    ADC_Init(&hadc);
    ADC_PeripheralControl(ADC1, ENABLE);
}


/*
 * PC13: User button interrupt.
 * Nucleo-F446RE button:
 * Released = LOW
 * Pressed  = HIGH
 * Pressing generates a rising edge.
 */
void Button_EXTI_Init(void)
{
    GPIO_Handle_t pin = {0};

    GPIO_PeriClockControl(GPIOC, ENABLE);

    pin.pGPIOx = GPIOC;
    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_RT;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    /* Configure PC13 and EXTI using the GPIO driver. */
    GPIO_Init(&pin);

    /* Clear any previous pending interrupt. */
    GPIO_IRQHandling(GPIO_PIN_NO_13);

    /* Enable the EXTI lines 10-15 interrupt in the NVIC. */
    GPIO_IRQConfig(
        IRQ_NO_EXTI15_10,
        NVIC_IRQ_PRI15,
        ENABLE
    );
}


/* EXTI interrupt handler for GPIO pins 10 through 15 */
void EXTI15_10_IRQHandler(void)
{
    /*
     * Check that EXTI13 caused the interrupt because
     * EXTI15_10 is shared by pins 10 through 15.
     */
    if ((EXTI->PR & (1U << 13U)) != 0U)
    {
        /* Clear the PC13 pending interrupt using the driver. */
        GPIO_IRQHandling(GPIO_PIN_NO_13);

        /* Cycle through modes 0, 1 and 2. */
        mode++;

        if (mode >= 3U)
        {
            mode = 0U;
        }

        button_event = 1U;
    }
}


/* Simple software delay without __NOP() */
static void Delay(volatile uint32_t count)
{
    while (count-- > 0U)
    {
        /* Wait */
    }
}


/* Software PWM: brightness range 0-1000 */
void Software_PWM(uint32_t brightness)
{
    const uint32_t period = 1000U;

    if (brightness > period)
    {
        brightness = period;
    }

    if (brightness == 0U)
    {
        GPIO_WriteToOutputPin(
            GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET
        );

        Delay(10000U);
    }
    else if (brightness >= period)
    {
        GPIO_WriteToOutputPin(
            GPIOA, GPIO_PIN_NO_8, GPIO_PIN_SET
        );

        Delay(10000U);
    }
    else
    {
        GPIO_WriteToOutputPin(
            GPIOA, GPIO_PIN_NO_8, GPIO_PIN_SET
        );

        Delay(brightness * 10U);

        GPIO_WriteToOutputPin(
            GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET
        );

        Delay((period - brightness) * 10U);
    }
}


int main(void)
{
    uint16_t adc;
    uint32_t brightness;

    uint8_t current_mode;
    uint8_t report_event;

    char text[100];
    int len;

    Pot_Init();
    LED_Init();
    UART_GPIO_Init();
    UART_Init();
    ADC_Init_Pot();
    Button_EXTI_Init();

    /*
     * No __enable_irq() call here.
     * The startup code normally enables interrupts globally.
     */

    while (1)
    {
        /* Read the potentiometer using the ADC driver. */
        adc = ADC_ReadChannel(&hadc, ADC_CHANNEL_0);

        /*
         * Read the current mode and button event.
         * No global interrupt-control intrinsics are used.
         */
        current_mode = mode;
        report_event = button_event;
        button_event = 0U;

        /* Mode 0: Potentiometer controls brightness. */
        if (current_mode == 0U)
        {
            brightness =
                ((uint32_t)adc * 1000U) / 4095U;
        }
        /* Mode 1: Fixed 50% brightness. */
        else if (current_mode == 1U)
        {
            brightness = 500U;
        }
        /* Mode 2: LED OFF. */
        else
        {
            brightness = 0U;
        }

        /* Print a message only when the button is pressed. */
        if (report_event != 0U)
        {
            if (current_mode == 0U)
            {
                len = sprintf(
                    text,
                    "Mode 0: Potentiometer control\r\n"
                );
            }
            else if (current_mode == 1U)
            {
                len = sprintf(
                    text,
                    "Mode 1: Fixed 50%% brightness\r\n"
                );
            }
            else
            {
                len = sprintf(
                    text,
                    "Mode 2: LED OFF\r\n"
                );
            }

            USART_SendData(
                &huart,
                (uint8_t *)text,
                (uint32_t)len
            );
        }

        /* Update LED brightness. */
        Software_PWM(brightness);
    }
}
