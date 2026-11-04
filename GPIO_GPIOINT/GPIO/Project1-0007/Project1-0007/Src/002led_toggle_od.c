/*
 * 002led_toggle_od.c
 *
 *  Created on: Aug 25, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Toggle the on-board user LED (LD2) with the pin configured in
 *            OPEN-DRAIN output mode, to contrast against 001led_toggle.c
 *            which uses push-pull.
 *
 *  Hardware note:
 *      LD2 (green user LED) is connected to PA5, active-high, with the LED
 *      anode towards the pin and cathode towards ground through a series
 *      resistor. Refer UM1724, Section 6.4 (LEDs).
 *
 *  EXPECTED BEHAVIOUR - this is the point of the experiment:
 *      An open-drain output can only pull the pin LOW. When the driver
 *      writes a 1, the N-channel transistor turns off and the pin is left
 *      floating (high impedance) - there is no transistor to Vdd to source
 *      current into the LED. With GPIO_NO_PUPD the LED therefore stays OFF
 *      permanently and no blinking is observed.
 *
 *      Enabling the internal pull-up (see the commented alternative below)
 *      ties the floating pin to Vdd through roughly 40 kOhm. That resistance
 *      limits the current to a few tens of microamps, far below the LED's
 *      normal drive current, so the LED glows only very faintly if at all.
 *
 *      Conclusion for the lab: open-drain is meant for shared buses such as
 *      I2C, where an external pull-up of 2.2k-4.7k is fitted and any device
 *      may pull the line low. It is not suitable for sourcing current into
 *      a load like an LED.
 */

#include "stm32f446xx.h"              // MCU header : register structures, base addresses, macros
#include "stm32f446xx_gpio_driver.h"  // GPIO driver header : handle/config structs, API prototypes


/*********************************************************************
 * @fn                - delay
 * @brief             - Crude blocking software delay
 * @return            - None
 * @Note              - MCU runs on 16 MHz HSI after reset, so this gives
 *                      roughly 200-300 ms. Counter is volatile so the
 *                      empty loop is not optimised away.
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
    GpioLed.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;   // still an output pin; only the output TYPE changes
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // slew rate; irrelevant for an LED, kept for illustration
    GpioLed.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_OD; // OPEN-DRAIN : sets bit 5 of GPIOA_OTYPER, pin can only sink
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // no internal pull-up, so the HIGH state is left floating slide 235

 // GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;     // <-- swap this line in to see the faint-glow case

    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode  = 0;               // unused in output mode, but set it so no stack garbage remains

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // ungate GPIOA clock in RCC_AHB1ENR; without this all register writes are ignored

    GPIO_Init(&GpioLed);                                          // apply the settings above to MODER, OSPEEDR, PUPDR and OTYPER

    while(1)                                                      // super loop : an embedded application never returns
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);               // XOR bit 5 of GPIOA_ODR; the ODR bit still flips even though the pin cannot drive high
        delay();                                                  // hold the state long enough for the eye to see it
    }

    return 0;                                                     // never reached; present only to satisfy the int return type
}
