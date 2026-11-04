#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#include <stdio.h>
#include <stdint.h>

ADC_Handle_t ADC1Handle;
USART_Handle_t USART2Handle;

/* USART2 GPIO: PA2 = TX, PA3 = RX */
void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART2_GPIO = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART2_GPIO.pGPIOx = GPIOA;

    USART2_GPIO.GPIO_PinConfig.GPIO_PinNumber =
        GPIO_PIN_NO_2;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinMode =
        GPIO_MODE_ALTFN;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinSpeed =
        GPIO_SPEED_FAST;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl =
        GPIO_PIN_PU;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinOPType =
        GPIO_OP_TYPE_PP;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 7;

    GPIO_Init(&USART2_GPIO);
}

/* USART2 initialization */
void USART2_Init(void)
{
    USART2Handle.pUSARTx = USART2;

    USART2Handle.USART_Config.USART_Baud =
        USART_STD_BAUD_115200;
    USART2Handle.USART_Config.USART_Mode =
        USART_MODE_ONLY_TX;
    USART2Handle.USART_Config.USART_WordLength =
        USART_WORDLEN_8BITS;
    USART2Handle.USART_Config.USART_NoOfStopBits =
        USART_STOPBITS_1;
    USART2Handle.USART_Config.USART_ParityControl =
        USART_PARITY_DISABLE;
    USART2Handle.USART_Config.USART_HWFlowControl =
        USART_HW_FLOW_CTRL_NONE;

    USART_Init(&USART2Handle);
    USART_PeripheralControl(USART2, ENABLE);
}

/* PA0 = ADC1 Channel 0 */
void ADC1_GPIO_Init(void)
{
    GPIO_Handle_t ADC_GPIO = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    ADC_GPIO.pGPIOx = GPIOA;
    ADC_GPIO.GPIO_PinConfig.GPIO_PinNumber =
        GPIO_PIN_NO_0;
    ADC_GPIO.GPIO_PinConfig.GPIO_PinMode =
        GPIO_MODE_ANALOG;
    ADC_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl =
        GPIO_NO_PUPD;

    GPIO_Init(&ADC_GPIO);
}

/* ADC1 initialization */
void ADC1_Init(void)
{
    ADC1Handle.pADCx = ADC1;

    ADC1Handle.ADC_Config.ADC_ClkPreScaler =
        ADC_CLK_DIV2;
    ADC1Handle.ADC_Config.ADC_Resolution =
        ADC_RES_12BITS;
    ADC1Handle.ADC_Config.ADC_ScanMode =
        ADC_SCAN_DISABLE;
    ADC1Handle.ADC_Config.ADC_DataAlignment =
        ADC_ALIGN_RIGHT;
    ADC1Handle.ADC_Config.ADC_ConvMode =
        ADC_CONV_SINGLE;
    ADC1Handle.ADC_Config.ADC_NumOfConversion = 1;

    /* Long sampling time for stable readings */
    ADC1Handle.ADC_Config.ADC_SamplingTime =
        ADC_SAMPLE_480CYCLES;

    ADC_Init(&ADC1Handle);
    ADC_PeripheralControl(ADC1, ENABLE);
}

/* Simple delay */
void delay(void)
{
    for (volatile uint32_t i = 0; i < 500000; i++);
}

int main(void)
{
    uint16_t adc_value;
    uint32_t voltage_mV;

    char message[100];
    int message_length;

    USART2_GPIO_Init();
    USART2_Init();

    ADC1_GPIO_Init();
    ADC1_Init();

    while (1)
    {
        /* Read potentiometer from PA0 */
        adc_value = ADC_ReadChannel(
            &ADC1Handle,
            ADC_CHANNEL_0
        );

        /* Convert ADC reading into millivolts */
        voltage_mV = ADC_CountsToMillivolts(
            &ADC1Handle,
            adc_value
        );

        /* Display ADC value and voltage */
        message_length = sprintf(
            message,
            "ADC = %u, Voltage = %lu mV\r\n",
            (unsigned int)adc_value,
            (unsigned long)voltage_mV
        );

        USART_SendData(
            &USART2Handle,
            (uint8_t *)message,
            (uint32_t)message_length
        );

        delay();
    }
}
