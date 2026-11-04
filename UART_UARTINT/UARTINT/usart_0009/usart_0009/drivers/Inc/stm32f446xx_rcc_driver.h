/*
 * stm32f446xx_rcc_driver.h
 *
 *  Created on: Aug 27, 2026
 *      Author: raghudathesh.gp
 *
 *  Shared clock services for every peripheral driver.
 *
 *  Any peripheral whose timing depends on the bus clock must ask RCC how fast
 *  that bus is actually running before it can program its own divider:
 *
 *      USART / UART : baud rate     BRR  = f(PCLK1 or PCLK2)
 *      I2C          : SCL frequency CCR  = f(PCLK1)
 *      SPI          : SCLK          BR   = f(PCLK1 or PCLK2)
 *      USB OTG FS   : requires an exact 48 MHz from the PLL Q output
 *
 *  Putting these functions here means the calculation exists once instead of
 *  being copied into every driver.
 */

#ifndef INC_STM32F446XX_RCC_DRIVER_H_      //this contains driver Specific data
#define INC_STM32F446XX_RCC_DRIVER_H_      //this contains driver Specific data

#include "stm32f446xx.h"                   //this contains MCU Specific data


/********************************************************************************
 *                        Oscillator frequency macros
 ********************************************************************************/

/*
 * @RCC_OSC_VALUES
 * HSI is the internal RC oscillator that runs after every reset.
 * HSE on the NUCLEO-F446RE is 8 MHz supplied by the ST-LINK MCO output,
 * NOT a crystal. Change HSE_VALUE if you move this code to another board.
 */
#define HSI_VALUE               16000000U   // 16 MHz internal RC oscillator
#define HSE_VALUE               8000000U    // 8 MHz from ST-LINK on the Nucleo
#define LSI_VALUE               32000U      // ~32 kHz internal RC, drives IWDG
#define LSE_VALUE               32768U      // 32.768 kHz crystal for the RTC


/*
 * @RCC_SYSCLK_SRC
 * Values returned in the SWS field of RCC_CFGR, bits [3:2]
 */
#define RCC_SYSCLK_SRC_HSI      0U   // HSI used as the system clock
#define RCC_SYSCLK_SRC_HSE      1U   // HSE used as the system clock
#define RCC_SYSCLK_SRC_PLL      2U   // Main PLL used as the system clock
#define RCC_SYSCLK_SRC_PLL_R    3U   // PLL R output used as the system clock


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Clock source query */
uint8_t  RCC_GetSysClkSource(void);

/* Clock frequency query - all values returned in Hz */
uint32_t RCC_GetSysClkValue(void);       // SYSCLK, before the AHB prescaler
uint32_t RCC_GetHCLKValue(void);         // AHB  bus clock, feeds the core and DMA
uint32_t RCC_GetPCLK1Value(void);        // APB1 bus clock, max 45 MHz
uint32_t RCC_GetPCLK2Value(void);        // APB2 bus clock, max 90 MHz

/* PLL output queries */
uint32_t RCC_GetPLLOutputClock(void);    // Main PLL P output, normally SYSCLK
uint32_t RCC_GetPLL48ClkValue(void);     // Main PLL Q output, must be 48 MHz for USB


#endif /* INC_STM32F446XX_RCC_DRIVER_H_ */
