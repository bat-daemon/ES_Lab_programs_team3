/*
 * 004button_interrupt.c
 *
 *  Created on: Aug 25, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Toggle the on-board user LED (LD2, PA5) from an interrupt service
 *            routine, triggered by a FALLING edge on the user button
 *            (B1, PC13). Interrupt method - contrast with 003led_button.c
 *            which polls the pin.
 *
 *  Hardware notes (UM1724, Sections 6.4 and 6.5):
 *      LD2 -> PA5,  active HIGH : writing 1 lights the LED.
 *      B1  -> PC13, active LOW  : line idles HIGH, drops to GND when pressed.
 *
 *      Because the line falls when the button goes DOWN, the press event is a
 *      FALLING edge. Hence GPIO_MODE_IT_FT.
 *
 *  Interrupt path (each stage is a gate that defaults to closed):
 *      PC13 pin -> EXTI line 13 -> NVIC IRQ 40 -> EXTI15_10_IRQHandler()
 *
 *      EXTI lines 10 to 15 SHARE one vector, so line 13 maps to IRQ number 40
 *      (IRQ_NO_EXTI15_10). SYSCFG_EXTICR4 decides that line 13 belongs to
 *      port C rather than port A, B, D ... - handled inside GPIO_Init().
 */

#include <string.h>                   // for memset, to zero the handle structures
#include "stm32f446xx.h"              // MCU header : register structures, base addresses, macros
#include "stm32f446xx_gpio_driver.h"  // GPIO driver header : handle/config structs, API prototypes

//#define BTN_IRQ_PRIORITY   15U        // lowest urgency; any value works with a single interrupt source


/*********************************************************************
 * @fn                - delay
 * @brief             - Crude blocking delay, used here to ride out switch bounce
 * @return            - None
 * @Note              - Roughly 200 ms on the 16 MHz HSI.
 *********************************************************************/
void delay(void)
{
    for(volatile uint32_t i = 0; i < 500000/2; i++);  // burn CPU cycles; body intentionally empty
}


int main(void)
{
    GPIO_Handle_t GpioLed;                                        // handle for the output pin (LED)
    GPIO_Handle_t GpioBtn;                                        // handle for the input pin (button)

    memset(&GpioLed, 0, sizeof(GpioLed));                         // clear stack garbage before filling the fields
    memset(&GpioBtn, 0, sizeof(GpioBtn));                         // same for the button handle

    /* ---------------- LED configuration : PA5, output ---------------- */

    GpioLed.pGPIOx = GPIOA;                                       // LED sits on port A

    GpioLed.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;   // LD2 is wired to pin 5
    GpioLed.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;   // drive the pin, general purpose output
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // slew rate; irrelevant for an LED
    GpioLed.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP; // push-pull so the pin can source current into the LED
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // push-pull drives both rails, no internal resistor needed

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // ungate GPIOA clock in RCC_AHB1ENR
    GPIO_Init(&GpioLed);                                          // write MODER, OSPEEDR, PUPDR, OTYPER for PA5

    /* -------- Button configuration : PC13, falling-edge interrupt -------- */

    GpioBtn.pGPIOx = GPIOC;                                       // button sits on port C

    GpioBtn.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_13;  // B1 is wired to pin 13
    GpioBtn.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_IT_FT; // STEP 1+2 : input mode + falling trigger (EXTI_FTSR bit 13)
    GpioBtn.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // meaningless for an input, kept for uniformity
    GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;     // internal pull-up holds the line HIGH while released

    GPIO_PeriClockControl(GPIOC, ENABLE);                         // ungate GPIOC clock; forgetting this leaves the button dead

    GPIO_Init(&GpioBtn);                                          // STEP 3 : also sets SYSCFG_EXTICR4 to port C and unmasks EXTI_IMR bit 13

    /* ---------- Processor-side interrupt configuration (NVIC) ---------- */

    //GPIO_IRQConfig(IRQ_NO_EXTI15_10, BTN_IRQ_PRIORITY, ENABLE);   // STEP 4+5+6 : IRQ 40, set priority, set NVIC_ISER1 bit 8
	
	GPIO_IRQConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15, ENABLE);

    /* ------------------------- Super loop ---------------------------- */

    while(1)                                                      // CPU is free here - no polling of the button at all
    {
        ;                                                         // in a real design put the application work here, or sleep
    }

    return 0;                                                     // never reached; present only to satisfy the int return type
}


/*********************************************************************
 * @fn                - EXTI15_10_IRQHandler - check startup file for name of handler - line - 189
 * @brief             - ISR for EXTI lines 10 to 15; called by the NVIC
 * @return            - None
 * @Note              - STEP 7. The name MUST match the symbol in
 *                      startup_stm32f446retx.s exactly. A typo compiles and
 *                      links cleanly, but the weak default handler runs
 *                      instead and traps in an infinite loop, so the board
 *                      appears to freeze on the first press.
 *********************************************************************/
void EXTI15_10_IRQHandler(void)
{
    delay();                                                      // crude debounce; contacts bounce for a few ms after the edge

    GPIO_IRQHandling(GPIO_PIN_NO_13);                             // clear EXTI_PR bit 13 by writing 1 - omit this and the ISR re-enters forever

    GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);                   // XOR bit 5 of GPIOA_ODR, flipping the LED state
}
