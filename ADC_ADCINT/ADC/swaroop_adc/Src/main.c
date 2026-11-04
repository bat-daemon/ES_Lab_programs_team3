
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

ADC_Handle_t hadc;
USART_Handle_t huart;

/* PA0 = potentiometer input */
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

/* PA8 = LED output, using GPIO instead of timer PWM */
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

    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET);
}

/* USART2 TX = PA2 */
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
    huart.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(&huart);
    USART_PeripheralControl(USART2, ENABLE);
}

/* ADC1 initialization */
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

/* Software PWM delay */
static void PWM_Delay(volatile uint32_t count)
{
    while (count-- > 0U)
    {
        /* Wait */
    }
}

/*
 * Software PWM:
 * brightness = 0    -> LED always OFF
 * brightness = 1000 -> LED always ON
 */
void Software_PWM(uint32_t brightness)
{
    const uint32_t period = 1000U;

    if (brightness > period)
    {
        brightness = period;
    }

    if (brightness == 0U)
    {
        GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET);
        PWM_Delay(1000U);
    }
    else if (brightness >= period)
    {
        GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_SET);
        PWM_Delay(1000U);
    }
    else
    {
        GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_SET);
        PWM_Delay(brightness * 10U);

        GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_8, GPIO_PIN_RESET);
        PWM_Delay((period - brightness) * 10U);
    }
}

int main(void)
{
    uint16_t adc;
    uint32_t brightness;
    uint32_t voltage;
    char text[100];
    int len;

    Pot_Init();
    LED_Init();
    UART_GPIO_Init();
    UART_Init();
    ADC_Init_Pot();

    while (1)
    {
        /* Read potentiometer through ADC */
        adc = ADC_ReadChannel(&hadc, ADC_CHANNEL_0);

        /* Convert ADC range 0-4095 to brightness 0-1000 */
        brightness = ((uint32_t)adc * 1000U) / 4095U;

        /* Convert ADC reading to millivolts */
        voltage = ADC_CountsToMillivolts(&hadc, adc);

        /* Print ADC, voltage and brightness */
        len = sprintf(text,
                      "ADC=%u Voltage=%lu mV Brightness=%lu%%\r\n",
                      (unsigned int)adc,
                      (unsigned long)voltage,
                      (unsigned long)((brightness * 100U) / 1000U));

        USART_SendData(&huart, (uint8_t *)text, (uint32_t)len);

        /* Control LED brightness using software PWM */
        Software_PWM(brightness);
    }
}
