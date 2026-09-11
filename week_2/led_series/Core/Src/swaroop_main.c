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

#include "stm32f446xx.h"
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
     * Enable the clock for GPIOB.
     *
     * RCC->AHB1ENR controls the clocks for GPIO
     * ports on the STM32F446.
     *
     * Bit 1 corresponds to GPIOB.
     */
    RCC->AHB1ENR |= (1U << 1);


    /*
     * Configure PB0 to PB9.
     *
     * Each GPIO pin uses 2 bits in the MODER,
     * OSPEEDR and PUPDR registers.
     */
    for(i = 0; i < 10; i++)
    {

        /*-----------------------------------------
          Configure GPIO Pin as Output
        -----------------------------------------*/

        /*
         * Each pin uses two bits in MODER.
         *
         * MODER:
         * 00 = Input
         * 01 = General-purpose output
         * 10 = Alternate function
         * 11 = Analog
         *
         * First, clear the two bits.
         */
        GPIOB->MODER &= ~(3U << (2 * i));

        /*
         * Set the bits to 01.
         * This configures the pin as output.
         */
        GPIOB->MODER |= (1U << (2 * i));


        /*-----------------------------------------
          Configure Output Type as Push-Pull
        -----------------------------------------*/

        /*
         * OTYPER:
         * 0 = Push-pull
         * 1 = Open-drain
         *
         * Clear the corresponding bit to select
         * push-pull output.
         */
        GPIOB->OTYPER &= ~(1U << i);


        /*-----------------------------------------
          Configure Output Speed as Low Speed
        -----------------------------------------*/

        /*
         * OSPEEDR uses two bits per pin.
         *
         * 00 = Low speed
         *
         * Clearing both bits selects low-speed mode.
         */
        GPIOB->OSPEEDR &= ~(3U << (2 * i));


        /*-----------------------------------------
          Disable Pull-up / Pull-down
        -----------------------------------------*/

        /*
         * PUPDR:
         * 00 = No pull-up / pull-down
         * 01 = Pull-up
         * 10 = Pull-down
         *
         * Clear both bits to select no pull resistor.
         */
        GPIOB->PUPDR &= ~(3U << (2 * i));
    }


    /*
     * Initially turn OFF all LEDs.
     *
     * Clearing bits 0-9 of ODR makes PB0-PB9 LOW.
     */
    GPIOB->ODR &= ~LED_MASK;
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
    /*
     * First clear PB0-PB9.
     * This removes the previous LED pattern.
     */
    GPIOB->ODR &= ~LED_MASK;


    /*
     * Write the new LED pattern.
     *
     * ANDing with LED_MASK ensures that only
     * bits 0-9 affect the LEDs.
     */
    GPIOB->ODR |= (value & LED_MASK);
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
