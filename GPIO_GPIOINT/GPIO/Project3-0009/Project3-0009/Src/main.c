/*
 * ================================================================
 *        STM32F446 - Odd-Even LED Wave Pattern on PB0-PB9
 * ================================================================
 *
 * Description:
 * Controls 10 LEDs connected to GPIOB pins PB0-PB9.
 * The odd and even LED patterns alternate every 300 ms
 * and continuously repeat.
 *
 * ================================================================
 */

#include "stm32f446xx_gpio_driver.h"
#include <stdint.h>


/*
 * LED_MASK
 * --------
 * PB0 to PB9 are connected to 10 LEDs.
 *
 * Binary:
 * 0x03FF = 0000001111111111
 *
 * Bits 0 to 9 are set to 1.
 * This mask is used to control only PB0-PB9.
 */

#define LED_MASK 0x03FF


/*-------------------------------------------------
  Delay Function
-------------------------------------------------*/

/*
 * delay_ms()
 * ----------
 * Generates an approximate software delay.
 *
 * The actual delay depends on the MCU clock frequency,
 * compiler optimization and processor speed.
 */

void delay_ms(uint32_t ms)
{
    /* Repeat the delay loop 'ms' times */

    while(ms--)
    {
        /*
         * Inner loop creates the actual time delay.
         * 'volatile' prevents the compiler from
         * optimizing the loop away.
         */

        for(volatile uint32_t i = 0; i < 4000; i++)
        {
            /*
             * NOP (No Operation) is an assembly instruction.
             * It does not perform any useful operation but
             * consumes processor cycles to create a delay.
             */

            __asm volatile("nop");
        }
    }
}


/*-------------------------------------------------
  Initialize GPIOB PB0-PB9
-------------------------------------------------*/

/*
 * leds_init()
 * -----------
 * Configures GPIOB pins PB0 to PB9 as:
 *
 * 1. General-purpose output
 * 2. Push-pull
 * 3. Low speed
 * 4. No pull-up or pull-down
 */

void leds_init(void)
{
    uint32_t i;

    /*
     * GPIO handle used to configure GPIOB pins.
     */

    GPIO_Handle_t GPIOB_Handle;


    /*
     * Enable the clock for GPIOB.
     *
     * GPIOB is connected to the AHB1 peripheral bus.
     *
     * The custom GPIO driver enables the GPIOB
     * peripheral clock.
     */

    GPIO_PeriClockControl(GPIOB, ENABLE);


    /*
     * Select GPIOB as the port to configure.
     */

    GPIOB_Handle.pGPIOx = GPIOB;


    /*
     * Configure PB0 to PB9.
     */

    for(i = 0; i < 10; i++)
    {
        /*-----------------------------------------
          Configure GPIO Pin as Output
        -----------------------------------------*/

        /*
         * GPIO_MODE_OUT configures the pin as
         * general-purpose output.
         */

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinNumber = i;

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;


        /*-----------------------------------------
          Configure Output Type as Push-Pull
        -----------------------------------------*/

        /*
         * GPIO_OP_TYPE_PP selects push-pull output.
         */

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;


        /*-----------------------------------------
          Configure Output Speed as Low Speed
        -----------------------------------------*/

        /*
         * Low speed is sufficient for LEDs.
         */

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;


        /*-----------------------------------------
          Disable Pull-up / Pull-down
        -----------------------------------------*/

        /*
         * No pull-up or pull-down resistor is required
         * for the LED output pins.
         */

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;


        /*
         * No alternate function is required because
         * these pins are being used as normal GPIO.
         */

        GPIOB_Handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;


        /*
         * Initialize the current GPIO pin using
         * the custom GPIO driver.
         */

        GPIO_Init(&GPIOB_Handle);
    }


    /*
     * Initially turn OFF all LEDs.
     *
     * Write LOW to PB0-PB9.
     */

    for(i = 0; i < 10; i++)
    {
        GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_RESET);
    }
}


/*-------------------------------------------------
  Write LEDs
-------------------------------------------------*/

/*
 * leds_write()
 * ------------
 * Sends a 10-bit pattern to PB0-PB9.
 *
 * Bit 0  -> PB0
 * Bit 1  -> PB1
 * ...
 * Bit 9  -> PB9
 *
 * Bit value:
 * 1 = LED ON
 * 0 = LED OFF
 */

void leds_write(uint16_t value)
{
    uint32_t i;


    /*
     * First make sure that only bits 0-9
     * are used.
     *
     * ANDing with LED_MASK ensures that only
     * bits 0-9 affect the LEDs.
     */

    value &= LED_MASK;


    /*
     * Write each bit of the pattern to its
     * corresponding GPIO pin.
     */

    for(i = 0; i < 10; i++)
    {
        /*
         * Check whether the current bit is 1.
         */

        if(value & (1U << i))
        {
            /*
             * Bit is 1, so turn the LED ON.
             */

            GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_SET);
        }
        else
        {
            /*
             * Bit is 0, so turn the LED OFF.
             */

            GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_RESET);
        }
    }
}


/*-------------------------------------------------
  Odd-Even Wave Pattern
-------------------------------------------------*/

/*
 * odd_even_wave()
 * ---------------
 * Alternately turns ON:
 *
 * 1. PB0, PB2, PB4, PB6 and PB8
 * 2. PB1, PB3, PB5, PB7 and PB9
 *
 * Each pattern remains ON for 300 ms.
 */

void odd_even_wave(void)
{
    /*
     * Pattern for PB0, PB2, PB4, PB6 and PB8.
     *
     * Binary:
     * 0101010101
     *
     * Hexadecimal:
     * 0x0155
     *
     * PB9 PB8 PB7 PB6 PB5 PB4 PB3 PB2 PB1 PB0
     *  0   1   0   1   0   1   0   1   0   1
     */

    uint16_t odd_pattern = 0x0155;


    /*
     * Pattern for PB1, PB3, PB5, PB7 and PB9.
     *
     * Binary:
     * 1010101010
     *
     * Hexadecimal:
     * 0x02AA
     *
     * PB9 PB8 PB7 PB6 PB5 PB4 PB3 PB2 PB1 PB0
     *  1   0   1   0   1   0   1   0   1   0
     */

    uint16_t even_pattern = 0x02AA;


    /*
     * Display the first pattern.
     *
     * LEDs connected to PB0, PB2, PB4, PB6
     * and PB8 turn ON.
     */

    leds_write(odd_pattern);


    /*
     * Keep the first pattern displayed for 300 ms.
     */

    delay_ms(300);


    /*
     * Display the second pattern.
     *
     * LEDs connected to PB1, PB3, PB5, PB7
     * and PB9 turn ON.
     */

    leds_write(even_pattern);


    /*
     * Keep the second pattern displayed for 300 ms.
     */

    delay_ms(300);
}


/*-------------------------------------------------
  Main Program
-------------------------------------------------*/

/*
 * main()
 * ------
 * Program execution starts from this function.
 */

int main(void)
{
    /*
     * Initialize PB0-PB9 as LED outputs.
     */

    leds_init();


    /*
     * Infinite loop.
     *
     * The odd-even wave pattern is continuously
     * repeated.
     */

    while(1)
    {
        /*
         * Execute the alternating LED pattern.
         */

        odd_even_wave();
    }
}
