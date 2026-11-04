/*
 * stm32f446xx_rcc_driver.c
 *
 *  Created on: Aug 27, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_rcc_driver.h"


/*
 * AHB prescaler lookup table.
 * RCC_CFGR HPRE field is 4 bits. Values 0 to 7 mean "not divided", values
 * 8 to 15 select the divisors below. Note that 32 is deliberately absent -
 * the hardware skips it, which is why the table is not a simple power of two.
 */
static uint16_t AHB_PreScaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};

/*
 * APB prescaler lookup table.
 * RCC_CFGR PPRE1 and PPRE2 fields are 3 bits. Values 0 to 3 mean
 * "not divided", values 4 to 7 select the divisors below.
 */
static uint8_t APB_PreScaler[4] = {2, 4, 8, 16};


/*********************************************************************
 * @fn      		  - RCC_GetSysClkSource
 *
 * @brief             - Reports which oscillator is currently driving SYSCLK
 *
 * @param[in]         - None
 *
 * @return            - One of the @RCC_SYSCLK_SRC values
 *
 * @Note              - Reads the SWS field, not the SW field. SW is what was
 *                      REQUESTED, SWS is what the hardware actually switched
 *                      to once the new source became stable.
 *********************************************************************/
uint8_t RCC_GetSysClkSource(void)
{
    /* RCC_CFGR bits [3:2] hold the system clock switch status */
    return (uint8_t)((RCC->CFGR >> 2) & 0x3U);
}


/*********************************************************************
 * @fn      		  - RCC_GetPLLOutputClock
 *
 * @brief             - Computes the main PLL P output frequency
 *
 * @param[in]         - None
 *
 * @return            - PLL P output clock in Hz
 *
 * @Note              - VCO    = (PLL source / PLLM) x PLLN
 *                      Output = VCO / PLLP
 *                      PLLM removes the source frequency dependency, PLLN
 *                      multiplies up to the VCO, PLLP divides back down.
 *********************************************************************/
uint32_t RCC_GetPLLOutputClock(void)
{
    uint32_t pllsource, pllm, plln, pllp, vco;

    pllm = (RCC->PLLCFGR >> 0) & 0x3FU;                  // Bits [5:0]  : division factor
    plln = (RCC->PLLCFGR >> 6) & 0x1FFU;                 // Bits [14:6] : multiplication factor
    pllp = (((RCC->PLLCFGR >> 16) & 0x3U) + 1U) * 2U;    // Bits [17:16]: encodes 2, 4, 6, 8

    pllsource = (RCC->PLLCFGR >> 22) & 0x1U;             // Bit 22 : 0 = HSI, 1 = HSE

    if(pllm == 0U)
    {
        /* PLLM of zero is an illegal setting; guard against a divide by zero */
        return HSI_VALUE;
    }

    /* Divide before multiplying, so the intermediate value cannot overflow */
    vco = ((pllsource ? HSE_VALUE : HSI_VALUE) / pllm) * plln;

    return (vco / pllp);
}


/*********************************************************************
 * @fn      		  - RCC_GetPLL48ClkValue
 *
 * @brief             - Computes the main PLL Q output frequency
 *
 * @param[in]         - None
 *
 * @return            - PLL Q output clock in Hz
 *
 * @Note              - USB OTG FS, SDIO and the random number generator all
 *                      need exactly 48 MHz from this output. If this function
 *                      does not return 48000000 then USB will enumerate
 *                      unreliably or not at all.
 *********************************************************************/
uint32_t RCC_GetPLL48ClkValue(void)
{
    uint32_t pllsource, pllm, plln, pllq, vco;

    pllm = (RCC->PLLCFGR >> 0) & 0x3FU;      // Bits [5:0]   : division factor
    plln = (RCC->PLLCFGR >> 6) & 0x1FFU;     // Bits [14:6]  : multiplication factor
    pllq = (RCC->PLLCFGR >> 24) & 0xFU;      // Bits [27:24] : Q divider, 2 to 15

    pllsource = (RCC->PLLCFGR >> 22) & 0x1U; // Bit 22 : 0 = HSI, 1 = HSE

    if((pllm == 0U) || (pllq == 0U))
    {
        /* Illegal settings; report zero so the caller can detect the fault */
        return 0U;
    }

    vco = ((pllsource ? HSE_VALUE : HSI_VALUE) / pllm) * plln;

    return (vco / pllq);
}


/*********************************************************************
 * @fn      		  - RCC_GetSysClkValue
 *
 * @brief             - Returns the system clock frequency before the AHB prescaler
 *
 * @param[in]         - None
 *
 * @return            - SYSCLK in Hz
 *
 * @Note              - After reset this is 16 MHz, because the HSI is selected
 *                      and no PLL has been configured
 *********************************************************************/
uint32_t RCC_GetSysClkValue(void)
{
    uint8_t clksrc = RCC_GetSysClkSource();

    if(clksrc == RCC_SYSCLK_SRC_HSI)
    {
        return HSI_VALUE;                    // HSI selected
    }
    else if(clksrc == RCC_SYSCLK_SRC_HSE)
    {
        return HSE_VALUE;                    // HSE selected
    }
    else
    {
        return RCC_GetPLLOutputClock();      // PLL selected
    }
}


/*********************************************************************
 * @fn      		  - RCC_GetHCLKValue
 *
 * @brief             - Returns the AHB bus clock frequency
 *
 * @param[in]         - None
 *
 * @return            - HCLK in Hz
 *
 * @Note              - HCLK feeds the core, the memories, DMA and the GPIO
 *                      ports. It is also the reference for SysTick.
 *********************************************************************/
uint32_t RCC_GetHCLKValue(void)
{
    uint32_t SystemClk;
    uint8_t  temp, ahbp;

    SystemClk = RCC_GetSysClkValue();

    /* AHB prescaler : RCC_CFGR HPRE bits [7:4] */
    temp = (uint8_t)((RCC->CFGR >> 4) & 0xFU);

    /* Values below 8 mean the clock is not divided at all */
    ahbp = (temp < 8U) ? 1U : AHB_PreScaler[temp - 8U];

    return (SystemClk / ahbp);
}


/*********************************************************************
 * @fn      		  - RCC_GetPCLK1Value
 *
 * @brief             - Returns the APB1 peripheral clock frequency
 *
 * @param[in]         - None
 *
 * @return            - PCLK1 in Hz
 *
 * @Note              - APB1 is limited to 45 MHz on the STM32F446.
 *                      Used by USART2, USART3, UART4, UART5, SPI2, SPI3,
 *                      I2C1 to I2C3, CAN1, CAN2 and the DAC.
 *********************************************************************/
uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t hclk;
    uint8_t  temp, apb1p;

    hclk = RCC_GetHCLKValue();

    /* APB1 prescaler : RCC_CFGR PPRE1 bits [12:10] */
    temp = (uint8_t)((RCC->CFGR >> 10) & 0x7U);

    /* Values below 4 mean the clock is not divided at all */
    apb1p = (temp < 4U) ? 1U : APB_PreScaler[temp - 4U];

    return (hclk / apb1p);
}


/*********************************************************************
 * @fn      		  - RCC_GetPCLK2Value
 *
 * @brief             - Returns the APB2 peripheral clock frequency
 *
 * @param[in]         - None
 *
 * @return            - PCLK2 in Hz
 *
 * @Note              - APB2 is limited to 90 MHz on the STM32F446, twice the
 *                      APB1 limit. That is why USART1 and USART6 reach higher
 *                      baud rates than the other serial ports.
 *                      Used by USART1, USART6, SPI1, SPI4, ADC1 to ADC3,
 *                      TIM1, TIM8, SDIO and SYSCFG.
 *********************************************************************/
uint32_t RCC_GetPCLK2Value(void)
{
    uint32_t hclk;
    uint8_t  temp, apb2p;

    hclk = RCC_GetHCLKValue();

    /* APB2 prescaler : RCC_CFGR PPRE2 bits [15:13] */
    temp = (uint8_t)((RCC->CFGR >> 13) & 0x7U);

    /* Values below 4 mean the clock is not divided at all */
    apb2p = (temp < 4U) ? 1U : APB_PreScaler[temp - 4U];

    return (hclk / apb2p);
}
