/*
 * 001led_toggle.c
 *
 *  Created on: Aug 25, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Toggle the on-board user LED (LD2) using the custom GPIO driver.
 *
 *  Hardware note:
 *      LD2 (green user LED) is connected to PA5, active-high.
 *      Refer UM1724 - STM32 Nucleo-64 boards user manual, Section 6.4 (LEDs).
 */

#include "stm32f446xx.h"              // MCU header : register structures, base addresses, macros
#include "stm32f446xx_gpio_driver.h"  // GPIO driver header : handle/config structs, API prototypes


/*********************************************************************
 * @fn                - delay
 * @brief             - Crude blocking software delay
 * @return            - None
 * @Note              - MCU runs on 16 MHz HSI after reset (no clock config
 *                      done here), so this gives roughly 200-300 ms.
 *                      Loop counter is volatile so the compiler does not
 *                      optimise the empty loop away at -O1 and above.
 *********************************************************************/
void delay(void)
{
    for(volatile uint32_t i = 0; i < 500000; i++);  // burn CPU cycles; body intentionally empty
}


int main(void)
{
    GPIO_Handle_t GpioLed;                                        // handle variable for the LED pin

    GpioLed.pGPIOx = GPIOA;                                       // LED sits on port A, so point handle at GPIOA base address

    GpioLed.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;   // LD2 is wired to pin 5 of port A
    GpioLed.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;   // drive the pin, so general purpose output mode
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // slew rate; irrelevant for an LED, kept for illustration
    GpioLed.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP; // push-pull so the pin can source current into the LED
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // push-pull drives both rails, no internal resistor needed
    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode  = 0;               // unused in output mode, but set it so no stack garbage remains

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // ungate GPIOA clock in RCC_AHB1ENR; without this all register writes are ignored

    GPIO_Init(&GpioLed);                                          // apply the settings above to MODER, OSPEEDR, PUPDR and OTYPER

    while(1)                                                      // super loop : an embedded application never returns
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);               // XOR bit 5 of GPIOA_ODR, flipping the LED state
        delay();                                                  // hold the state long enough for the eye to see it
    }

    return 0;                                                     // never reached; present only to satisfy the int return type
}
