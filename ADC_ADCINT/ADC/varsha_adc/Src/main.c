#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#include <stdio.h>
#include <string.h>


ADC_Handle_t ADC1Handle;
USART_Handle_t USART2Handle;


/*----------------------------------------------------------
 * USART2 GPIO Initialization
 *
 * PA2 -> USART2_TX
 * PA3 -> USART2_RX
 * AF7
 *----------------------------------------------------------*/
void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART2_GPIO;

    /* Enable GPIOA clock */
    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART2_GPIO.pGPIOx = GPIOA;

    /* PA2 -> USART2 TX */
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

    USART2_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode =
        7;

    GPIO_Init(&USART2_GPIO);


    /* PA3 -> USART2 RX */
    USART2_GPIO.GPIO_PinConfig.GPIO_PinNumber =
        GPIO_PIN_NO_3;

    GPIO_Init(&USART2_GPIO);
}


/*----------------------------------------------------------
 * USART2 Initialization
 *----------------------------------------------------------*/
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


/*----------------------------------------------------------
 * ADC1 Initialization
 *----------------------------------------------------------*/
void ADC1_Init(void)
{
    ADC1Handle.pADCx = ADC1;

    /* ADC clock = PCLK2 / 2 */
    ADC1Handle.ADC_Config.ADC_ClkPreScaler =
        ADC_CLK_DIV2;

    /* 12-bit ADC */
    ADC1Handle.ADC_Config.ADC_Resolution =
        ADC_RES_12BITS;

    /* Disable scan mode */
    ADC1Handle.ADC_Config.ADC_ScanMode =
        ADC_SCAN_DISABLE;

    /* Right aligned */
    ADC1Handle.ADC_Config.ADC_DataAlignment =
        ADC_ALIGN_RIGHT;

    /* Single conversion */
    ADC1Handle.ADC_Config.ADC_ConvMode =
        ADC_CONV_SINGLE;

    /* One conversion */
    ADC1Handle.ADC_Config.ADC_NumOfConversion =
        1;

    /*
     * Long sampling time.
     * Required for the internal temperature sensor.
     */
    ADC1Handle.ADC_Config.ADC_SamplingTime =
        ADC_SAMPLE_480CYCLES;

    /* Initialize ADC1 */
    ADC_Init(&ADC1Handle);

    /* Enable internal temperature sensor */
    ADC_TempSensorVrefintControl(ENABLE);

    /* Enable ADC1 */
    ADC_PeripheralControl(ADC1, ENABLE);
}


/*----------------------------------------------------------
 * Simple Delay
 *----------------------------------------------------------*/
void delay(void)
{
    for(volatile uint32_t i = 0;
            i < 1000000;
            i++);
}


/*----------------------------------------------------------
 * MAIN
 *----------------------------------------------------------*/
int main(void)
{
    uint16_t adc_value;
    uint32_t voltage_mV;

    int32_t temperature_x100;

    uint32_t temperature_integer;
    uint32_t temperature_decimal;

    char message[100];

    int message_length;


    /* Initialize USART2 */
    USART2_GPIO_Init();
    USART2_Init();


    /* Initialize ADC1 */
    ADC1_Init();


    while(1)
    {
        /*----------------------------------------------
         * Read internal temperature sensor
         *
         * STM32F446:
         * Temperature sensor = ADC Channel 18
         *----------------------------------------------*/
        adc_value =
            ADC_ReadChannel(
                &ADC1Handle,
                ADC_CHANNEL_TEMPSENSOR
            );


        /*----------------------------------------------
         * Convert ADC value to millivolts
         *----------------------------------------------*/
        voltage_mV =
            ADC_CountsToMillivolts(
                &ADC1Handle,
                adc_value
            );


        /*----------------------------------------------
         * Calculate temperature
         *
         * Approximate equation:
         *
         * Temperature = (Voltage - 760) / 2.5 + 25
         *
         * We calculate temperature x100 so that
         * we can display two decimal places without %f.
         *
         * 1 / 2.5 = 0.4
         *
         * Therefore:
         *
         * Temperature x100 =
         * (Voltage - 760) x 40 + 2500
         *----------------------------------------------*/
        temperature_x100 =
            ((int32_t)voltage_mV - 760) * 40
            + 2500;


        /*----------------------------------------------
         * Separate integer and decimal parts
         *----------------------------------------------*/
        if(temperature_x100 >= 0)
        {
            temperature_integer =
                temperature_x100 / 100;

            temperature_decimal =
                temperature_x100 % 100;
        }
        else
        {
            temperature_integer = 0;
            temperature_decimal = 0;
        }


        /*----------------------------------------------
         * Create message
         *----------------------------------------------*/
        message_length =
            sprintf(
                message,
                "ADC = %u, Voltage = %lu mV, "
                "Temperature = %lu.%02lu C\r\n",

                adc_value,
                voltage_mV,
                temperature_integer,
                temperature_decimal
            );


        /*----------------------------------------------
         * Send ONLY the actual message length
         *
         * This is important.
         *
         * Do NOT use:
         *
         * sizeof(message) - 1
         *
         * because message is a 100-byte array.
         *----------------------------------------------*/
        USART_SendData(
            &USART2Handle,
            (uint8_t *)message,
            message_length
        );


        /* Wait before next reading */
        delay();
    }
}
