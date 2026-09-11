/*
 * ================================================================
 *       STM32F446 - Two-LED Shifting Pattern on PB0-PB9
 * ================================================================
 *
 * Description:
 * Controls 10 LEDs connected to PB0-PB9. Two adjacent LEDs
 * are turned ON and shifted from PB0-PB1 to PB8-PB9 with
 * a delay of 300 ms between each step.
 *
 * ================================================================
 */

#include "stm32f446xx.h"
#include <stdint.h>


/*
 * LED_MASK defines the 10 GPIO pins used for the LEDs.
 *
 * LEDs are connected to:
 *
 *     PB0 -> Bit 0
 *     PB1 -> Bit 1
 *     ...
 *     PB9 -> Bit 9
 *
 * 0x03FF in binary:
 *
 *     0011 1111 1111
 *        ^^^^^^^^^^
 *          10 bits
 *
 * Therefore, LED_MASK allows us to modify only PB0-PB9.
 */
#define LED_MASK 0x03FF


/*-------------------------------------------------
  Delay Function
-------------------------------------------------*/

/*
 * @brief  Generate an approximate delay in milliseconds.
 *
 * @param  ms  Number of milliseconds to delay.
 *
 * @note
 * This is a software-based delay. The exact delay depends
 * on the CPU clock frequency and compiler optimization.
 */
void delay_ms(uint32_t ms)
{
    /*
     * Repeat the delay loop once for every requested
     * millisecond.
     */
    while (ms--)
    {
        /*
         * Inner loop creates the actual time delay.
         *
         * The value 4000 is approximate and may need
         * to be changed depending on the MCU clock
         * frequency.
         *
         * 'volatile' prevents the compiler from removing
         * the loop during optimization.
         */
        for (volatile uint32_t i = 0; i < 4000; i++)
        {
            /*
             * NOP = No Operation.
             *
             * __asm tells the compiler that the instruction
             * written inside is an assembly instruction.
             *
             * NOP does not perform any useful operation but
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
 * @brief  Configure PB0-PB9 as GPIO output pins.
 *
 * The ten pins are used to control the ten LEDs.
 */
void leds_init(void)
{
    /*
     * Variable used to select each GPIO pin
     * during configuration.
     */
    uint32_t i;


    /*
     * Enable the clock for GPIOB.
     *
     * GPIOB is connected to the AHB1 peripheral bus.
     *
     * In the AHB1ENR register:
     *
     * Bit 1 = GPIOB clock enable.
     *
     * Setting this bit to 1 supplies the clock to GPIOB.
     */
    RCC->AHB1ENR |= (1U << 1);


    /*
     * Configure PB0 to PB9 one pin at a time.
     */
    for (i = 0; i < 10; i++)
    {
        /*
         * -----------------------------------------
         * Configure GPIO mode
         * -----------------------------------------
         *
         * Each GPIO pin uses two MODER bits.
         *
         * MODER values:
         *
         *   00 -> Input
         *   01 -> General-purpose output
         *   10 -> Alternate function
         *   11 -> Analog
         *
         * First clear the two mode bits for the
         * current pin.
         */
        GPIOB->MODER &= ~(3U << (2 * i));


        /*
         * Set the mode to 01.
         *
         * This configures the current pin as a
         * general-purpose output.
         */
        GPIOB->MODER |= (1U << (2 * i));


        /*
         * -----------------------------------------
         * Configure output type
         * -----------------------------------------
         *
         * OTYPER:
         *
         *   0 -> Push-pull
         *   1 -> Open-drain
         *
         * Clear the corresponding bit to select
         * push-pull output.
         */
        GPIOB->OTYPER &= ~(1U << i);


        /*
         * -----------------------------------------
         * Configure output speed
         * -----------------------------------------
         *
         * OSPEEDR uses two bits per GPIO pin.
         *
         * Clearing both bits selects low-speed output.
         *
         * Low speed is sufficient for LEDs.
         */
        GPIOB->OSPEEDR &= ~(3U << (2 * i));


        /*
         * -----------------------------------------
         * Configure pull-up / pull-down
         * -----------------------------------------
         *
         * PUPDR:
         *
         *   00 -> No pull-up / pull-down
         *   01 -> Pull-up
         *   10 -> Pull-down
         *   11 -> Reserved
         *
         * Clear both bits to select no pull resistor.
         */
        GPIOB->PUPDR &= ~(3U << (2 * i));
    }


    /*
     * Initially turn all LEDs OFF.
     *
     * LED_MASK contains bits 0-9 as 1.
     *
     * Clearing these bits in ODR makes PB0-PB9 LOW.
     */
    GPIOB->ODR &= ~LED_MASK;
}


/*-------------------------------------------------
  Write LEDs
-------------------------------------------------*/

/*
 * @brief  Write a pattern to the ten LEDs.
 *
 * @param  value
 *         10-bit LED pattern.
 *
 * Each bit corresponds to one LED:
 *
 *   Bit 0 -> PB0
 *   Bit 1 -> PB1
 *   ...
 *   Bit 9 -> PB9
 *
 * Example:
 *
 *   value = 0x0001
 *   PB0 = ON, all other LEDs OFF
 *
 *   value = 0x0003
 *   PB0 and PB1 = ON
 *
 *   value = 0x03FF
 *   All ten LEDs = ON
 */
void leds_write(uint16_t value)
{
    /*
     * First clear the current state of PB0-PB9.
     *
     * This removes the previous LED pattern.
     */
    GPIOB->ODR &= ~LED_MASK;


    /*
     * Write the new LED pattern.
     *
     * value & LED_MASK ensures that only the
     * lower 10 bits are used.
     *
     * This prevents accidental changes to GPIOB
     * pins other than PB0-PB9.
     */
    GPIOB->ODR |= (value & LED_MASK);
}


/*-------------------------------------------------
  Main Program
-------------------------------------------------*/

/*
 * @brief  Main application function.
 *
 * The program continuously displays two adjacent
 * LEDs and shifts the pair from left to right.
 */
int main(void)
{
    /*
     * Initial LED pattern:
     *
     * 0x0003 in binary:
     *
     *   0000000011
     *
     * Therefore:
     *
     *   PB0 = ON
     *   PB1 = ON
     *   PB2-PB9 = OFF
     *
     * The pattern will then be shifted left.
     */
    uint16_t pattern = 0x0003;


    /*
     * Loop counter used for shifting the LED pattern.
     */
    int i;


    /*
     * Initialize PB0-PB9 as LED output pins.
     */
    leds_init();


    /*
     * Infinite loop.
     *
     * The LED pattern continuously repeats.
     */
    while (1)
    {
        /*
         * Reset the pattern at the beginning of
         * every complete sequence.
         *
         * Starting pattern:
         *
         *   0000000011
         *
         * PB0 and PB1 are ON.
         */
        pattern = 0x0003;


        /*
         * Shift the two-LED pattern towards the left.
         *
         * The loop executes 9 times.
         */
        for (i = 0; i < 9; i++)
        {
            /*
             * Display the current LED pattern.
             */
            leds_write(pattern);


            /*
             * Keep the current LED pattern ON
             * for approximately 300 ms.
             */
            delay_ms(300);


            /*
             * Shift the pattern one position to the left.
             *
             * Sequence:
             *
             *   0000000011
             *   0000000110
             *   0000001100
             *   0000011000
             *   0000110000
             *   ...
             *   1100000000
             *
             * Therefore, two adjacent LEDs appear
             * to move from PB0-PB1 towards PB8-PB9.
             */
            pattern <<= 1;
        }
    }
}
