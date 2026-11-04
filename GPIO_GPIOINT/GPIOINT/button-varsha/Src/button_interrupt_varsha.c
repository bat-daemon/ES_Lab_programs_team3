/* USER CODE BEGIN Header */

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  *
  * This program controls 10 LEDs connected to GPIOB pins PB0-PB9.
  * A push button connected to PC13 is used to select and start LED patterns.
  *
  * Every time the button is pressed and released, the next LED pattern
  * is executed.
  *
  * Pattern sequence:
  *   0 -> Chase
  *   1 -> Knight Rider
  *   2 -> Bar Fill and Drain
  *   3 -> Binary Counter
  *   4 -> Expand and Contract
  *   5 -> Alternate LEDs
  *   6 -> Inverted Chase
  *   7 -> LFSR Random Pattern
  *   8 -> Half-and-Half Alternate
  *
  ******************************************************************************
  */

/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/

#include "stm32f446xx_gpio_driver.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/*
 * Number of LEDs connected to the microcontroller.
 *
 * LEDs are connected to:
 * PB0, PB1, PB2, ... PB9
 */
#define LED_COUNT       10u


/*
 * First GPIO pin used for the LED array.
 *
 * PB0 is the first LED pin.
 */
#define LED_FIRST       0u


/*
 * Mask representing all 10 LED pins.
 *
 * LED_COUNT = 10
 *
 * (1 << 10) - 1 = 0x3FF
 *
 * Therefore:
 * LED_MASK = 0x000003FF
 *
 * Bits 0-9 correspond to PB0-PB9.
 */
#define LED_MASK        (((1u << LED_COUNT) - 1u) << LED_FIRST)


/*
 * Delay used between LED pattern steps.
 *
 * Unit: milliseconds
 *
 * 120 ms gives a visible movement speed for the LED patterns.
 */
#define STEP_MS         120u


/*
 * User button is connected to PC13.
 *
 * On many STM32 development boards, the user button is active LOW.
 *
 * Therefore:
 *   PC13 = 1 -> Button released
 *   PC13 = 0 -> Button pressed
 */
#define BUTTON_PIN      13u

/* USER CODE END PD */


/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */


/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/*
 * This flag is set by the EXTI interrupt when the button is pressed.
 *
 * volatile is used because this variable is modified inside the
 * interrupt service routine.
 */
volatile uint8_t button_event = 0;

/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/*
 * LED initialization function.
 *
 * Configures PB0-PB9 as GPIO outputs.
 */
static void leds_init(void);


/*
 * Writes a 10-bit value to the LED outputs.
 *
 * Bit 0 -> PB0
 * Bit 1 -> PB1
 * ...
 * Bit 9 -> PB9
 */
static void leds_write(uint32_t value);


/*
 * Initializes the user push button on PC13.
 */
static void button_init(void);


/*
 * Reads the current state of the push button.
 *
 * Returns:
 *   0 -> Button pressed
 *   1 -> Button released
 */
static uint8_t button_pressed(void);


/*
 * Blocking delay used instead of delay_ms().
 */
static void delay_ms(uint32_t ms);


/*
 * Delay which checks whether a button interrupt occurred.
 */
static uint8_t delay_ms_check_button(uint32_t ms);


/* LED pattern functions */

/* Pattern 1: Single LED moves from left to right. */
static void pattern_chase(uint32_t rounds);


/* Pattern 2: LED moves forward and backward. */
static void pattern_knight_rider(uint32_t rounds);


/* Pattern 3: LED bar fills and drains. */
static void pattern_bar_fill(uint32_t rounds);


/* Pattern 4: Displays binary counting values. */
static void pattern_binary_count(uint32_t upto);


/* Pattern 5: Two LEDs expand outward and move inward. */
static void pattern_expand(uint32_t rounds);


/* Pattern 6: Alternate even and odd LEDs. */
static void pattern_alternate(uint32_t rounds);


/* Pattern 7: All LEDs ON except one moving LED. */
static void pattern_invert_chase(uint32_t rounds);


/* Pattern 8: Pseudo-random pattern generated using LFSR. */
static void pattern_lfsr(uint32_t frames);


/* Pattern 9: Lower and upper halves alternate. */
static void pattern_half_alternate(uint32_t rounds);

/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  * @retval int
  */

int main(void)
{
    /*
     * Variable used to keep track of the currently selected LED pattern.
     *
     * Pattern numbers:
     *   0 -> Chase
     *   1 -> Knight Rider
     *   2 -> Bar Fill
     *   3 -> Binary Count
     *   4 -> Expand
     *   5 -> Alternate
     *   6 -> Inverted Chase
     *   7 -> LFSR
     *   8 -> Half Alternate
     */
    uint32_t pattern = 0;


    /* MCU Configuration------------------------------------------------------*/

    /*
     * The custom GPIO driver is used instead of the HAL.
     *
     * GPIO clocks and GPIO pin configuration are handled by the
     * GPIO driver through GPIO_PeriClockControl() and GPIO_Init().
     */

    /* USER CODE BEGIN 2 */

    /*
     * Configure PB0-PB9 as LED outputs.
     */
    leds_init();


    /*
     * Configure PC13 as the user button input.
     */
    button_init();


    /*
     * Initially turn all LEDs OFF.
     *
     * Passing 0 means all ten LED bits are cleared.
     */
    leds_write(0u);

    /* USER CODE END 2 */


    /* Infinite loop */

    /* USER CODE BEGIN WHILE */

    while (1)
    {
        /*
         * Execute the currently selected pattern.
         *
         * The pattern normally runs until it finishes.
         *
         * If the button interrupt occurs while the pattern is running,
         * button_event becomes 1 and the pattern is stopped.
         */

        switch (pattern)
        {
            /*
             * Pattern 0:
             * Single LED moves from PB0 to PB9.
             */
            case 0:

                pattern_chase(1);

                break;


            /*
             * Pattern 1:
             * LED moves from PB0 to PB9 and then back.
             */
            case 1:

                pattern_knight_rider(1);

                break;


            /*
             * Pattern 2:
             * LED bar fills and then drains.
             */
            case 2:

                pattern_bar_fill(1);

                break;


            /*
             * Pattern 3:
             * Display binary values from 0 to 63.
             */
            case 3:

                pattern_binary_count(63);

                break;


            /*
             * Pattern 4:
             * Two LEDs start at the center and expand outward.
             */
            case 4:

                pattern_expand(1);

                break;


            /*
             * Pattern 5:
             * Even and odd LEDs alternate.
             */
            case 5:

                pattern_alternate(3);

                break;


            /*
             * Pattern 6:
             * All LEDs remain ON except one moving LED.
             */
            case 6:

                pattern_invert_chase(1);

                break;


            /*
             * Pattern 7:
             * Generate pseudo-random LED patterns using an LFSR.
             */
            case 7:

                pattern_lfsr(60);

                break;


            /*
             * Pattern 8:
             * Lower five and upper five LEDs alternate.
             */
            case 8:

                pattern_half_alternate(3);

                break;


            /*
             * Safety case.
             *
             * If pattern somehow gets an invalid value,
             * nothing is executed.
             */
            default:

                pattern = 0;

                break;
        }


        /*
         * Check whether the button interrupt occurred.
         *
         * If button_event is 1, the current pattern was interrupted.
         */
        if (button_event)
        {
            /*
             * Clear the event flag.
             */
            button_event = 0u;


            /*
             * Turn all LEDs OFF.
             *
             * This makes the interrupt action clearly visible.
             */
            leds_write(0u);


            /*
             * Keep all LEDs OFF for 500 ms.
             *
             * This gives a visible pause between patterns.
             */
            delay_ms(2000);


            /*
             * Move to the next pattern.
             *
             * Pattern sequence:
             *
             *   0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 0
             */
            pattern = (pattern + 1u) % 9u;
        }


        /*
         * If the pattern completed normally without a button press,
         * move to the next pattern as well.
         */
        else
        {
            /*
             * Turn all LEDs OFF after the pattern finishes.
             *
             * This gives a clear separation between two patterns.
             */
            leds_write(0u);


            /*
             * Move to the next pattern.
             *
             * Pattern sequence:
             *
             *   0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 0
             *
             * The modulo operation makes the sequence wrap back to zero.
             */
            pattern = (pattern + 1u) % 9u;


            /*
             * Short delay before starting the next pattern.
             */
            delay_ms(100);
        }
    }

    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */

    /*
     * The program never normally reaches this point because
     * the application runs inside the infinite while(1) loop.
     */

    /* USER CODE END 3 */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */


/**
  * @brief GPIO Initialization Function
  * @retval None
  */


/* USER CODE BEGIN 4 */


/**
  * @brief Initialize LEDs connected to PB0 - PB9.
  * @retval None
  *
  * PB0-PB9 are configured as:
  *
  *   - General-purpose output
  *   - Push-pull
  *   - Low speed
  *   - No pull-up / pull-down
  */

static void leds_init(void)
{
    /*
     * Loop variable used to configure each LED pin.
     */
    uint32_t pin;

    GPIO_Handle_t GPIOB_Handle;


    /*
     * Enable GPIOB clock.
     *
     * The custom GPIO driver handles the GPIO peripheral clock.
     */
    GPIO_PeriClockControl(GPIOB, ENABLE);

    GPIOB_Handle.pGPIOx = GPIOB;


    /*
     * Configure PB0-PB9 one pin at a time.
     */
    for (pin = LED_FIRST;
         pin < (LED_FIRST + LED_COUNT);
         pin++)
    {
        /*
         * Configure each LED pin as:
         *
         *   - General-purpose output
         *   - Push-pull
         *   - Low speed
         *   - No pull-up / pull-down
         */
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinNumber = pin;
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
        GPIOB_Handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;


        /*
         * Initialize the current LED pin through the custom driver.
         */
        GPIO_Init(&GPIOB_Handle);
    }


    /*
     * Initially turn all LEDs OFF.
     *
     * Clearing the LED pins does not affect other GPIOB output pins.
     */
    leds_write(0u);
}


/**
  * @brief Write a 10-bit value to PB0 - PB9.
  * @param value 10-bit LED pattern.
  * @retval None
  *
  * Example:
  *
  *   value = 0x001
  *   PB0 = ON
  *   PB1-PB9 = OFF
  *
  *   value = 0x3FF
  *   PB0-PB9 = ON
  */

static void leds_write(uint32_t value)
{
    uint32_t pin;


    /*
     * Keep only the lower 10 bits.
     *
     * This prevents accidental modification of GPIO pins
     * outside PB0-PB9.
     */
    value &= LED_MASK;


    /*
     * Write each LED pin through the custom GPIO driver.
     *
     * Bit 0 -> PB0
     * Bit 1 -> PB1
     * ...
     * Bit 9 -> PB9
     */
    for (pin = 0; pin < LED_COUNT; pin++)
    {
        if (value & (1u << pin))
        {
            GPIO_WriteToOutputPin(GPIOB, pin, GPIO_PIN_SET);
        }
        else
        {
            GPIO_WriteToOutputPin(GPIOB, pin, GPIO_PIN_RESET);
        }
    }
}


/**
  * @brief Initialize USER button on PC13.
  * @retval None
  *
  * PC13 is configured as an interrupt input with an internal pull-up.
  *
  * Therefore:
  *
  *   Button released -> PC13 = HIGH
  *   Button pressed  -> PC13 = LOW
  */

static void button_init(void)
{
    GPIO_Handle_t GPIOC_Handle;


    /*
     * Enable GPIOC clock.
     */
    GPIO_PeriClockControl(GPIOC, ENABLE);

    GPIOC_Handle.pGPIOx = GPIOC;


    /*
     * Configure PC13 as an interrupt input with an internal pull-up.
     *
     * PC13 is active LOW:
     *
     *   Button released -> PC13 = HIGH
     *   Button pressed  -> PC13 = LOW
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;

    /*
     * GPIO_MODE_IT_FT means interrupt on falling edge.
     *
     * Since the button is active LOW:
     *
     * Released -> HIGH
     * Pressed  -> LOW
     *
     * Therefore HIGH -> LOW creates the interrupt.
     */
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;

    GPIOC_Handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GPIOC_Handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;


    /*
     * Initialize PC13 through the custom GPIO driver.
     */
    GPIO_Init(&GPIOC_Handle);


    /*
     * Enable the EXTI15_10 interrupt in the NVIC.
     *
     * PC13 belongs to EXTI lines 10-15.
     */
    GPIO_IRQConfig(IRQ_NO_EXTI15_10,
                   NVIC_IRQ_PRI15,
                   ENABLE);
}


/**
  * @brief Check USER button state.
  * @retval uint8_t
  *
  * Returns:
  *
  *   0 -> Button IS pressed
  *   1 -> Button is NOT pressed
  *
  * PC13 is active LOW.
  */

static uint8_t button_pressed(void)
{
    /*
     * Read the PC13 input bit through the custom GPIO driver.
     *
     * If the bit is zero, the button is pressed.
     */
    if (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET)
    {
        /*
         * Button pressed.
         */
        return 0u;
    }


    /*
     * Button released.
     */
    return 1u;
}


/**
  * @brief Delay which checks for button interrupt.
  * @param ms Delay time in milliseconds.
  * @retval uint8_t
  *
  * Returns:
  *
  *   0 -> Delay completed normally
  *   1 -> Button interrupt occurred
  */

static uint8_t delay_ms_check_button(uint32_t ms)
{
    /*
     * Instead of waiting for the complete delay at once,
     * divide the delay into 1 ms pieces.
     *
     * This allows the program to notice the button interrupt
     * while a pattern is running.
     */
    while (ms--)
    {
        /*
         * Wait for approximately 1 ms.
         */
        delay_ms(1);


        /*
         * Check whether the button interrupt occurred.
         */
        if (button_event)
        {
            /*
             * Button interrupt detected.
             */
            return 1u;
        }
    }


    /*
     * Delay completed without an interrupt.
     */
    return 0u;
}


/**
  * @brief Pattern 1 - LED Chase.
  * @param rounds Number of times the complete pattern is repeated.
  * @retval None
  *
  * A single LED moves from left to right:
  *
  * PB0 -> PB1 -> PB2 -> ... -> PB9
  */

static void pattern_chase(uint32_t rounds)
{
    /*
     * r     -> Current repetition.
     * i     -> Current LED number.
     */
    uint32_t r, i;


    /*
     * Repeat the complete chase for the requested number of rounds.
     */
    for (r = 0; r < rounds; r++)
    {
        /*
         * Move the illuminated LED from PB0 to PB9.
         */
        for (i = 0; i < LED_COUNT; i++)
        {
            /*
             * Shift 1 to the current LED position.
             *
             * Example:
             * i = 0 -> 0000000001 -> PB0 ON
             * i = 1 -> 0000000010 -> PB1 ON
             * i = 2 -> 0000000100 -> PB2 ON
             */
            leds_write(1u << i);


            /*
             * Wait before moving to the next LED.
             */
            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }
    }
}


/**
  * @brief Pattern 2 - Knight Rider.
  * @param rounds Number of times the complete pattern is repeated.
  * @retval None
  *
  * LED movement:
  *
  * PB0 -> PB1 -> ... -> PB9
  * PB8 -> PB7 -> ... -> PB1
  *
  * PB9 and PB0 are not repeated during the return movement.
  */

static void pattern_knight_rider(uint32_t rounds)
{
    /*
     * r -> Current repetition.
     * i -> Current LED index.
     *
     * int32_t is used for i because the second loop counts downward.
     */
    uint32_t r;
    int32_t i;


    for (r = 0; r < rounds; r++)
    {
        /*
         * Forward movement:
         *
         * PB0 -> PB1 -> ... -> PB9
         */
        for (i = 0;
             i < (int32_t)LED_COUNT;
             i++)
        {
            /*
             * Turn ON only the current LED.
             */
            leds_write(1u << i);


            /*
             * Wait before moving to the next position.
             */
            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }


        /*
         * Return movement:
         *
         * PB8 -> PB7 -> ... -> PB1
         *
         * PB9 is skipped because it was already displayed
         * at the end of the forward movement.
         *
         * PB0 is also skipped because the next cycle starts
         * from PB0.
         */
        for (i = (int32_t)LED_COUNT - 2;
             i > 0;
             i--)
        {
            /*
             * Turn ON only the current LED.
             */
            leds_write(1u << i);


            /*
             * Wait before moving backward.
             */
            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }
    }
}


/**
  * @brief Pattern 3 - LED Bar Fill and Drain.
  * @param rounds Number of times the pattern is repeated.
  * @retval None
  *
  * First all LEDs are gradually turned ON:
  *
  * 0000000000
  * 0000000001
  * 0000000011
  * ...
  * 1111111111
  *
  * Then they are gradually turned OFF.
  */

static void pattern_bar_fill(uint32_t rounds)
{
    /*
     * r -> Current repetition.
     * i -> Number of LEDs currently involved.
     */
    uint32_t r, i;


    for (r = 0; r < rounds; r++)
    {
        /*
         * -------------------------
         * Fill section
         * -------------------------
         *
         * Turn LEDs ON from PB0 towards PB9.
         */
        for (i = 0; i <= LED_COUNT; i++)
        {
            /*
             * (1 << i) - 1 creates a sequence of lower bits set to 1.
             *
             * Examples:
             *
             * i = 0 -> 0000000000
             * i = 1 -> 0000000001
             * i = 2 -> 0000000011
             * i = 3 -> 0000000111
             *
             * At i = 10 all ten LEDs are ON.
             */
            leds_write((1u << i) - 1u);


            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }


        /*
         * -------------------------
         * Drain section
         * -------------------------
         *
         * Turn LEDs OFF from PB0 towards PB9.
         */
        for (i = LED_COUNT; i > 0; i--)
        {
            /*
             * Generate a mask where the required lower LEDs
             * are cleared and the remaining LEDs stay ON.
             *
             * LED_MASK limits the operation to PB0-PB9.
             */
            leds_write(
                LED_MASK &
                ~((1u << (LED_COUNT - i + 1u)) - 1u)
            );


            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }
    }
}


/**
  * @brief Pattern 4 - Binary Counter.
  * @param upto Last number to display.
  * @retval None
  *
  * The ten LEDs are treated as a 10-bit binary display.
  *
  * Example:
  *
  * Decimal 0  -> 0000000000
  * Decimal 1  -> 0000000001
  * Decimal 2  -> 0000000010
  * Decimal 3  -> 0000000011
  *
  * In this program the counter runs from 0 to 63.
  */

static void pattern_binary_count(uint32_t upto)
{
    /*
     * Variable containing the current binary number.
     */
    uint32_t n;


    /*
     * Display every value from 0 to the requested limit.
     */
    for (n = 0; n <= upto; n++)
    {
        /*
         * The binary representation of n is directly
         * displayed on PB0-PB9.
         */
        leds_write(n);


        /*
         * Use a shorter delay so that the binary counter
         * changes faster than the normal LED patterns.
         */
        if (delay_ms_check_button(STEP_MS / 2u))
        {
            return;
        }
    }
}


/**
  * @brief Pattern 5 - Expand and Contract.
  * @param rounds Number of times the pattern is repeated.
  * @retval None
  *
  * Two LEDs start near the center:
  *
  *        PB4  PB5
  *         ON  ON
  *
  * Then they move outward:
  *
  * PB3 and PB6
  * PB2 and PB7
  * PB1 and PB8
  * PB0 and PB9
  *
  * Finally they move back toward the center.
  */

static void pattern_expand(uint32_t rounds)
{
    /*
     * r -> Current repetition.
     * s -> Expansion step.
     */
    uint32_t r, s;


    for (r = 0; r < rounds; r++)
    {
        /*
         * -------------------------
         * Outward movement
         * -------------------------
         */
        for (s = 0;
             s < (LED_COUNT / 2u);
             s++)
        {
            /*
             * Left LED:
             *
             * 4, 3, 2, 1, 0
             *
             * Right LED:
             *
             * 5, 6, 7, 8, 9
             */
            leds_write(
                (1u << (4u - s)) |
                (1u << (5u + s))
            );


            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }


        /*
         * -------------------------
         * Inward movement
         * -------------------------
         *
         * Start from the outermost position and move back
         * toward PB4 and PB5.
         */
        for (s = (LED_COUNT / 2u);
             s > 0;
             s--)
        {
            leds_write(
                (1u << (4u - (s - 1u))) |
                (1u << (5u + (s - 1u)))
            );


            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }
    }
}


/**
  * @brief Pattern 6 - Alternate LEDs.
  * @param rounds Number of times the pattern is repeated.
  * @retval None
  *
  * Two groups of LEDs alternate:
  *
  * Group 1:
  *   PB0, PB2, PB4, PB6, PB8
  *
  * Group 2:
  *   PB1, PB3, PB5, PB7, PB9
  */

static void pattern_alternate(uint32_t rounds)
{
    /*
     * Binary mask for even-numbered LED positions:
     *
     * PB0, PB2, PB4, PB6, PB8
     *
     * 0x155 = 0101010101
     */
    const uint32_t even = 0x155u;


    /*
     * Binary mask for odd-numbered LED positions:
     *
     * PB1, PB3, PB5, PB7, PB9
     *
     * 0x2AA = 1010101010
     */
    const uint32_t odd  = 0x2AAu;


    /*
     * Loop counter.
     */
    uint32_t r;


    for (r = 0; r < rounds; r++)
    {
        /*
         * Turn ON even-numbered LEDs.
         */
        leds_write(even);


        /*
         * Keep them ON for two pattern steps.
         */
        if (delay_ms_check_button(STEP_MS * 2u))
        {
            return;
        }


        /*
         * Turn ON odd-numbered LEDs.
         */
        leds_write(odd);


        /*
         * Keep them ON before switching back.
         */
        if (delay_ms_check_button(STEP_MS * 2u))
        {
            return;
        }
    }
}


/**
  * @brief Pattern 7 - Inverted Chase.
  * @param rounds Number of times the pattern is repeated.
  * @retval None
  *
  * All LEDs are ON except one.
  *
  * Example:
  *
  * 1111111110
  * 1111111101
  * 1111111011
  * ...
  *
  * The OFF position moves from PB0 to PB9.
  */

static void pattern_invert_chase(uint32_t rounds)
{
    /*
     * r -> Current repetition.
     * i -> Current LED position.
     */
    uint32_t r, i;


    for (r = 0; r < rounds; r++)
    {
        for (i = 0; i < LED_COUNT; i++)
        {
            /*
             * 1 << i creates a value containing one '1'
             * at the current LED position.
             *
             * XOR with LED_MASK inverts that bit pattern.
             *
             * Result:
             * Every LED is ON except the current LED.
             */
            leds_write(
                (1u << i) ^ LED_MASK
            );


            if (delay_ms_check_button(STEP_MS))
            {
                return;
            }
        }
    }
}


/**
  * @brief Pattern 8 - LFSR Pseudo-Random LED Pattern.
  * @param frames Number of LED states to display.
  * @retval None
  *
  * LFSR stands for Linear Feedback Shift Register.
  *
  * It generates a deterministic sequence that appears
  * pseudo-random.
  *
  * The current 10-bit LFSR state is directly displayed
  * on the ten LEDs.
  */

static void pattern_lfsr(uint32_t frames)
{
    /*
     * LFSR state.
     *
     * static means the value is retained between function calls.
     *
     * Starting value must not be zero because an LFSR can
     * become stuck at zero.
     */
    static uint32_t state = 1u;


    /*
     * Frame counter.
     */
    uint32_t f;


    /*
     * Stores the newly calculated feedback bit.
     */
    uint32_t new_bit;


    /*
     * Generate the requested number of LFSR frames.
     */
    for (f = 0; f < frames; f++)
    {
        /*
         * Display the current LFSR state on the LEDs.
         */
        leds_write(state);


        /*
         * Keep the current state visible.
         */
        if (delay_ms_check_button(STEP_MS))
        {
            return;
        }


        /*
         * Calculate the new feedback bit.
         *
         * The selected taps are:
         *
         *   bit 9
         *   bit 6
         *
         * XORing the two bits creates the feedback bit.
         */
        new_bit =
            ((state >> 9) ^
             (state >> 6)) & 1u;


        /*
         * Shift the state left by one position.
         *
         * The newly calculated feedback bit is inserted
         * into bit 0.
         *
         * LED_MASK ensures that only 10 bits are retained.
         */
        state =
            ((state << 1) |
             new_bit) & LED_MASK;


        /*
         * An LFSR must not remain at zero.
         *
         * If the state somehow becomes zero, restore it
         * to a non-zero starting state.
         */
        if (state == 0u)
        {
            state = 1u;
        }
    }
}


/**
  * @brief Pattern 9 - Half-and-Half Alternate.
  * @param rounds Number of times the pattern is repeated.
  * @retval None
  *
  * First state:
  *
  *   PB0-PB4 = ON
  *   PB5-PB9 = OFF
  *
  * Second state:
  *
  *   PB0-PB4 = OFF
  *   PB5-PB9 = ON
  */

static void pattern_half_alternate(uint32_t rounds)
{
    /*
     * Loop counter.
     */
    uint32_t r;


    for (r = 0; r < rounds; r++)
    {
        /*
         * -------------------------
         * Lower half ON
         * -------------------------
         *
         * PB0-PB4 are ON.
         * PB5-PB9 are OFF.
         *
         * 0x01F = 0000011111
         */
        leds_write(0x01Fu);


        /*
         * Keep the lower half ON.
         */
        if (delay_ms_check_button(STEP_MS * 2u))
        {
            return;
        }


        /*
         * -------------------------
         * Upper half ON
         * -------------------------
         *
         * PB0-PB4 are OFF.
         * PB5-PB9 are ON.
         *
         * 0x3E0 = 1111100000
         */
        leds_write(0x3E0u);


        /*
         * Keep the upper half ON.
         */
        if (delay_ms_check_button(STEP_MS * 2u))
        {
            return;
        }
    }
}


/*
 * EXTI15_10_IRQHandler
 *
 * PC13 belongs to EXTI lines 10 to 15.
 *
 * When the button is pressed, the falling edge on PC13
 * generates an interrupt and this function is executed.
 */
void EXTI15_10_IRQHandler(void)
{
    /*
     * Clear the EXTI interrupt pending flag through
     * the custom GPIO driver.
     */
    GPIO_IRQHandling(GPIO_PIN_NO_13);


    /*
     * Inform the main program that the button was pressed.
     */
    button_event = 1u;
}


/* USER CODE END 4 */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  *
  * If a HAL initialization/configuration function fails,
  * the program enters this function and remains here.
  */


/*
 * Blocking delay.
 */
static void delay_ms(uint32_t ms)
{
    uint32_t j;

    for (j = 0; j < ms; j++)
    {
        for (volatile uint32_t i = 0; i < 1250u; i++)
        {
            ;
        }
    }
}


void Error_Handler(void)
{
    /*
     * Infinite loop.
     *
     * The program stays here until the MCU is reset.
     */
    while (1)
    {
        /*
         * Error state.
         *
         * You could optionally add an error LED indication here.
         */
    }
}


#ifdef USE_FULL_ASSERT

/**
  * @brief Reports the name of the source file and source line
  *        where the assert_param error has occurred.
  * @param file Pointer to the source file name.
  * @param line Line number of the failed assertion.
  * @retval None
  *
  * This function is used when FULL ASSERT is enabled.
  */

void assert_failed(uint8_t *file, uint32_t line)
{
    /*
     * Add debugging code here if required.
     *
     * For example, the file and line number could be sent
     * through UART for debugging.
     */
}


#endif /* USE_FULL_ASSERT */
