#include "stm32f446xx_gpio_driver.h"
#include <stdint.h>


/*
 * LED_MASK:
 *
 * We are using 10 LEDs connected to:
 *
 * PB0  -> LED 1
 * PB1  -> LED 2
 * PB2  -> LED 3
 * ...
 * PB9  -> LED 10
 *
 * Therefore, we need only the lower 10 bits.
 *
 * Binary:
 *
 * 0x03FF = 0000 0011 1111 1111
 *                    ^^^^^^^^^^
 *                    10 bits
 *
 * Bit 0  -> PB0
 * Bit 1  -> PB1
 * ...
 * Bit 9  -> PB9
 */
#define LED_MASK       0x03FF


/*
 * The push button is connected to PC13.
 *
 * PC13 is therefore the GPIO pin used for
 * the button interrupt.
 */
#define BUTTON_PIN     13


/*
 * button_event is a flag used to communicate
 * between the interrupt handler and the main program.
 *
 * 0 -> No button event has occurred.
 * 1 -> Button press has occurred.
 *
 * 'volatile' is used because this variable can
 * be changed inside the interrupt handler while
 * the main program is also using it.
 *
 * Initially the flag is 0 because no button
 * press has happened yet.
 */
volatile uint8_t button_event = 0;


/*-------------------------------------------------
  Delay Function
-------------------------------------------------*/

/*
 * This function creates an approximate delay
 * in milliseconds.
 *
 * The function does not use a hardware timer.
 * Instead, it uses software loops.
 */
void delay_ms(uint32_t ms)
{
    /*
     * Repeat the delay operation 'ms' times.
     *
     * If ms = 200, this loop executes 200 times.
     */
    while (ms--)
    {
        /*
         * This inner loop consumes CPU time.
         *
         * The exact delay depends on the
         * processor clock frequency and compiler.
         */
        for (volatile uint32_t i = 0; i < 4000; i++)
        {
            /*
             * NOP = No Operation.
             *
             * The processor does nothing for
             * one instruction cycle.
             *
             * It is used here to consume time.
             */
            __asm volatile("nop");
        }
    }
}


/*-------------------------------------------------
  Initialize GPIOB PB0-PB9
-------------------------------------------------*/

/*
 * This function configures PB0 to PB9
 * as GPIO output pins for the 10 LEDs.
 */
void leds_init(void)
{
    /*
     * Variable used to select PB0, PB1, ... PB9.
     */
    uint32_t i;

    /*
     * Create a GPIO handle.
     *
     * This structure contains the configuration
     * information required by the GPIO driver.
     */
    GPIO_Handle_t GPIOB_Handle;


    /*
     * Enable the clock for GPIOB.
     *
     * Before using a GPIO peripheral, its
     * peripheral clock must be enabled.
     */
    GPIO_PeriClockControl(GPIOB, ENABLE);


    /*
     * Tell the GPIO driver that the handle
     * belongs to GPIOB.
     */
    GPIOB_Handle.pGPIOx = GPIOB;


    /*
     * Configure all ten LED pins.
     *
     * i = 0 -> PB0
     * i = 1 -> PB1
     * ...
     * i = 9 -> PB9
     */
    for (i = 0; i < 10; i++)
    {
        /*
         * Select the current GPIO pin.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinNumber = i;


        /*
         * Configure the pin as a normal GPIO output.
         *
         * The LEDs need output pins because
         * the microcontroller controls whether
         * each LED is ON or OFF.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;


        /*
         * Select low GPIO speed.
         *
         * The LEDs do not require high-speed
         * GPIO operation.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;


        /*
         * No internal pull-up or pull-down resistor
         * is required for the LED output pins.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;


        /*
         * Configure the output type as push-pull.
         *
         * Push-pull allows the GPIO to actively
         * drive the output HIGH or LOW.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;


        /*
         * Alternate function is not required because
         * PB0-PB9 are being used as normal GPIO outputs.
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;


        /*
         * Apply the above configuration to the
         * currently selected GPIO pin.
         */
        GPIO_Init(&GPIOB_Handle);
    }


    /*
     * Initially turn all LEDs OFF.
     *
     * GPIO_PIN_RESET writes a LOW value to
     * each LED pin.
     */
    for (i = 0; i < 10; i++)
    {
        GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_RESET);
    }
}


/*-------------------------------------------------
  Write LEDs
-------------------------------------------------*/

/*
 * This function controls all 10 LEDs.
 *
 * The 'value' parameter determines which LEDs
 * should be ON and which should be OFF.
 *
 * Example:
 *
 * value = 0x0001
 * -> PB0 ON
 * -> all other LEDs OFF
 *
 * value = 0x0003
 * -> PB0 ON
 * -> PB1 ON
 * -> all other LEDs OFF
 *
 * value = 0x03FF
 * -> all 10 LEDs ON
 *
 * value = 0x0000
 * -> all 10 LEDs OFF
 */
void leds_write(uint16_t value)
{
    /*
     * Variable used to select PB0-PB9.
     */
    uint32_t i;


    /*
     * Keep only the lower 10 bits of 'value'.
     *
     * This prevents bits above bit 9 from
     * affecting the LEDs.
     */
    value &= LED_MASK;


    /*
     * Check every LED one by one.
     *
     * i = 0 -> check bit 0 -> PB0
     * i = 1 -> check bit 1 -> PB1
     * ...
     * i = 9 -> check bit 9 -> PB9
     */
    for (i = 0; i < 10; i++)
    {
        /*
         * (1U << i) creates a number with only
         * bit 'i' set.
         *
         * Example when i = 0:
         * 1U << 0 = 0000000001
         *
         * Example when i = 1:
         * 1U << 1 = 0000000010
         *
         * Example when i = 2:
         * 1U << 2 = 0000000100
         *
         * The '&' checks whether the corresponding
         * bit in 'value' is 1.
         */
        if (value & (1U << i))
        {
            /*
             * Bit is 1.
             *
             * Therefore, turn the corresponding
             * LED ON.
             */
            GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_SET);
        }
        else
        {
            /*
             * Bit is 0.
             *
             * Therefore, turn the corresponding
             * LED OFF.
             */
            GPIO_WriteToOutputPin(GPIOB, i, GPIO_PIN_RESET);
        }
    }
}


/*-------------------------------------------------
  Initialize Button PC13
-------------------------------------------------*/

/*
 * This function configures PC13 as an
 * interrupt input.
 */
void button_init(void)
{
    /*
     * Create a GPIO handle for GPIOC.
     */
    GPIO_Handle_t GPIOC_Handle;


    /*
     * Enable the clock for GPIOC.
     */
    GPIO_PeriClockControl(GPIOC, ENABLE);


    /*
     * Tell the GPIO driver that this handle
     * belongs to GPIOC.
     */
    GPIOC_Handle.pGPIOx = GPIOC;


    /*
     * Select PC13 as the button pin.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinNumber = BUTTON_PIN;


    /*
     * Configure PC13 for interrupt mode
     * with a falling edge.
     *
     * GPIO_MODE_IT_FT means:
     *
     * IT = Interrupt
     * FT = Falling Trigger
     *
     * Therefore, an interrupt is generated
     * when the signal changes from HIGH to LOW.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;


    /*
     * Select low GPIO speed.
     *
     * A push button does not require
     * high-speed GPIO operation.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;


    /*
     * Enable the internal pull-up resistor.
     *
     * This keeps PC13 HIGH when the button
     * is not pressed.
     *
     * When the button is pressed, the pin
     * is pulled LOW.
     *
     * Therefore:
     *
     * Button released -> HIGH
     * Button pressed  -> LOW
     *
     * HIGH -> LOW is a falling edge.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;


    /*
     * Configure the input pin output type.
     *
     * This setting is part of the GPIO
     * configuration structure.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;


    /*
     * No alternate function is required.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;


    /*
     * Apply the PC13 configuration.
     */
    GPIO_Init(&GPIOC_Handle);


    /*
     * Enable the EXTI interrupt for pins 10 to 15.
     *
     * PC13 belongs to this interrupt group.
     *
     * EXTI15_10 handles:
     *
     * PC10
     * PC11
     * PC12
     * PC13
     * PC14
     * PC15
     */
    GPIO_IRQConfig(IRQ_NO_EXTI15_10,
                   NVIC_IRQ_PRI15,
                   ENABLE);
}


/*-------------------------------------------------
  External Interrupt Handler
-------------------------------------------------*/

/*
 * This function is automatically called by
 * the processor when the EXTI15_10 interrupt
 * occurs.
 *
 * Since the button is connected to PC13,
 * pressing the button generates the interrupt.
 */
void EXTI15_10_IRQHandler(void)
{
    /*
     * Handle the GPIO interrupt.
     *
     * This function clears the interrupt
     * pending condition so that the same
     * interrupt can be detected again later.
     */
    GPIO_IRQHandling(BUTTON_PIN);


    /*
     * Record that the button was pressed.
     *
     * 1 means:
     * "A button event has occurred."
     *
     * The main program will later check
     * this flag.
     */
    button_event = 1;
}


/*-------------------------------------------------
  Interrupt Pattern
-------------------------------------------------*/

/*
 * This function is executed when the
 * button interrupt is detected.
 *
 * The pattern makes all 10 LEDs blink
 * three times.
 */
void interrupt_pattern(void)
{
    /*
     * Loop counter.
     */
    int i;


    /*
     * Repeat the ON/OFF sequence three times.
     */
    for (i = 0; i < 3; i++)
    {
        /*
         * Turn all 10 LEDs ON.
         *
         * 0x03FF:
         *
         * PB9 PB8 ... PB1 PB0
         *  1   1  ...  1   1
         *
         * Therefore all LEDs are ON.
         */
        leds_write(0x03FF);


        /*
         * Keep all LEDs ON for approximately
         * 200 milliseconds.
         */
        delay_ms(200);


        /*
         * Turn all 10 LEDs OFF.
         *
         * 0x0000 means all bits are zero.
         */
        leds_write(0x0000);


        /*
         * Keep all LEDs OFF for approximately
         * 200 milliseconds.
         */
        delay_ms(200);
    }
}


/*-------------------------------------------------
  Main LED Pattern
  Two LEDs move from PB0-PB1 to PB8-PB9
-------------------------------------------------*/

/*
 * This is the normal LED pattern.
 *
 * Two LEDs are ON at a time.
 *
 * The pair moves from left to right:
 *
 * PB0 PB1
 * PB1 PB2
 * PB2 PB3
 * ...
 * PB8 PB9
 */
void pattern_two_leds(void)
{
    /*
     * Initial pattern:
     *
     * 0x0003 = binary 0000000011
     *
     * Bit 0 = 1 -> PB0 ON
     * Bit 1 = 1 -> PB1 ON
     *
     * Therefore PB0 and PB1 are ON.
     */
    uint16_t pattern = 0x0003;


    /*
     * Loop counter used to move the
     * two-LED pattern.
     */
    int i;


    /*
     * Execute the pattern 9 times.
     *
     * Starting at PB0-PB1:
     *
     * 1. PB0-PB1
     * 2. PB1-PB2
     * 3. PB2-PB3
     * ...
     * 9. PB8-PB9
     */
    for (i = 0; i < 9; i++)
    {
        /*
         * Display the current LED pattern.
         *
         * Initially:
         * pattern = 0x0003
         * -> PB0 and PB1 ON.
         */
        leds_write(pattern);


        /*
         * Keep the current LED position ON
         * for some time.
         *
         * During this time, the button is
         * continuously checked.
         */
        for (volatile uint32_t j = 0; j < 300; j++)
        {
            /*
             * Wait approximately 1 millisecond.
             *
             * 300 iterations therefore give
             * approximately 300 ms at each
             * LED position.
             */
            delay_ms(1);


            /*
             * Check whether the button interrupt
             * has occurred.
             *
             * The interrupt handler changes:
             *
             * button_event = 0
             *
             * to:
             *
             * button_event = 1
             */
            if (button_event)
            {
                /*
                 * A button event has occurred.
                 *
                 * return exits this function
                 * immediately.
                 *
                 * Control goes back to main().
                 */
                return;
            }
        }


        /*
         * Shift the pattern one bit to the left.
         *
         * Example:
         *
         * 0000000011  -> PB0, PB1
         *
         * shift left:
         *
         * 0000000110  -> PB1, PB2
         *
         * shift left:
         *
         * 0000001100  -> PB2, PB3
         *
         * and so on.
         */
        pattern <<= 1;
    }
}


/*-------------------------------------------------
  Main Program
-------------------------------------------------*/

/*
 * Program execution starts from main().
 */
int main(void)
{
    /*
     * Initialize PB0-PB9 as LED outputs.
     */
    leds_init();


    /*
     * Initialize PC13 as a button input
     * with external interrupt support.
     */
    button_init();


    /*
     * Initially turn all LEDs OFF.
     */
    leds_write(0);


    /*
     * The main program continuously repeats.
     */
    while (1)
    {
        /*
         * Clear the previous button event.
         *
         * 0 means:
         * "There is currently no unprocessed
         * button event."
         *
         * This prepares the program for a
         * new button press.
         */
        button_event = 0;


        /*
         * Run the normal LED pattern.
         *
         * The two LEDs move from PB0-PB1
         * toward PB8-PB9.
         *
         * If the button is pressed while
         * this function is running, the
         * interrupt changes button_event
         * to 1 and pattern_two_leds()
         * returns.
         */
        pattern_two_leds();


        /*
         * Check whether the button was pressed
         * during the LED pattern.
         *
         * If button_event = 1:
         *
         *     A button press occurred.
         *
         * If button_event = 0:
         *
         *     No button press occurred.
         */
        if (button_event)
        {
            /*
             * Execute the special interrupt
             * pattern.
             *
             * All LEDs turn ON and OFF
             * three times.
             */
            interrupt_pattern();
        }
    }
}
