/*
 * 003led_button.c
 *
 *  Created on: Aug 25, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Toggle the on-board user LED (LD2, PA5) each time the on-board
 *            user button (B1, PC13) is pressed. Polling method.
 *
 *  Hardware notes (UM1724, Sections 6.4 and 6.5):
 *      LD2  -> PA5,  active HIGH : writing 1 lights the LED.
 *      B1   -> PC13, active LOW  : the line idles HIGH and is pulled to GND
 *                                  while the button is held down.
 *
 *      The button is on port C, the LED on port A, so BOTH clocks must be
 *      enabled - a very common mistake is enabling only GPIOA.
 *
 *      The board already fits an external pull-up on PC13, so GPIO_NO_PUPD
 *      would work. The internal pull-up is enabled here anyway: it costs
 *      nothing, and the same code then works on a bare pin with just a
 *      push-to-ground switch.
 */

#include "stm32f446xx.h"              // MCU header : register structures, base addresses, macros
#include "stm32f446xx_gpio_driver.h"  // GPIO driver header : handle/config structs, API prototypes

#define BTN_PRESSED   GPIO_PIN_RESET  // B1 is active LOW, so a pressed button reads 0, refer 4_mb1136_Nucleo446rw-default-c03_schematic


/*********************************************************************
 * @fn                - delay
 * @brief             - Crude blocking delay, used here for switch debouncing
 * @return            - None
 * @Note              - Roughly 200 ms on the 16 MHz HSI. Anything above about
 *                      20 ms is enough to ride out contact bounce.
 *********************************************************************/
void delay(void)
{
    for(volatile uint32_t i = 0; i < 500000/2; i++);  // burn CPU cycles; body intentionally empty
}


int main(void)
{
    GPIO_Handle_t GpioLed;                                        // handle for the output pin (LED)
    GPIO_Handle_t GpioBtn;                                        // handle for the input pin (button)

    /* ---------------- LED configuration : PA5, output ---------------- */

    GpioLed.pGPIOx = GPIOA;                                       // LED sits on port A

    GpioLed.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;   // LD2 is wired to pin 5
    GpioLed.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;   // drive the pin, general purpose output
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // slew rate; irrelevant for an LED
    GpioLed.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP; // push-pull so the pin can source current into the LED
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // push-pull drives both rails, no internal resistor needed
    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode  = 0;               // unused in output mode, cleared to avoid stack garbage

    /* -------------- Button configuration : PC13, input --------------- */

    GpioBtn.pGPIOx = GPIOC;                                       // button sits on port C

    GpioBtn.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_13;  // B1 is wired to pin 13
    GpioBtn.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_IN;    // read the pin, general purpose input
    GpioBtn.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // speed field is meaningless for an input, kept for uniformity
    GpioBtn.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP; // output type is meaningless for an input, kept for uniformity
    GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;     // internal pull-up holds the line HIGH while the button is released
    GpioBtn.GPIO_PinConfig.GPIO_PinAltFunMode  = 0;               // unused in input mode, cleared to avoid stack garbage

    /* ---------------------- Bring up both ports ---------------------- */

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // ungate GPIOA clock in RCC_AHB1ENR for the LED
    GPIO_PeriClockControl(GPIOC, ENABLE);                         // ungate GPIOC clock in RCC_AHB1ENR for the button

    GPIO_Init(&GpioLed);                                          // write the LED settings into MODER, OSPEEDR, PUPDR, OTYPER
    GPIO_Init(&GpioBtn);                                          // write the button settings into MODER and PUPDR

    /* ------------------------- Super loop ---------------------------- */

    while(1)                                                      // an embedded application never returns
    {
        if(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == BTN_PRESSED)   // sample bit 13 of GPIOC_IDR; 0 means pressed
        {
            delay();                                              // debounce : wait for the mechanical contacts to settle

            if(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == BTN_PRESSED)  // re-sample; still low, so it was a real press not a bounce spike
            {
                GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);       // XOR bit 5 of GPIOA_ODR, flipping the LED state

                while(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == BTN_PRESSED);  // wait here until released, so one press gives exactly one toggle

                delay();                                          // debounce the release edge as well
            }
        }
    }

    return 0;                                                     // never reached; present only to satisfy the int return type
}
