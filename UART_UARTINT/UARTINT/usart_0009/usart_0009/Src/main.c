
/*
 * main.c - ES lab practice
 * STM32 Nucleo-F446RE
 *
 * Functions:
 * 1. Bubble sort and linear search
 * 2. Print results using USART2
 * 3. Read internal temperature sensor using ADC1
 * 4. Three external LEDs on PA6, PA7, PA8
 * 5. PC13 button interrupt switches LED patterns
 *
 * Custom drivers only - no HAL
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_adc_driver.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NUM_LEDS  3U
#define BTN_PORT  GPIOC
#define BTN_PIN   GPIO_PIN_NO_13

/* External LEDs: PA6, PA7, PA8 */
static const uint8_t led_pins[NUM_LEDS] =
{
    GPIO_PIN_NO_6,
    GPIO_PIN_NO_7,
    GPIO_PIN_NO_8
};

USART_Handle_t usart2;
ADC_Handle_t adc1;

volatile uint8_t btn_flag = 0U;
uint8_t pattern = 0U;

/* --------------------------------------------------
   Delay
   -------------------------------------------------- */
static void Delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0U; i < ms * 2000U; i++)
    {
        /* Wait */
    }
}

/* --------------------------------------------------
   Configure one GPIO pin using the custom driver
   -------------------------------------------------- */
static void Pin_Config(GPIO_RegDef_t *port,
                       uint8_t pin,
                       uint8_t mode,
                       uint8_t pupd,
                       uint8_t af)
{
    GPIO_Handle_t h = {0};

    h.pGPIOx = port;

    h.GPIO_PinConfig.GPIO_PinNumber = pin;
    h.GPIO_PinConfig.GPIO_PinMode = mode;
    h.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    h.GPIO_PinConfig.GPIO_PinPuPdControl = pupd;
    h.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    h.GPIO_PinConfig.GPIO_PinAltFunMode = af;

    GPIO_Init(&h);
}

/* --------------------------------------------------
   USART2: PA2 = TX, Alternate Function 7
   -------------------------------------------------- */
static void USART2_Setup(void)
{
    GPIO_PeriClockControl(GPIOA, ENABLE);

    Pin_Config(GPIOA,
               GPIO_PIN_NO_2,
               GPIO_MODE_ALTFN,
               GPIO_NO_PUPD,
               GPIO_AF7);

    usart2.pUSARTx = USART2;

    usart2.USART_Config.USART_Mode =
        USART_MODE_ONLY_TX;

    usart2.USART_Config.USART_Baud =
        USART_STD_BAUD_115200;

    usart2.USART_Config.USART_NoOfStopBits =
        USART_STOPBITS_1;

    usart2.USART_Config.USART_WordLength =
        USART_WORDLEN_8BITS;

    usart2.USART_Config.USART_ParityControl =
        USART_PARITY_DISABLE;

    usart2.USART_Config.USART_HWFlowControl =
        USART_HW_FLOW_CTRL_NONE;

    USART_Init(&usart2);
    USART_PeripheralControl(USART2, ENABLE);
}

/* --------------------------------------------------
   Print a string through USART2
   -------------------------------------------------- */
static void UART_Print(const char *s)
{
    USART_SendData(&usart2,
                   (uint8_t *)s,
                   (uint32_t)strlen(s));
}

/* --------------------------------------------------
   ADC1: internal temperature sensor
   -------------------------------------------------- */
static void ADC1_Setup(void)
{
    adc1.pADCx = ADC1;

    adc1.ADC_Config.ADC_Resolution =
        ADC_RES_12BITS;

    adc1.ADC_Config.ADC_DataAlignment =
        ADC_ALIGN_RIGHT;

    adc1.ADC_Config.ADC_ConvMode =
        ADC_CONV_SINGLE;

    adc1.ADC_Config.ADC_ScanMode =
        ADC_SCAN_DISABLE;

    adc1.ADC_Config.ADC_NumOfConversion = 1U;

    adc1.ADC_Config.ADC_SamplingTime =
        ADC_SAMPLE_480CYCLES;

    adc1.ADC_Config.ADC_ClkPreScaler =
        ADC_CLK_DIV2;

    ADC_Init(&adc1);

    /* Enable internal temperature sensor */
    ADC_TempSensorVrefintControl(ENABLE);

    /* Enable ADC1 */
    ADC_PeripheralControl(ADC1, ENABLE);
}

/* --------------------------------------------------
   LED functions
   -------------------------------------------------- */
static void LEDs_All(uint8_t state)
{
    for (uint8_t i = 0U; i < NUM_LEDS; i++)
    {
        GPIO_WriteToOutputPin(GPIOA,
                              led_pins[i],
                              state);
    }
}

/* Pattern 0: PA6 -> PA7 -> PA8 -> PA7 */
static void Pattern_KnightRider(void)
{
    const uint8_t order[4] = {0U, 1U, 2U, 1U};

    for (uint8_t i = 0U; i < 4U; i++)
    {
        LEDs_All(GPIO_PIN_RESET);

        GPIO_WriteToOutputPin(GPIOA,
                              led_pins[order[i]],
                              GPIO_PIN_SET);

        Delay_ms(150U);
    }
}

/* Pattern 1: all three LEDs blink together */
static void Pattern_AllBlink(void)
{
    LEDs_All(GPIO_PIN_SET);
    Delay_ms(300U);

    LEDs_All(GPIO_PIN_RESET);
    Delay_ms(300U);
}

/* --------------------------------------------------
   Array printing
   -------------------------------------------------- */
static void Print_Array(const char *label,
                        const int *a,
                        int n)
{
    char buf[24];

    UART_Print(label);

    for (int i = 0; i < n; i++)
    {
        sprintf(buf, "%d ", a[i]);
        UART_Print(buf);
    }

    UART_Print("\r\n");
}

/* --------------------------------------------------
   Bubble sort - ascending order
   -------------------------------------------------- */
static void Sort_Array(int *a, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

/* --------------------------------------------------
   Linear search - returns index or -1
   -------------------------------------------------- */
static int Search_Array(const int *a, int n, int key)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] == key)
        {
            return i;
        }
    }

    return -1;
}

/* --------------------------------------------------
   Read internal temperature sensor
   Return value is in tenths of degrees Celsius.
   -------------------------------------------------- */
static int32_t Read_Temp_x10(void)
{
    uint16_t raw;
    uint32_t mv;

    raw = ADC_ReadChannel(&adc1, ADC_CHANNEL_TEMPSENSOR);
    mv = ADC_CountsToMillivolts(&adc1, raw);

    /*
     * Approximate formula:
     * Temperature = (Voltage - 760 mV) / 2.5 mV + 25 C
     *
     * Result multiplied by 10:
     * T_x10 = (mv - 760) * 4 + 250
     */
    return (((int32_t)mv - 760) * 4) + 250;
}

/* --------------------------------------------------
   Print temperature
   -------------------------------------------------- */
static void Print_Temp(void)
{
    char msg[48];
    int32_t t = Read_Temp_x10();
    int32_t whole = t / 10;
    int32_t decimal = t % 10;

    /* Handle negative temperatures correctly */
    if (decimal < 0)
    {
        decimal = -decimal;
    }

    sprintf(msg, "Temp = %ld.%ld C\r\n",
            (long)whole,
            (long)decimal);

    UART_Print(msg);
}

/* --------------------------------------------------
   PC13 button interrupt setup
   Nucleo-F446RE B1 is active HIGH:
   released = LOW, pressed = HIGH.
   -------------------------------------------------- */
static void Button_Interrupt_Init(void)
{
    GPIO_PeriClockControl(GPIOC, ENABLE);

    /*
     * Configure PC13 as interrupt input.
     * Internal pull-down keeps the input LOW
     * while the button is released.
     */
    Pin_Config(GPIOC,
               BTN_PIN,
               GPIO_MODE_IT_RT,
               GPIO_PIN_PD,
               0U);

    /* Clear any pending EXTI13 interrupt */
    EXTI->PR = (1U << 13);

    /* Enable EXTI15_10 IRQ, number 40 */
    GPIO_IRQConfig(IRQ_NO_EXTI15_10,
                   NVIC_IRQ_PRI15,
                   ENABLE);
}

/* --------------------------------------------------
   Interrupt handler
   -------------------------------------------------- */
void EXTI15_10_IRQHandler(void)
{
    GPIO_IRQHandling(BTN_PIN);

    btn_flag = 1U;
}

/* --------------------------------------------------
   Main
   -------------------------------------------------- */
int main(void)
{
    int data[8] = {42, 7, 19, 3, 88, 15, 61, 27};
    int key = 19;
    int idx;
    char msg[48];
    uint32_t sweeps = 0U;

    /* Enable GPIO clocks */
    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_PeriClockControl(GPIOC, ENABLE);

    /* Configure the three external LEDs */
    for (uint8_t i = 0U; i < NUM_LEDS; i++)
    {
        Pin_Config(GPIOA,
                   led_pins[i],
                   GPIO_MODE_OUT,
                   GPIO_NO_PUPD,
                   0U);
    }

    LEDs_All(GPIO_PIN_RESET);

    /* Initialize peripherals */
    USART2_Setup();
    ADC1_Setup();
    Button_Interrupt_Init();

    UART_Print("\r\n=== Lab Practice ===\r\n");

    /* Bubble sort */
    Print_Array("Before sort: ", data, 8);

    Sort_Array(data, 8);

    Print_Array("After sort : ", data, 8);

    /* Linear search */
    idx = Search_Array(data, 8, key);

    if (idx >= 0)
    {
        sprintf(msg,
                "Key %d found at index %d\r\n",
                key,
                idx);
    }
    else
    {
        sprintf(msg, "Key %d not found\r\n", key);
    }

    UART_Print(msg);

    /* Initial temperature reading */
    Print_Temp();

    UART_Print("Press B1 to change LED pattern\r\n");
    UART_Print("Pattern: knight rider\r\n");

    while (1)
    {
        /*
         * Process button events in main,
         * not inside the interrupt handler.
         */
        if (btn_flag != 0U)
        {
            Delay_ms(20U);

            /*
             * Confirm button is still pressed.
             * PC13 is HIGH while pressed.
             */
            if (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) != 0U)
            {
                /*
                 * Wait for release.
                 * PC13 returns LOW when released.
                 */
                while (GPIO_ReadFromInputPin(BTN_PORT,
                                             BTN_PIN) != 0U)
                {
                    /* Wait for release */
                }

                Delay_ms(50U);

                btn_flag = 0U;

                /* Switch pattern */
                pattern ^= 1U;

                LEDs_All(GPIO_PIN_RESET);

                if (pattern != 0U)
                {
                    UART_Print("Pattern: all blink\r\n");
                }
                else
                {
                    UART_Print("Pattern: knight rider\r\n");
                }
            }
            else
            {
                /* Ignore a short/noisy event */
                btn_flag = 0U;
            }
        }

        /* Run the selected pattern */
        if (pattern == 0U)
        {
            Pattern_KnightRider();
        }
        else
        {
            Pattern_AllBlink();
        }

        sweeps++;

        /* Print temperature every 10 sweeps */
        if ((sweeps % 10U) == 0U)
        {
            Print_Temp();
        }
    }
}
