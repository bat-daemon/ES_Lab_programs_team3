/*
 * main.c - ES lab practice
 * Sort + search printed over USART, internal temperature,
 * and a 3-LED pattern that changes with the B1 button interrupt.
 */

#include "stm32f446xx.h"
#include <stdio.h>
#include <string.h>

#define NUM_LEDS   3
#define BTN_PORT   GPIOC
#define BTN_PIN    GPIO_PIN_NO_13           // B1 on the board, 0 when pressed

// external LEDs on PA6, PA7, PA8 (D12, D11, D7), all on GPIOA
static const uint8_t led_pins[NUM_LEDS] = { GPIO_PIN_NO_6, GPIO_PIN_NO_7, GPIO_PIN_NO_8 };

USART_Handle_t usart2;
ADC_Handle_t   adc1;

volatile uint8_t btn_flag = 0;              // ISR sets it, main clears it
uint8_t pattern = 0;                        // 0 = knight rider, 1 = all blink

/* ---------- helpers ---------- */

static void Delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 2000U; i++);   // rough, 16 MHz
}

static void Pin_Config(GPIO_RegDef_t *port, uint8_t pin,
                       uint8_t mode, uint8_t pupd, uint8_t af)
{
    GPIO_Handle_t h;
    h.pGPIOx                             = port;
    h.GPIO_PinConfig.GPIO_PinNumber      = pin;
    h.GPIO_PinConfig.GPIO_PinMode        = mode;
    h.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    h.GPIO_PinConfig.GPIO_PinPuPdControl = pupd;
    h.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    h.GPIO_PinConfig.GPIO_PinAltFunMode  = af;
    GPIO_Init(&h);
}

static void UART_Print(const char *s)
{
    USART_SendData(&usart2, (uint8_t *)s, strlen(s));
}

static void USART2_Setup(void)
{
    Pin_Config(GPIOA, GPIO_PIN_NO_2, GPIO_MODE_ALTFN, GPIO_PIN_PU, GPIO_AF7);   // TX only

    usart2.pUSARTx                          = USART2;
    usart2.USART_Config.USART_Mode          = USART_MODE_ONLY_TX;
    usart2.USART_Config.USART_Baud          = USART_STD_BAUD_115200;
    usart2.USART_Config.USART_NoOfStopBits  = USART_STOPBITS_1;
    usart2.USART_Config.USART_WordLength    = USART_WORDLEN_8BITS;
    usart2.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    usart2.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(&usart2);
    USART_PeripheralControl(USART2, ENABLE);
}

static void ADC1_Setup(void)
{
    adc1.pADCx                          = ADC1;
    adc1.ADC_Config.ADC_Resolution      = ADC_RES_12BITS;
    adc1.ADC_Config.ADC_DataAlignment   = ADC_ALIGN_RIGHT;
    adc1.ADC_Config.ADC_ConvMode        = ADC_CONV_SINGLE;
    adc1.ADC_Config.ADC_ScanMode        = ADC_SCAN_DISABLE;
    adc1.ADC_Config.ADC_NumOfConversion = 1;
    adc1.ADC_Config.ADC_SamplingTime    = ADC_SAMPLE_480CYCLES;
    adc1.ADC_Config.ADC_ClkPreScaler    = ADC_CLK_DIV2;

    ADC_Init(&adc1);
    ADC_TempSensorVrefintControl(ENABLE);      // sensor is off after reset
    ADC_PeripheralControl(ADC1, ENABLE);
}

/* ---------- LED helpers and patterns ---------- */

static void LEDs_All(uint8_t state)
{
    for (int i = 0; i < NUM_LEDS; i++)
        GPIO_WriteToOutputPin(GPIOA, led_pins[i], state);
}

// pattern 0: knight rider, 0 -> 1 -> 2 -> 1 -> back
static void Pattern_KnightRider(void)
{
    const uint8_t order[4] = { 0, 1, 2, 1 };

    for (int i = 0; i < 4; i++)
    {
        LEDs_All(GPIO_PIN_RESET);
        GPIO_WriteToOutputPin(GPIOA, led_pins[order[i]], GPIO_PIN_SET);
        Delay_ms(150);
    }
}

// pattern 1: all three blink together
static void Pattern_AllBlink(void)
{
    LEDs_All(GPIO_PIN_SET);
    Delay_ms(300);
    LEDs_All(GPIO_PIN_RESET);
    Delay_ms(300);
}

/* ---------- sort and search ---------- */

static void Print_Array(const char *label, const int *a, int n)
{
    char buf[16];

    UART_Print(label);
    for (int i = 0; i < n; i++)
    {
        sprintf(buf, "%d ", a[i]);
        UART_Print(buf);
    }
    UART_Print("\r\n");
}

// bubble sort, ascending
static void Sort_Array(int *a, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                int t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

// linear search, returns the index or -1
static int Search_Array(const int *a, int n, int key)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] == key)
            return i;
    }
    return -1;
}

/* ---------- temperature ---------- */

// returns temperature in tenths of a degree C
static int32_t Read_Temp_x10(void)
{
    uint16_t raw = ADC_ReadChannel(&adc1, ADC_CHANNEL_TEMPSENSOR);
    uint32_t mv  = ADC_CountsToMillivolts(&adc1, raw);

    return ((int32_t)mv - 760) * 4 + 250;      // T = (V - 0.76) / 2.5mV + 25
}

static void Print_Temp(void)
{
    char    msg[40];
    int32_t t = Read_Temp_x10();

    sprintf(msg, "Temp = %d.%d C\r\n", (int)(t / 10), (int)(t % 10));
    UART_Print(msg);
}

/* ---------- main ---------- */

int main(void)
{
    int data[8] = { 42, 7, 19, 3, 88, 15, 61, 27 };
    int key = 19;
    int idx;
    char msg[40];
    int sweeps = 0;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_PeriClockControl(GPIOC, ENABLE);

    for (int i = 0; i < NUM_LEDS; i++)
        Pin_Config(GPIOA, led_pins[i], GPIO_MODE_OUT, GPIO_NO_PUPD, 0);

    Pin_Config(BTN_PORT, BTN_PIN, GPIO_MODE_IT_FT, GPIO_NO_PUPD, 0);
    GPIO_IRQConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15, ENABLE);

    USART2_Setup();
    ADC1_Setup();

    UART_Print("\r\n=== Lab practice ===\r\n");

    /* sort */
    Print_Array("Before sort: ", data, 8);
    Sort_Array(data, 8);
    Print_Array("After sort : ", data, 8);

    /* search */
    idx = Search_Array(data, 8, key);
    if (idx >= 0)
        sprintf(msg, "Key %d found at index %d\r\n", key, idx);
    else
        sprintf(msg, "Key %d not found\r\n", key);
    UART_Print(msg);

    /* temperature once, then LEDs run forever */
    Print_Temp();
    UART_Print("Press B1 to change the LED pattern\r\n");

    while (1)
    {
        if (btn_flag)
        {
            Delay_ms(20);                                           // debounce
            while (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == 0);  // wait for release
            Delay_ms(50);
            btn_flag = 0;

            pattern ^= 1;
            LEDs_All(GPIO_PIN_RESET);
            UART_Print(pattern ? "Pattern: all blink\r\n" : "Pattern: knight rider\r\n");
        }

        if (pattern == 0)
            Pattern_KnightRider();
        else
            Pattern_AllBlink();

        sweeps++;
        if (sweeps % 10 == 0)
            Print_Temp();                      // temperature every 10 sweeps
    }
}

void EXTI15_10_IRQHandler(void)
{
    GPIO_IRQHandling(BTN_PIN);
    btn_flag = 1;
}
