
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include <stdint.h>

/* Peripheral handles */
USART_Handle_t USART2Handle;
GPIO_Handle_t LED;

/* Shared between interrupt handler and main */
volatile uint8_t button_pressed = 0U;
uint8_t led_status = 0U;


/* USART2 GPIO initialization: PA2 = TX, AF7 */
void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART_GPIO = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART_GPIO.pGPIOx = GPIOA;
    USART_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    USART_GPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART_GPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    USART_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    USART_GPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 7U;

    GPIO_Init(&USART_GPIO);
}


/* LED initialization: PA5 = onboard LD2 */
void LED_Init(void)
{
    GPIO_PeriClockControl(GPIOA, ENABLE);

    LED.pGPIOx = GPIOA;
    LED.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    LED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    LED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    LED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    LED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    LED.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&LED);

    /* Start with LED OFF */
    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, 0U);
}


/* USART2 initialization: 115200, 8-N-1 */
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


/* Send a null-terminated string to PuTTY */
void Send_Message(char *msg)
{
    uint32_t length = 0U;

    while (msg[length] != '\0')
    {
        length++;
    }

    USART_SendData(
        &USART2Handle,
        (uint8_t *)msg,
        length
    );
}


/* PC13 button interrupt initialization */
void Button_Interrupt_Init(void)
{
    GPIO_Handle_t BUTTON = {0};

    /* Enable GPIOC clock */
    GPIO_PeriClockControl(GPIOC, ENABLE);

    /* Configure PC13 as input */
    BUTTON.pGPIOx = GPIOC;
    BUTTON.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    BUTTON.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    BUTTON.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    BUTTON.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
    BUTTON.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    BUTTON.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&BUTTON);

    /* Enable SYSCFG clock */
    RCC->APB2ENR |= (1U << 14);

    /* Connect EXTI13 to GPIOC */
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);

    /* Mask EXTI13 while configuring */
    EXTI->IMR &= ~(1U << 13);

    /* Trigger on rising edge when button is pressed */
    EXTI->FTSR &= ~(1U << 13);
    EXTI->RTSR |=  (1U << 13);

    /* Clear pending EXTI13 flag */
    EXTI->PR = (1U << 13);

    /* Enable EXTI15_10 IRQ (IRQ number 40) */
    *NVIC_ISER1 = (1U << (IRQ_NO_EXTI15_10 % 32U));

    /* Unmask EXTI13 */
    EXTI->IMR |= (1U << 13);
}


/* Interrupt Service Routine */
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & (1U << 13)) != 0U)
    {
        /* Clear pending interrupt */
        EXTI->PR = (1U << 13);

        /* Set flag; handle LED and USART in main */
        button_pressed = 1U;
    }
}


/* Main application */
int main(void)
{
    USART2_GPIO_Init();
    LED_Init();
    USART2_Init();
    Button_Interrupt_Init();

    Send_Message("\r\nButton Interrupt LED Control\r\n");
    Send_Message("Press USER button to toggle LED\r\n");

    while (1)
    {
        if (button_pressed != 0U)
        {
            button_pressed = 0U;

            /* Toggle LED state */
            led_status = !led_status;

            GPIO_WriteToOutputPin(
                GPIOA,
                GPIO_PIN_NO_5,
                led_status
            );

            if (led_status != 0U)
            {
                Send_Message("Button pressed: LED ON\r\n");
            }
            else
            {
                Send_Message("Button pressed: LED OFF\r\n");
            }
        }
    }
}
