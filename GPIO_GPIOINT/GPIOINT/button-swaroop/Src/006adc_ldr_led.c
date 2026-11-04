/*
 * 006adc_ldr_led.c
 *
 *  Created on: Aug 29, 2026
 *      Author: raghudathesh.gp
 *
 *  Board   : NUCLEO-F446RE
 *  Target  : STM32F446RET6
 *
 *  Purpose : Read an LDR (light dependent resistor) on an analog pin and
 *            drive the on-board LED (LD2, PA5) from the light level.
 *
 *            ADC value <  2000  ->  LED ON
 *            ADC value >= 2000  ->  LED OFF
 *
 *  Hardware:
 *      LDR forms a voltage divider with a fixed resistor (typically 10k)
 *      between 3V3 and GND. The midpoint of the divider feeds the ADC pin.
 *
 *          3V3 ---[ LDR ]---+---[ 10k ]--- GND
 *                            |
 *                          PA0 (ADC1_IN0)
 *
 *      With this arrangement, MORE light -> LDR resistance drops -> midpoint
 *      voltage RISES -> ADC count goes UP.
 *      LESS light (dark) -> LDR resistance rises -> midpoint voltage FALLS
 *      -> ADC count goes DOWN.
 *
 *      So "ADC value < 2000" corresponds to a DARK condition with this
 *      wiring, and the LED turns ON when it gets dark - the classic
 *      automatic night light behaviour.
 *
 *      If your LDR and resistor are swapped (LDR to GND, resistor to 3V3),
 *      the relationship inverts. Check with semihosting printf before
 *      trusting the threshold on your bench.
 *
 *  Pin usage:
 *      PA0  -> ADC1_IN0, analog input from the LDR divider
 *      PA5  -> LD2, digital output, the on-board LED
 */

#include <string.h>                   // for memset
#include "stm32f446xx.h"              // MCU header, pulls in every driver header

#define LDR_THRESHOLD       2000U     // decision point, out of a 0-4095 range at 12-bit

ADC_Handle_t adc1_handle;             // file scope so every function can reach it
GPIO_Handle_t led_handle;             // handle for the LED output pin


/*********************************************************************
 * @fn                - delay
 * @brief             - Crude blocking delay, used to pace the polling loop
 * @return            - None
 * @Note              - Roughly 100 ms on the 16 MHz HSI. The LDR does not
 *                      change fast, so there is no need to sample constantly.
 *********************************************************************/
void delay(void)
{
    for(volatile uint32_t i = 0; i < 250000; i++);  // burn CPU cycles; body intentionally empty
}


/*********************************************************************
 * @fn                - LED_GPIOInit
 * @brief             - Configures PA5 as a push-pull output for LD2
 * @return            - None
 *********************************************************************/
void LED_GPIOInit(void)
{
    memset(&led_handle, 0, sizeof(led_handle));                   // clear stack garbage before filling fields

    led_handle.pGPIOx = GPIOA;                                    // LD2 sits on port A

    led_handle.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;   // LD2 is wired to pin 5
    led_handle.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;   // drive the pin
    led_handle.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST; // irrelevant for an LED, kept for uniformity
    led_handle.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP; // push-pull so the pin can source current
    led_handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // push-pull drives both rails

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // ungate GPIOA clock in RCC_AHB1ENR

    GPIO_Init(&led_handle);                                       // write MODER, OSPEEDR, PUPDR, OTYPER for PA5
}


/*********************************************************************
 * @fn                - LDR_GPIOInit
 * @brief             - Configures PA0 as an analog input for the ADC
 * @return            - None
 * @Note              - Analog mode disconnects the digital input buffer.
 *                      Pull resistors must stay off, or the internal
 *                      40 kohm pull-up forms an unwanted divider with the
 *                      LDR network and shifts every reading.
 *********************************************************************/
void LDR_GPIOInit(void)
{
    GPIO_Handle_t ldr_pin;                                        // local handle, only needed once

    memset(&ldr_pin, 0, sizeof(ldr_pin));                         // clear stack garbage

    ldr_pin.pGPIOx = GPIOA;                                       // A0 is PA0

    ldr_pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_0;   // ADC1_IN0 lives on pin 0
    ldr_pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ANALOG;// MODER = 11, disconnects the digital buffer
    ldr_pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;    // no pull resistor on an analog pin

    GPIO_PeriClockControl(GPIOA, ENABLE);                         // GPIOA clock is already on from the LED,
                                                                    // but call it anyway so this function stands alone

    GPIO_Init(&ldr_pin);                                          // write MODER and PUPDR for PA0
}


/*********************************************************************
 * @fn                - ADC1_Init
 * @brief             - Configures ADC1 for a single-shot, right aligned,
 *                      12-bit blocking read of one channel
 * @return            - None
 *********************************************************************/
void ADC1_Init(void)
{
    memset(&adc1_handle, 0, sizeof(adc1_handle));                 // clear the handle before filling it

    adc1_handle.pADCx = ADC1;                                     // ADC1 has the LDR channel available

    adc1_handle.ADC_Config.ADC_Resolution      = ADC_RES_12BITS;      // full 0-4095 range
    adc1_handle.ADC_Config.ADC_DataAlignment   = ADC_ALIGN_RIGHT;     // result in the low bits of DR
    adc1_handle.ADC_Config.ADC_ConvMode        = ADC_CONV_SINGLE;     // one conversion per trigger, then stop
    adc1_handle.ADC_Config.ADC_ScanMode        = ADC_SCAN_DISABLE;    // only one channel is used here
    adc1_handle.ADC_Config.ADC_NumOfConversion = 1;                   // a single conversion in the sequence
    adc1_handle.ADC_Config.ADC_SamplingTime    = ADC_SAMPLE_84CYCLES; // LDR divider has moderate impedance
    adc1_handle.ADC_Config.ADC_ClkPreScaler    = ADC_CLK_DIV4;        // PCLK2 / 4, safely under the 36 MHz limit

    ADC_Init(&adc1_handle);                                       // write CR1, CR2, SQR1 and the common prescaler

    ADC_PeripheralControl(ADC1, ENABLE);                          // set ADON; the driver also waits for it to settle
}


int main(void)
{
    uint16_t ldr_raw;                                             // raw 12-bit result from the ADC
    uint32_t ldr_mv;                                              // same result converted to millivolts

    LED_GPIOInit();                                               // set up PA5 as the LED output
    LDR_GPIOInit();                                                // set up PA0 as the analog input
    ADC1_Init();                                                   // bring up ADC1 itself

    while(1)                                                      // an embedded application never returns
    {
        /* Blocking read of the LDR channel: configure, start, poll EOC, return DR */
        ldr_raw = ADC_ReadChannel(&adc1_handle, ADC_CHANNEL_0);

        ldr_mv = ADC_CountsToMillivolts(&adc1_handle, ldr_raw);   // just for reference, not used in the decision

        (void)ldr_mv;                                             // silence an unused-variable warning if no
                                                                    // debugger is attached to watch it

        if(ldr_raw < LDR_THRESHOLD)                                // dark condition, with the wiring described above
        {
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, GPIO_PIN_SET);    // turn the LED ON
        }
        else                                                       // bright condition
        {
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, GPIO_PIN_RESET);  // turn the LED OFF
        }

        delay();                                                   // pace the loop; the LDR changes slowly
    }

    return 0;                                                      // never reached; present only to satisfy the int return type
}
