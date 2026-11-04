/*
 ******************************************************************************
 * main.c  -  10 LED SEQUENTIAL DISPLAY on NUCLEO-F446RE
 *
 * 10 LEDs on PA0 .. PA9. Eight display patterns run one after another,
 * then the whole show repeats.
 *
 * Every pattern is just a 10 bit number written to GPIOA->ODR.
 * That is the entire trick. Once you see the LEDs as 10 bits, the
 * patterns become simple arithmetic.
 *
 * Project setup:
 *   Board Selector -> NUCLEO-F446RE -> answer "No" to
 *   "Initialize all peripherals with their default Mode?"
 ******************************************************************************
 *
 *                        WIRING  (active high, LED lights when pin = 1)
 *
 *   PA0  ---[ 470R ]---|>|--- GND        |>|  = LED, long leg (anode)
 *   PA1  ---[ 470R ]---|>|--- GND               points at the resistor
 *   PA2  ---[ 470R ]---|>|--- GND
 *   PA3  ---[ 470R ]---|>|--- GND        Repeat the same three parts
 *   PA4  ---[ 470R ]---|>|--- GND        ten times. Every LED needs
 *   PA5  ---[ 470R ]---|>|--- GND        its OWN resistor. Do not
 *   PA6  ---[ 470R ]---|>|--- GND        share one resistor between
 *   PA7  ---[ 470R ]---|>|--- GND        several LEDs.
 *   PA8  ---[ 470R ]---|>|--- GND
 *   PA9  ---[ 470R ]---|>|--- GND
 *
 *   NUCLEO-64 pin locations
 *     PA0  CN7-28     PA5  CN5-6  (also drives on-board LED LD2)
 *     PA1  CN7-30     PA6  CN5-5
 *     PA2  CN10-35    PA7  CN5-4
 *     PA3  CN10-37    PA8  CN10-23
 *     PA4  CN7-32     PA9  CN10-21
 *     GND  CN6-6, CN7-8, CN7-20, CN7-22, CN10-9
 *
 * ---------------------------------------------------------------------------
 *  READ THESE THREE WARNINGS BEFORE WIRING
 * ---------------------------------------------------------------------------
 *
 *  1. NEVER USE PA13 OR PA14.
 *     They are SWDIO and SWCLK, the debug connection. Driving them as
 *     outputs disconnects your ST-LINK and you can no longer flash the
 *     board without an erase-and-recover. PA15 is JTDI, also best avoided.
 *     That is why this program stops at PA9.
 *
 *  2. PA2 AND PA3 ARE SHARED WITH THE ST-LINK VIRTUAL COM PORT.
 *     Solder bridges SB62 and SB63 connect them to the ST-LINK UART.
 *     PA3 is the worst case: the ST-LINK is driving it as an output at
 *     the same time as your MCU. It usually survives, but it is genuine
 *     pin contention and it will corrupt anything sent over the VCP.
 *
 *     Three choices:
 *       (a) Accept it for a short lab. Do not use printf or UART.
 *       (b) Open SB62 and SB63 to disconnect the ST-LINK properly.
 *       (c) Use PA4..PA12 with PA13/PA14 skipped, and change LED_FIRST
 *           below. Non contiguous pins make the bit maths uglier, which
 *           is why PA0..PA9 is the default here.
 *
 *  3. CURRENT LIMIT.
 *     Each STM32 pin can source 25 mA, but the whole VDD supply is
 *     limited to about 150 mA. With all 10 LEDs on at once:
 *          470 ohm  ->  about 3.5 mA each  ->  35 mA total   SAFE
 *          330 ohm  ->  about 5 mA each    ->  50 mA total   safe
 *          220 ohm  ->  about 7.5 mA each  ->  75 mA total   getting warm
 *          100 ohm  ->  about 16 mA each   ->  160 mA total  TOO MUCH
 *     Use 470 ohm. The LEDs are slightly dimmer and nothing overheats.
 *
 ******************************************************************************
 */

#include "stm32f446xx.h"
#include <stdint.h>

/* ==========================================================================
 * CONFIGURATION
 * ========================================================================== */

#define LED_COUNT   10u
#define LED_FIRST   0u                          /* first pin number, PA0    */

/* LED_MASK marks which bits of the port belong to us.
 *   (1 << 10) - 1  =  1024 - 1  =  1023  =  0x3FF  =  0b1111111111
 * Ten ones in a row, sitting at positions 0 to 9.
 */
#define LED_MASK    (((1u << LED_COUNT) - 1u) << LED_FIRST)   /* 0x000003FF */

#define STEP_MS     120u                        /* speed of the animation   */
#define BUTTON_PIN  13u

/* ==========================================================================
 * OBSERVABLES  -  put these in the Live Expressions view
 * ========================================================================== */

volatile uint32_t led_value   = 0;   /* the 10 bit pattern being shown      */
volatile uint32_t pattern_id  = 0;   /* which pattern is running, 1 to 8    */
volatile uint32_t frame_count = 0;   /* frames drawn since reset            */

//void SystemClock_Config(void);
static void leds_init(void);
static void leds_write(uint32_t value);

/* ==========================================================================
 * LOW LEVEL
 * ========================================================================== */

/**
 * Configure PA0..PA9 as push-pull outputs.
 *
 * MODER holds 2 bits per pin, so pin n sits at bits 2n and 2n+1.
 * We must CLEAR the 2 bit slot first, then write 01 into it.
 * OR alone would not work: if a slot already held 10, OR could never
 * clear that upper bit and we would end up with 11 (analog mode).
 */
static void leds_init(void)
{
    uint32_t pin;

    /* GPIOA clock on. Nothing in the port responds until this is set. */
    RCC->AHB1ENR |= (1u << 1);

    for (pin = LED_FIRST; pin < (LED_FIRST + LED_COUNT); pin++)
    {
        /* MODER: clear the 2 bit slot, then write 01 = output */
		// 1. Erase old 2-bit configuration
        GPIOB->MODER &= ~(3u << (2u * pin));
		// 2. Write 01 = OUTPUT
        GPIOB->MODER |=  (1u << (2u * pin));

        /* OTYPER: 0 = push-pull. One bit per pin.
         * Push-pull can drive both high and low, which is what an LED
         * to ground needs. Open-drain could only pull low.
         */
        GPIOB->OTYPER &= ~(1u << pin);

        /* OSPEEDR: 2 bits per pin. 00 = low speed.
         * LEDs do not need fast edges, and slow edges make less noise.
         */
        GPIOB->OSPEEDR &= ~(3u << (2u * pin));

        /* PUPDR: 2 bits per pin. 00 = no pull.
         * The pin is an output driving the LED, so no resistor needed.
         */
        GPIOB->PUPDR &= ~(3u << (2u * pin));
    }

    GPIOB->ODR &= ~LED_MASK;            /* start with all LEDs off */
}

static void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        for (volatile uint32_t i = 0; i < 4000u; i++)
        {
            __asm volatile ("nop");
        }
    }
}

static void button_init(void)
{
    RCC->AHB1ENR |= (1u << 2);  /* Enable GPIOC clock */

    GPIOC->MODER &= ~(3u << (2u * BUTTON_PIN)); /* PC13 input */

    GPIOC->PUPDR &= ~(3u << (2u * BUTTON_PIN));
    GPIOC->PUPDR |=  (1u << (2u * BUTTON_PIN)); /* Pull-up */
}

static uint8_t button_pressed(void)
{
    /* USER button B1 on PC13 is active LOW */
    return ((GPIOC->IDR & (1u << BUTTON_PIN)) == 0u);
}
/**
 * Show a 10 bit pattern on the LEDs.
 *
 * This is a READ-MODIFY-WRITE and it is deliberate. We must not disturb
 * PA10..PA15, especially the debug pins. So:
 *    step 1  clear only our 10 bits
 *    step 2  OR in the new pattern
 *
 * A plain  GPIOA->ODR = value;  would zero PA10..PA15 as a side effect.
 */
static void leds_write(uint32_t value)
{
    uint32_t reg;

    value &= LED_MASK;                  /* ignore anything outside our bits */

    reg  = GPIOB->ODR;                  /* read                             */
    reg &= ~LED_MASK;                   /* modify: blank our 10 bits        */
    reg |= value;                       /* modify: paint the new pattern    */
    GPIOB->ODR = reg;                   /* write                            */

    led_value = value;                  /* for the debugger                 */
    frame_count++;
}

/* ==========================================================================
 * PATTERN 1  -  CHASE
 * One LED runs from PA0 to PA9, then starts again from PA0.
 *
 * 0000000001  ->  0000000010  ->  0000000100  ->  ...  ->  1000000000
 *
 * Each frame is (1 << i). Shifting a single 1 left by one place moves
 * the lit LED one position along the row.
 * ========================================================================== */
static void pattern_chase(uint32_t rounds)
{
    uint32_t r, i;

    pattern_id = 1;

    for (r = 0; r < rounds; r++)
    {
        for (i = 0; i < LED_COUNT; i++)
        {
            leds_write(1u << i);
            delay_ms(400);
        }
    }
}

/* ==========================================================================
 * PATTERN 2  -  KNIGHT RIDER
 * The LED runs right, then bounces back left. 18 frames per full bounce.
 *
 * Going up   : i = 0,1,2 ... 9
 * Coming back: i = 8,7,6 ... 1
 * The two ends are skipped on the return trip, otherwise the end LED
 * would appear to pause for two frames.
 * ========================================================================== */
static void pattern_knight_rider(uint32_t rounds)
{
    uint32_t r;
    int32_t  i;

    pattern_id = 2;

    for (r = 0; r < rounds; r++)
    {
        for (i = 0; i < (int32_t)LED_COUNT; i++)
        {
            leds_write(1u << i);
            delay_ms(400);
        }
        for (i = (int32_t)LED_COUNT - 2; i > 0; i--)
        {
            leds_write(1u << i);
            delay_ms(400);
        }
    }
}

/* ==========================================================================
 * PATTERN 3  -  BAR FILL
 * The row fills up one LED at a time, then empties from the other end.
 *
 * 0000000000  ->  0000000001  ->  0000000011  ->  0000000111  -> ...
 *
 * (1 << i) - 1 gives i ones in a row. Worth checking by hand:
 *    (1 << 3) = 0000001000
 *    minus 1  = 0000000111     three ones
 * ========================================================================== */
static void pattern_bar_fill(uint32_t rounds)
{
    uint32_t r, i;

    pattern_id = 3;

    for (r = 0; r < rounds; r++)
    {
        for (i = 0; i <= LED_COUNT; i++)         /* fill  */
        {
            leds_write((1u << i) - 1u);
            delay_ms(400);
        }
        for (i = LED_COUNT; i > 0; i--)          /* drain */
        {
            leds_write(LED_MASK & ~((1u << (LED_COUNT - i + 1u)) - 1u));
            delay_ms(400);
        }
    }
}

/* ==========================================================================
 * PATTERN 4  -  BINARY COUNTER
 * Counts 0 to 1023 in binary. PA0 is the least significant bit.
 *
 * This is the one that teaches the most. Watch PA0 flip every frame,
 * PA1 every second frame, PA2 every fourth. That doubling IS binary.
 * ========================================================================== */
static void pattern_binary_count(uint32_t upto)
{
    uint32_t n;

    pattern_id = 4;

    for (n = 0; n <= upto; n++)
    {
        leds_write(n);
        delay_ms(400);
    }
}

/* ==========================================================================
 * PATTERN 5  -  EXPAND FROM CENTRE
 * Two LEDs start in the middle and walk outward to both ends.
 *
 * 0000110000  ->  0001001000  ->  0010000100  ->  0100000010  ->  1000000001
 *
 * Left LED  is (1 << (4 - s)), moving down.
 * Right LED is (1 << (5 + s)), moving up.
 * OR them together to light both at once.
 * ========================================================================== */
static void pattern_expand(uint32_t rounds)
{
    uint32_t r, s;

    pattern_id = 5;

    for (r = 0; r < rounds; r++)
    {
        for (s = 0; s < (LED_COUNT / 2u); s++)          /* outward */
        {
            leds_write((1u << (4u - s)) | (1u << (5u + s)));
            delay_ms(400);
        }
        for (s = (LED_COUNT / 2u); s > 0; s--)          /* inward  */
        {
            leds_write((1u << (4u - (s - 1u))) | (1u << (5u + (s - 1u))));
            delay_ms(400);
        }
    }
}

/* ==========================================================================
 * PATTERN 6  -  ODD / EVEN ALTERNATE
 * Even numbered LEDs flash, then odd numbered ones.
 *
 * even = 0101010101 = 0x155
 * odd  = 1010101010 = 0x2AA
 *
 * Check: 0x155 | 0x2AA = 0x3FF, so between them they cover all ten
 * and never overlap.
 * ========================================================================== */
static void pattern_alternate(uint32_t rounds)
{
    const uint32_t even = 0x155u;
    const uint32_t odd  = 0x2AAu;
    uint32_t r;

    pattern_id = 6;

    for (r = 0; r < rounds; r++)
    {
        leds_write(even);
        delay_ms(400);
        leds_write(odd);
        delay_ms(400);
    }
}

/* ==========================================================================
 * PATTERN 7  -  INVERT / NEGATIVE
 * Shows a chase, but with every LED flipped. The dark one moves instead.
 *
 * XOR with LED_MASK flips all ten bits at once:
 *    0000000001  ^  1111111111  =  1111111110
 * ========================================================================== */
static void pattern_invert_chase(uint32_t rounds)
{
    uint32_t r, i;

    pattern_id = 7;

    for (r = 0; r < rounds; r++)
    {
        for (i = 0; i < LED_COUNT; i++)
        {
            leds_write((1u << i) ^ LED_MASK);
            delay_ms(400);
        }
    }
}

/* ==========================================================================
 * PATTERN 8  -  PSEUDO RANDOM SPARKLE  (LFSR)
 *
 * A Linear Feedback Shift Register produces a sequence that looks random
 * but repeats exactly. With taps at bit 9 and bit 6 the period is 1023,
 * which is every possible 10 bit value except zero. Verified, not assumed.
 *
 * How it works:
 *   1. XOR two chosen bits together to get one new bit
 *   2. Shift the whole register left
 *   3. Drop the new bit into the empty position 0
 *
 * Never let the state reach 0. Zero XOR zero is zero forever, and the
 * display would freeze dark. That is why the seed is 1.
 * ========================================================================== */
static void pattern_lfsr(uint32_t frames)
{
    static uint32_t state = 1u;         /* seed. NEVER 0. */
    uint32_t f, new_bit;

    pattern_id = 8;

    for (f = 0; f < frames; f++)
    {
        leds_write(state);
        delay_ms(400);

        new_bit = ((state >> 9) ^ (state >> 6)) & 1u;
        state   = ((state << 1) | new_bit) & LED_MASK;

        if (state == 0u)                /* belt and braces */
        {
            state = 1u;
        }
    }
}

/* ==========================================================================
 * MAIN
 * ========================================================================== */
int main(void)
{
    //HAL_Init();
    //SystemClock_Config();
	uint8_t pattern = 0u;
    leds_init();
    button_init();
    /* Power-on test: all ten on, then all off.
     * If one LED stays dark here, it is wired backwards or its resistor
     * is not connected. Check that BEFORE blaming the code.
     */
    leds_write(LED_MASK);
    delay_ms(600);
    //HAL_Delay(600);
    leds_write(0);
    delay_ms(400);
    //HAL_Delay(400);

    while (1)
    {
        if (button_pressed())
        {
            delay_ms(30);  /* debounce */

            if (button_pressed())
            {
                while (button_pressed())
                {
                    /* Wait for button release */
                }

                delay_ms(30);  /* debounce release */

                switch (pattern)
                {
                    case 0: pattern_chase(1);         break;
                    case 1: pattern_knight_rider(1);  break;
                    case 2: pattern_bar_fill(1);      break;
                    case 3: pattern_binary_count(63); break;
                    case 4: pattern_expand(1);        break;
                    case 5: pattern_alternate(3);     break;
                    case 6: pattern_invert_chase(1);  break;
                    case 7: pattern_lfsr(30);         break;
                }

                pattern = (pattern + 1u) % 8u;
            }
        }
    }
}
/* Clock setup, 16 MHz internal oscillator.
 * Keep the version STM32CubeMX generated. This copy is only
 * here so the file is complete.
 */
/*void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}*/

void Error_Handler(void)
{
    while (1)
    {
    }
}

/* ==========================================================================
 *  EXERCISES
 *
 *  1. Change STEP_MS to 40 and then to 400. What is the slowest speed at
 *     which the chase still looks like motion rather than separate blinks?
 *
 *  2. Write pattern 9: a chase that runs from PA9 down to PA0. One line
 *     changes inside the loop body.
 *
 *  3. In pattern_binary_count, count how many frames PA0 is on out of 64.
 *     Now do the same for PA5. Explain the ratio.
 *
 *  4. leds_write does read-modify-write. Replace its body with the single
 *     line  GPIOA->ODR = value;  and rebuild. The LEDs still work. Explain
 *     why it is still wrong, and what would break if PA10 were also in use.
 *
 *  5. Change the LFSR taps from bits 9 and 6 to bits 9 and 5. Run it and
 *     watch. The sequence now repeats much sooner. Why does the choice of
 *     taps matter?
 *
 *  6. Write a pattern that shows the lower 10 bits of a counter that
 *     increments once per second, so the LEDs act as a binary clock.
 * ========================================================================== */
