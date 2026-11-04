
/*
 * main.c
 * Application: Button Interrupt + USART2
 * Board: STM32 Nucleo-F446RE
 *
 * PA2  -> USART2 TX -> ST-LINK Virtual COM Port / PuTTY
 * PC13 -> On-board USER button
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include <stdint.h>

/* USART handle */
USART_Handle_t USART2Handle;

/* Button event flag shared between ISR and main */
volatile uint8_t button_pressed = 0U;


/* --------------------------------------------------
   USART2 GPIO initialization
   PA2 = USART2_TX, Alternate Function 7
   -------------------------------------------------- */
void USART2_GPIO_Init(void)
{
    GPIO_Handle_t USART2_GPIO = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    USART2_GPIO.pGPIOx = GPIOA;

    USART2_GPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART2_GPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 7U;

    GPIO_Init(&USART2_GPIO);
}


/* --------------------------------------------------
   USART2 initialization
   115200 baud, 8 data bits, 1 stop bit, no parity
   -------------------------------------------------- */
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


/* --------------------------------------------------
   PC13 button interrupt initialization
   -------------------------------------------------- */
void Button_Interrupt_Init(void)
{
    GPIO_Handle_t ButtonGPIO = {0};

    /* Enable GPIOC clock */
    GPIO_PeriClockControl(GPIOC, ENABLE);

    /* Configure PC13 as input */
    ButtonGPIO.pGPIOx = GPIOC;

    ButtonGPIO.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    ButtonGPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    ButtonGPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    ButtonGPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
    ButtonGPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    ButtonGPIO.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&ButtonGPIO);

    /* Enable SYSCFG clock */
    RCC->APB2ENR |= (1U << 14);

    /* Route EXTI13 to GPIOC
       EXTICR[3] controls EXTI12 through EXTI15.
       EXTI13 uses bits [7:4].
       GPIOC selection = 0010. */
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);

    /* Mask EXTI13 while configuring */
    EXTI->IMR &= ~(1U << 13);

    /* Trigger on rising edge: button press */
    EXTI->FTSR &= ~(1U << 13);
    EXTI->RTSR |=  (1U << 13);

    /* Clear pending EXTI13 interrupt */
    EXTI->PR = (1U << 13);

    /* Enable IRQ 40 (EXTI15_10) in NVIC.
       ISER1 is at address 0xE000E104.
       IRQ 40 corresponds to bit 8 of ISER1. */
    *(volatile uint32_t *)0xE000E104U = (1U << 8);

    /* Unmask EXTI13 */
    EXTI->IMR |= (1U << 13);
}


/* --------------------------------------------------
   EXTI15_10 interrupt handler
   Runs when PC13 generates an interrupt.
   Do not transmit USART data inside the ISR.
   -------------------------------------------------- */
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & (1U << 13)) != 0U)
    {
        /* Clear pending interrupt by writing 1 */
        EXTI->PR = (1U << 13);

        /* Inform main loop that the button was pressed */
        button_pressed = 1U;
    }
}


/* --------------------------------------------------
   Main application
   -------------------------------------------------- */
int main(void)
{
    char message[] =
        "Button pressed! USART interrupt application.\r\n";

    /* Initialize peripherals */
    USART2_GPIO_Init();
    USART2_Init();
    Button_Interrupt_Init();

    while (1)
    {
        if (button_pressed != 0U)
        {
            /* Clear event flag */
            button_pressed = 0U;

            /* Send message through USART2 */
            USART_SendData(
                &USART2Handle,
                (uint8_t *)message,
                sizeof(message) - 1U
            );
        }
    }
}
