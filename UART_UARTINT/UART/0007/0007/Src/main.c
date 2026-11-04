
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"

USART_Handle_t USART2Handle;
GPIO_Handle_t LED;

void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART_GPIO;

    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART_GPIO.pGPIOx = GPIOA;

    /* PA2: USART2 TX */
    USART_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    USART_GPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART_GPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    USART_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_NO_PUPD;
    USART_GPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 7;

    GPIO_Init(&USART_GPIO);

    /* PA3: USART2 RX */
    USART_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&USART_GPIO);
}

void LED_Init(void)
{
    GPIO_PeriClockControl(GPIOA, ENABLE);

    LED.pGPIOx = GPIOA;
    LED.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    LED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    LED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    LED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_NO_PUPD;
    LED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;

    GPIO_Init(&LED);
}

void USART2_Init(void)
{
    USART2Handle.pUSARTx = USART2;

    USART2Handle.USART_Config.USART_Baud =
        USART_STD_BAUD_115200;

    USART2Handle.USART_Config.USART_Mode =
        USART_MODE_TXRX;

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

void Send_Message(char *msg)
{
    uint32_t length = 0;

    while (msg[length] != '\0')
    {
        length++;
    }

    USART_SendData(&USART2Handle,
                   (uint8_t *)msg,
                   length);
}

int main(void)
{
    uint8_t command;

    USART2_GPIO_Init();
    LED_Init();
    USART2_Init();

    Send_Message("\r\nSTM32 USART LED Control\r\n");
    Send_Message("1 - LED ON\r\n");
    Send_Message("0 - LED OFF\r\n");
    Send_Message("s - Status\r\n");

    while (1)
    {
        USART_ReceiveData(&USART2Handle, &command, 1);

        if (command == '1')
        {
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, ENABLE);
            Send_Message("\r\nLED turned ON\r\n");
        }
        else if (command == '0')
        {
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, DISABLE);
            Send_Message("\r\nLED turned OFF\r\n");
        }
        else if (command == 's' || command == 'S')
        {
            Send_Message("\r\nSystem is running\r\n");
        }
        else
        {
            Send_Message("\r\nInvalid command\r\n");
        }
    }
}
