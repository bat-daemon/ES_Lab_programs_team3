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

#include "main.h"


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

/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);


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
     * Initialize the HAL library.
     *
     * This initializes the SysTick timer and other HAL-related functions.
     */
    HAL_Init();


    /*
     * Configure the system clock.
     *
     * This function configures the oscillator, PLL and bus clocks.
     */
    SystemClock_Config();


    /*
     * Initialize GPIO peripherals.
     *
     * GPIO clocks are enabled here.
     */
    MX_GPIO_Init();


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
         * Wait for the USER button to be pressed.
         *
         * PC13 is active LOW:
         *
         *   1 -> Button is NOT pressed
         *   0 -> Button IS pressed
         *
         * Therefore, button_pressed() returns 1 when the button is released.
         */

        /*
         * Wait here until the button becomes pressed.
         *
         * While the button is released, button_pressed() returns 1.
         */
        while (button_pressed())
        {
            /*
             * Do nothing while waiting for the button press.
             */
        }


        /*
         * Small delay to debounce the mechanical button.
         *
         * Mechanical switches can produce multiple rapid transitions
         * when pressed. A short delay helps prevent false detection.
         */
        HAL_Delay(20);


        /*
         * Wait until the button is released.
         *
         * This ensures that one complete press-and-release action
         * corresponds to only one pattern selection.
         */
        while (!button_pressed())
        {
            /*
             * Do nothing while the button is still pressed.
             */
        }


        /*
         * Small delay after button release for debounce.
         */
        HAL_Delay(20);


        /*
         * The button has now been pressed and released.
         *
         * Execute the currently selected pattern once.
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
                break;
        }


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
         * Short delay before accepting the next button press.
         */
        HAL_Delay(100);
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
void SystemClock_Config(void)
{
    /*
     * Structure containing oscillator configuration.
     *
     * Initializing with {0} clears all fields before configuration.
     */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};


    /*
     * Structure containing CPU and peripheral clock configuration.
     */
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    /*
     * Enable the clock for the power control peripheral.
     *
     * This is required before configuring power-related settings.
     */
    __HAL_RCC_PWR_CLK_ENABLE();


    /*
     * Configure the voltage scaling.
     *
     * Scale 3 is selected here.
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);


    /*
     * Select HSI as the oscillator.
     *
     * HSI = Internal High-Speed oscillator.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;


    /*
     * Turn ON the HSI oscillator.
     */
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;


    /*
     * Use the default HSI calibration value.
     */
    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;


    /*
     * Enable the PLL.
     *
     * The PLL is used to increase the system clock frequency.
     */
    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;


    /*
     * Select HSI as the PLL input source.
     */
    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSI;


    /*
     * PLL input divider.
     *
     * The actual effect depends on the MCU clock architecture.
     */
    RCC_OscInitStruct.PLL.PLLM = 16;


    /*
     * PLL multiplication factor.
     */
    RCC_OscInitStruct.PLL.PLLN = 336;


    /*
     * PLL output divider for the main system clock.
     */
    RCC_OscInitStruct.PLL.PLLP =
        RCC_PLLP_DIV4;


    /*
     * PLL divider for the peripheral clock output.
     */
    RCC_OscInitStruct.PLL.PLLQ = 2;


    /*
     * PLL divider for the additional PLL output.
     */
    RCC_OscInitStruct.PLL.PLLR = 2;


    /*
     * Apply the oscillator configuration.
     *
     * If configuration fails, enter the error handler.
     */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    /*
     * Select which clock domains will be configured.
     *
     * HCLK   -> AHB clock
     * SYSCLK -> System clock
     * PCLK1  -> APB1 peripheral clock
     * PCLK2  -> APB2 peripheral clock
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;


    /*
     * Use the PLL output as the system clock source.
     */
    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;


    /*
     * AHB clock divider.
     *
     * DIV1 means HCLK = SYSCLK.
     */
    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;


    /*
     * APB1 clock divider.
     *
     * DIV2 means APB1 clock is half the HCLK frequency.
     */
    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;


    /*
     * APB2 clock divider.
     *
     * DIV1 means APB2 clock is equal to HCLK.
     */
    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    /*
     * Apply the bus clock configuration.
     *
     * FLASH_LATENCY_2 specifies the Flash wait-state configuration.
     */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                            FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    /*
     * Enable the GPIO peripheral clocks.
     *
     * GPIO peripherals must have their clocks enabled before
     * their registers can be accessed.
     */

    /* Enable GPIOC clock - used by the PC13 button. */
    __HAL_RCC_GPIOC_CLK_ENABLE();


    /* Enable GPIOH clock. */
    __HAL_RCC_GPIOH_CLK_ENABLE();


    /* Enable GPIOA clock. */
    __HAL_RCC_GPIOA_CLK_ENABLE();


    /* Enable GPIOB clock - used by PB0-PB9 LEDs. */
    __HAL_RCC_GPIOB_CLK_ENABLE();


    /*
     * LED and button GPIO pins are configured manually below
     * in leds_init() and button_init().
     *
     * This is intentional because the program directly accesses
     * GPIO registers instead of using HAL_GPIO_Init().
     */
}


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


    /*
     * Enable GPIOB clock.
     *
     * Bit 1 of AHB1ENR corresponds to GPIOB.
     */
    RCC->AHB1ENR |= (1u << 1);


    /*
     * Configure PB0-PB9 one pin at a time.
     */
    for (pin = LED_FIRST;
         pin < (LED_FIRST + LED_COUNT);
         pin++)
    {
        /*
         * Each GPIO pin uses two MODER bits.
         *
         * First clear the existing mode bits for this pin.
         */
        GPIOB->MODER &= ~(3u << (2u * pin));


        /*
         * Set MODER = 01.
         *
         * 01 means:
         * General-purpose output mode.
         */
        GPIOB->MODER |= (1u << (2u * pin));


        /*
         * Configure output type as push-pull.
         *
         * OTYPER = 0 -> Push-pull
         * OTYPER = 1 -> Open-drain
         */
        GPIOB->OTYPER &= ~(1u << pin);


        /*
         * Configure low-speed output.
         *
         * Clearing both OSPEEDR bits selects low speed.
         */
        GPIOB->OSPEEDR &= ~(3u << (2u * pin));


        /*
         * Disable internal pull-up and pull-down resistors.
         *
         * PUPDR = 00 -> No pull-up / pull-down
         */
        GPIOB->PUPDR &= ~(3u << (2u * pin));
    }


    /*
     * Initially turn all LEDs OFF.
     *
     * Clearing LED_MASK clears PB0-PB9 without affecting
     * other GPIOB output pins.
     */
    GPIOB->ODR &= ~LED_MASK;
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
    /*
     * Temporary variable used to modify the GPIO output register.
     */
    uint32_t reg;


    /*
     * Keep only the lower 10 bits.
     *
     * This prevents accidental modification of GPIO pins
     * outside PB0-PB9.
     */
    value &= LED_MASK;


    /*
     * Read the current GPIOB output data register.
     *
     * Reading first allows us to modify only PB0-PB9
     * while preserving the state of other GPIOB pins.
     */
    reg = GPIOB->ODR;


    /*
     * Clear the existing PB0-PB9 values.
     */
    reg &= ~LED_MASK;


    /*
     * Insert the new LED pattern into PB0-PB9.
     */
    reg |= value;


    /*
     * Write the updated value back to the GPIO output register.
     */
    GPIOB->ODR = reg;
}


/**
  * @brief Initialize USER button on PC13.
  * @retval None
  *
  * PC13 is configured as an input with an internal pull-up.
  *
  * Therefore:
  *
  *   Button released -> PC13 = HIGH
  *   Button pressed  -> PC13 = LOW
  */
static void button_init(void)
{
    /*
     * Enable GPIOC clock.
     *
     * Bit 2 of AHB1ENR corresponds to GPIOC.
     */
    RCC->AHB1ENR |= (1u << 2);


    /*
     * Configure PC13 as input.
     *
     * Each GPIO pin uses two MODER bits.
     *
     * 00 -> Input
     */
    GPIOC->MODER &= ~(3u << (2u * BUTTON_PIN));


    /*
     * Clear the existing pull-up/pull-down configuration.
     */
    GPIOC->PUPDR &= ~(3u << (2u * BUTTON_PIN));


    /*
     * Enable the internal pull-up resistor.
     *
     * PUPDR = 01 -> Pull-up
     */
    GPIOC->PUPDR |= (1u << (2u * BUTTON_PIN));
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
     * Read the PC13 input bit.
     *
     * If the bit is zero, the button is pressed.
     */
    if ((GPIOC->IDR & (1u << BUTTON_PIN)) == 0u)
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
            HAL_Delay(STEP_MS);
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
            HAL_Delay(STEP_MS);
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
            HAL_Delay(STEP_MS);
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


            HAL_Delay(STEP_MS);
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


            HAL_Delay(STEP_MS);
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
        HAL_Delay(STEP_MS / 2u);
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


            HAL_Delay(STEP_MS);
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


            HAL_Delay(STEP_MS);
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
        HAL_Delay(STEP_MS * 2u);


        /*
         * Turn ON odd-numbered LEDs.
         */
        leds_write(odd);


        /*
         * Keep them ON before switching back.
         */
        HAL_Delay(STEP_MS * 2u);
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


            HAL_Delay(STEP_MS);
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
        HAL_Delay(STEP_MS);


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
        HAL_Delay(STEP_MS * 2u);


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
        HAL_Delay(STEP_MS * 2u);
    }
}


/* USER CODE END 4 */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  *
  * If a HAL initialization/configuration function fails,
  * the program enters this function and remains here.
  */
void Error_Handler(void)
{
    /*
     * Disable interrupts so that the MCU remains in a controlled
     * error state.
     */
    __disable_irq();


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
