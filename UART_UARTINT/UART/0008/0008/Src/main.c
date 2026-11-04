#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"

USART_Handle_t USART2Handle;


/*
 * USART2 GPIO initialization
 *
 * PA2 -> USART2_TX
 * PA3 -> USART2_RX
 *
 * Alternate Function 7
 */
void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART2_GPIO;

    /* Enable GPIOA clock */
    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART2_GPIO.pGPIOx = GPIOA;

    /* PA2 - USART2 TX */
    USART2_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 7;

    GPIO_Init(&USART2_GPIO);

    /* PA3 - USART2 RX */
    USART2_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;

    GPIO_Init(&USART2_GPIO);
}


/*
 * USART2 initialization
 */
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

    /*
     * USART_Init() already enables the USART2 peripheral clock.
     */
    USART_Init(&USART2Handle);

    /* Enable USART2 */
    USART_PeripheralControl(USART2, ENABLE);
}


int main(void)
{
    char message[] = "This is USART program of week 4.\r\n";

    /* Initialize GPIOA for USART2 */
    USART2_GPIO_Init();

    /* Initialize USART2 */
    USART2_Init();

    while (1)
    {
        USART_SendData(&USART2Handle,
                       (uint8_t *)message,
                       sizeof(message) - 1);

        /* Delay */
        for (volatile uint32_t i = 0; i < 1000000; i++);
    }
}
