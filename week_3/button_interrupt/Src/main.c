/*
 * main.c
 *
 *  Created on: Sep 26, 2026
 */

#include <string.h>
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"

#define BTN_PORT    GPIOC          //onboard button B1 sits on port C
#define BTN_PIN     GPIO_PIN_NO_13 //B1 = PC13

USART_Handle_t usart2handle;

volatile uint8_t button_pressed = 0;   //ISR sets/clears this, main loop just checks it

//just a busy loop, gives the button contact time to stop bouncing
static void delay(void)
{
    for(uint32_t i = 0; i < 250000; i++);
}

//sets up PA2/PA3 in AF mode for USART2 - this is the ST-Link VCP, no extra wiring needed
static void USART2_GPIO_Init(void)
{
    GPIO_PeriClockControl(GPIOA, ENABLE);

    GPIO_Handle_t usart_pins;
    usart_pins.pGPIOx = GPIOA;
    usart_pins.GPIO_PinConfig.GPIO_PinMode      = GPIO_MODE_ALTFN;
    usart_pins.GPIO_PinConfig.GPIO_PinAltFunMode = GPIO_AF7;  //AF7 = USART2 on PA2/PA3
    usart_pins.GPIO_PinConfig.GPIO_PinSpeed     = GPIO_SPEED_FAST;
    usart_pins.GPIO_PinConfig.GPIO_PinOPType    = GPIO_OP_TYPE_PP;
    usart_pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    usart_pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;  //Tx
    GPIO_Init(&usart_pins);

    usart_pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;  //Rx
    GPIO_Init(&usart_pins);
}

//just Tx is enough here since we're only printing to the terminal, not reading anything back
static void USART2_Init(void)
{
    usart2handle.pUSARTx = USART2;
    usart2handle.USART_Config.USART_Mode          = USART_MODE_ONLY_TX;
    usart2handle.USART_Config.USART_Baud          = USART_STD_BAUD_115200;
    usart2handle.USART_Config.USART_NoOfStopBits  = USART_STOPBITS_1;
    usart2handle.USART_Config.USART_WordLength    = USART_WORDLEN_8BITS;
    usart2handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    usart2handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(&usart2handle);
}

//B1 already has an external pull-up on the Nucleo board, so no internal pupd needed
//interrupt on both edges - RFT - since we want to catch press AND release
static void Button_Init(void)
{
    GPIO_PeriClockControl(BTN_PORT, ENABLE);

    GPIO_Handle_t btn;
    btn.pGPIOx = BTN_PORT;
    btn.GPIO_PinConfig.GPIO_PinNumber     = BTN_PIN;
    btn.GPIO_PinConfig.GPIO_PinMode       = GPIO_MODE_IT_RFT;
    btn.GPIO_PinConfig.GPIO_PinSpeed      = GPIO_SPEED_FAST;
    btn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&btn);

    GPIO_IRQConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15, ENABLE); //PC13 = EXTI13, shares the 15_10 vector
}

int main(void)
{
    USART2_GPIO_Init();
    USART2_Init();
    Button_Init();

    USART_PeripheralControl(USART2, ENABLE);

    while(1)
    {
        //only poll/print while the button isn't held down
        if(!button_pressed)
        {
            USART_SendData(&usart2handle, (uint8_t*)"polling\r\n", strlen("polling\r\n"));
            delay();   //spacing it out so the terminal doesn't get flooded
        }
    }
}

//PC13 lands in this shared vector (EXTI lines 10 to 15)
void EXTI15_10_IRQHandler(void)
{
    EXTI->IMR &= ~(1U << BTN_PIN);   //mask the line first, don't want bounce edges re-firing this

    GPIO_IRQHandling(BTN_PIN);       //clears the pending flag for this edge

    delay();                        //wait for the bounce to die down

    EXTI->PR = (1U << BTN_PIN);     //clear again - throws away any bounce edges that snuck in during the delay

    if(GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == 0)
    {
        //pin reads 0 = button is pressed (pulled low)
        button_pressed = 1;   //tells the main loop to stop polling
        USART_SendData(&usart2handle, (uint8_t*)"Button Pressed\r\n", strlen("Button Pressed\r\n"));
    }
    else
    {
        //pin reads 1 = released, back to the pull-up
        button_pressed = 0;   //main loop goes back to polling
        USART_SendData(&usart2handle, (uint8_t*)"Button Released\r\n", strlen("Button Released\r\n"));
    }

    EXTI->IMR |= (1U << BTN_PIN);   //done debouncing, re-arm the line
}
