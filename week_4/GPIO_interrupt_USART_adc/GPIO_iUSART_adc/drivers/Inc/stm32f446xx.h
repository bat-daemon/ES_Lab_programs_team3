/*
 * stm32f446xx.h
 *
 *  Created on: Aug 22, 2026
 *      Author: raghudathesh.gp
 */

#ifndef INC_STM32F446XX_H_
#define INC_STM32F446XX_H_

#include<stdint.h>
#include <stddef.h>   // for NULL, used by the interrupt based drivers

// #define __vo volatile //You can use __vo in place of volatile


/*
* base addresses of Flash and SRAM memories
*/

#define FLASH_BASEADDR     0x08000000U  // Base address of main Flash memory
#define SRAM1_BASEADDR     0x20000000U  // Base address of SRAM1 (112 KB)
#define SRAM2_BASEADDR     0x2001C000U  // Base address of SRAM2 (16 KB)
#define ROM_BASEADDR       0x1FFF0000U  // Base address of system memory (bootloader ROM)
#define SRAM               SRAM1_BASEADDR  // Alias for the start of SRAM

/*
* AHBx and APBx Bus Peripheral base addresses
*/

#define PERIPH_BASE         0x40000000U  // Base address of peripheral memory

#define APB1PERIPH_BASE     PERIPH_BASE  // Base address of APB1 peripherals
#define APB2PERIPH_BASE     0x40010000U  // Base address of APB2 peripherals
#define AHB1PERIPH_BASE     0x40020000U  // Base address of AHB1 peripherals
#define AHB2PERIPH_BASE     0x50000000U  // Base address of AHB2 peripherals

/*
* Base addresses of peripherals which are hanging on AHB1 bus
* TODO : Complete for all other peripherals
*/

#define GPIOA_BASEADDR     (AHB1PERIPH_BASE + 0x0000U)  // GPIO Port A
#define GPIOB_BASEADDR     (AHB1PERIPH_BASE + 0x0400U)  // GPIO Port B
#define GPIOC_BASEADDR     (AHB1PERIPH_BASE + 0x0800U)  // GPIO Port C
#define GPIOD_BASEADDR     (AHB1PERIPH_BASE + 0x0C00U)  // GPIO Port D
#define GPIOE_BASEADDR     (AHB1PERIPH_BASE + 0x1000U)  // GPIO Port E
#define GPIOF_BASEADDR     (AHB1PERIPH_BASE + 0x1400U)  // GPIO Port F
#define GPIOG_BASEADDR     (AHB1PERIPH_BASE + 0x1800U)  // GPIO Port G
#define GPIOH_BASEADDR     (AHB1PERIPH_BASE + 0x1C00U)  // GPIO Port H

#define CRC_BASEADDR       (AHB1PERIPH_BASE + 0x3000U)  // CRC unit
#define RCC_BASEADDR       (AHB1PERIPH_BASE + 0x3800U)  // Reset and Clock Control
#define FLASH_R_BASEADDR   (AHB1PERIPH_BASE + 0x3C00U)  // Flash interface
#define BKPSRAM_BASEADDR   (AHB1PERIPH_BASE + 0x4000U)  // Backup SRAM
#define DMA1_BASEADDR      (AHB1PERIPH_BASE + 0x6000U)  // DMA controller 1
#define DMA2_BASEADDR      (AHB1PERIPH_BASE + 0x6400U)  // DMA controller 2

/*
* Base addresses of peripherals which are hanging on APB1 bus
* TODO : Complete for all other peripherals
*/

/* Peripherals hanging on APB1 bus */

#define TIM2_BASEADDR       (APB1PERIPH_BASE + 0x0000U)  // Timer 2
#define TIM3_BASEADDR       (APB1PERIPH_BASE + 0x0400U)  // Timer 3
#define TIM4_BASEADDR       (APB1PERIPH_BASE + 0x0800U)  // Timer 4
#define TIM5_BASEADDR       (APB1PERIPH_BASE + 0x0C00U)  // Timer 5
#define TIM6_BASEADDR       (APB1PERIPH_BASE + 0x1000U)  // Basic Timer 6
#define TIM7_BASEADDR       (APB1PERIPH_BASE + 0x1400U)  // Basic Timer 7

#define TIM12_BASEADDR      (APB1PERIPH_BASE + 0x1800U)  // Timer 12
#define TIM13_BASEADDR      (APB1PERIPH_BASE + 0x1C00U)  // Timer 13
#define TIM14_BASEADDR      (APB1PERIPH_BASE + 0x2000U)  // Timer 14

#define RTC_BASEADDR        (APB1PERIPH_BASE + 0x2800U)  // Real-Time Clock
#define WWDG_BASEADDR       (APB1PERIPH_BASE + 0x2C00U)  // Window Watchdog
#define IWDG_BASEADDR       (APB1PERIPH_BASE + 0x3000U)  // Independent Watchdog

#define SPI2_BASEADDR       (APB1PERIPH_BASE + 0x3800U)  // SPI2 / I2S2
#define SPI3_BASEADDR       (APB1PERIPH_BASE + 0x3C00U)  // SPI3 / I2S3
#define SPDIFRX_BASEADDR    (APB1PERIPH_BASE + 0x4000U)  // S/PDIF receiver

#define USART2_BASEADDR     (APB1PERIPH_BASE + 0x4400U)  // USART2
#define USART3_BASEADDR     (APB1PERIPH_BASE + 0x4800U)  // USART3
#define UART4_BASEADDR      (APB1PERIPH_BASE + 0x4C00U)  // UART4
#define UART5_BASEADDR      (APB1PERIPH_BASE + 0x5000U)  // UART5

#define I2C1_BASEADDR       (APB1PERIPH_BASE + 0x5400U)  // I2C1
#define I2C2_BASEADDR       (APB1PERIPH_BASE + 0x5800U)  // I2C2
#define I2C3_BASEADDR       (APB1PERIPH_BASE + 0x5C00U)  // I2C3
#define FMPI2C1_BASEADDR    (APB1PERIPH_BASE + 0x6000U)  // Fast-mode Plus I2C1

#define CAN1_BASEADDR       (APB1PERIPH_BASE + 0x6400U)  // CAN controller 1
#define CAN2_BASEADDR       (APB1PERIPH_BASE + 0x6800U)  // CAN controller 2
#define CEC_BASEADDR        (APB1PERIPH_BASE + 0x6C00U)  // HDMI-CEC controller

#define PWR_BASEADDR        (APB1PERIPH_BASE + 0x7000U)  // Power controller
#define DAC_BASEADDR        (APB1PERIPH_BASE + 0x7400U)  // Digital-to-Analog Converter


/*
* Base addresses of peripherals which are hanging on APB2 bus
* TODO : Complete for all other peripherals
*/

/* Peripherals hanging on APB2 bus */

#define TIM1_BASEADDR       (APB2PERIPH_BASE + 0x0000U)  // Advanced-control Timer 1
#define TIM8_BASEADDR       (APB2PERIPH_BASE + 0x0400U)  // Advanced-control Timer 8

#define USART1_BASEADDR     (APB2PERIPH_BASE + 0x1000U)  // USART1
#define USART6_BASEADDR     (APB2PERIPH_BASE + 0x1400U)  // USART6

#define ADC1_BASEADDR       (APB2PERIPH_BASE + 0x2000U)  // ADC1
#define ADC2_BASEADDR       (APB2PERIPH_BASE + 0x2100U)  // ADC2
#define ADC3_BASEADDR       (APB2PERIPH_BASE + 0x2200U)  // ADC3
#define ADC_COMMON_BASEADDR (APB2PERIPH_BASE + 0x2300U)  // ADC common registers

#define SDIO_BASEADDR       (APB2PERIPH_BASE + 0x2C00U)  // SDIO/SDMMC interface

#define SPI1_BASEADDR       (APB2PERIPH_BASE + 0x3000U)  // SPI1
#define SPI4_BASEADDR       (APB2PERIPH_BASE + 0x3400U)  // SPI4

#define SYSCFG_BASEADDR     (APB2PERIPH_BASE + 0x3800U)  // System configuration controller
#define EXTI_BASEADDR       (APB2PERIPH_BASE + 0x3C00U)  // External interrupt/event controller

#define TIM9_BASEADDR       (APB2PERIPH_BASE + 0x4000U)  // Timer 9
#define TIM10_BASEADDR      (APB2PERIPH_BASE + 0x4400U)  // Timer 10
#define TIM11_BASEADDR      (APB2PERIPH_BASE + 0x4800U)  // Timer 11

#define SAI1_BASEADDR       (APB2PERIPH_BASE + 0x5800U)  // Serial Audio Interface 1
#define SAI2_BASEADDR       (APB2PERIPH_BASE + 0x5C00U)  // Serial Audio Interface 2

/*
* Note : Registers of a peripheral are specific to MCU
* e.g : Number of Registers of SPI peripheral of STM32F4x family of MCUs may be different(more or less)
* Compared to number of registers of SPI peripheral of STM32Lx or STM32F0x family of MCUs
* Please check your Device RM (our case Table 26. GPIO register map and reset values)
* Below is a peripheral Registers definition structure for GPIO
*/

typedef struct
{
    volatile uint32_t MODER;      // Offset 0x00: Mode register  uint32_t = #include<stdint.h>
    volatile uint32_t OTYPER;     // Offset 0x04: Output type register
    volatile uint32_t OSPEEDR;    // Offset 0x08: Output speed register
    volatile uint32_t PUPDR;      // Offset 0x0C: Pull-up/Pull-down register
    volatile uint32_t IDR;        // Offset 0x10: Input data register
    volatile uint32_t ODR;        // Offset 0x14: Output data register
    volatile uint32_t BSRR;       // Offset 0x18: Bit set/reset register
    volatile uint32_t LCKR;       // Offset 0x1C: Configuration lock register
    volatile uint32_t AFR[2];     // Offset 0x20, 0x24: Alternate function registers

} GPIO_RegDef_t;

/*
* Below is a peripheral Registers definition structure for RCC
*/
typedef struct
{
    volatile uint32_t CR;            // Offset 0x00: RCC clock control register
    volatile uint32_t PLLCFGR;       // Offset 0x04: RCC PLL configuration register
    volatile uint32_t CFGR;          // Offset 0x08: RCC clock configuration register
    volatile uint32_t CIR;           // Offset 0x0C: RCC clock interrupt register

    volatile uint32_t AHB1RSTR;      // Offset 0x10: AHB1 peripheral reset register
    volatile uint32_t AHB2RSTR;      // Offset 0x14: AHB2 peripheral reset register
    volatile uint32_t AHB3RSTR;      // Offset 0x18: AHB3 peripheral reset register
    uint32_t RESERVED0;              // Offset 0x1C: Reserved

    volatile uint32_t APB1RSTR;      // Offset 0x20: APB1 peripheral reset register
    volatile uint32_t APB2RSTR;      // Offset 0x24: APB2 peripheral reset register
    uint32_t RESERVED1[2];           // Offset 0x28-0x2C: Reserved

    volatile uint32_t AHB1ENR;       // Offset 0x30: AHB1 peripheral clock enable register
    volatile uint32_t AHB2ENR;       // Offset 0x34: AHB2 peripheral clock enable register
    volatile uint32_t AHB3ENR;       // Offset 0x38: AHB3 peripheral clock enable register
    uint32_t RESERVED2;              // Offset 0x3C: Reserved

    volatile uint32_t APB1ENR;       // Offset 0x40: APB1 peripheral clock enable register
    volatile uint32_t APB2ENR;       // Offset 0x44: APB2 peripheral clock enable register
    uint32_t RESERVED3[2];           // Offset 0x48-0x4C: Reserved

    volatile uint32_t AHB1LPENR;     // Offset 0x50: AHB1 clock enable in low-power mode
    volatile uint32_t AHB2LPENR;     // Offset 0x54: AHB2 clock enable in low-power mode
    volatile uint32_t AHB3LPENR;     // Offset 0x58: AHB3 clock enable in low-power mode
    uint32_t RESERVED4;              // Offset 0x5C: Reserved

    volatile uint32_t APB1LPENR;     // Offset 0x60: APB1 clock enable in low-power mode
    volatile uint32_t APB2LPENR;     // Offset 0x64: APB2 clock enable in low-power mode
    uint32_t RESERVED5[2];           // Offset 0x68-0x6C: Reserved

    volatile uint32_t BDCR;          // Offset 0x70: Backup domain control register
    volatile uint32_t CSR;           // Offset 0x74: Clock control & status register
    uint32_t RESERVED6[2];           // Offset 0x78-0x7C: Reserved

    volatile uint32_t SSCGR;         // Offset 0x80: Spread spectrum clock generation register
    volatile uint32_t PLLI2SCFGR;    // Offset 0x84: PLLI2S configuration register
    volatile uint32_t PLLSAICFGR;    // Offset 0x88: PLLSAI configuration register
    volatile uint32_t DCKCFGR;       // Offset 0x8C: Dedicated clocks configuration register
    volatile uint32_t CKGATENR;      // Offset 0x90: Clocks gated enable register
    volatile uint32_t DCKCFGR2;      // Offset 0x94: Dedicated clocks configuration register 2

} RCC_RegDef_t;



/*
* Below is a peripheral Registers definition structure for EXTI
*/
typedef struct
{
    volatile uint32_t IMR;           // Offset 0x00: Interrupt mask register
    volatile uint32_t EMR;           // Offset 0x04: Event mask register
    volatile uint32_t RTSR;          // Offset 0x08: Rising trigger selection register
    volatile uint32_t FTSR;          // Offset 0x0C: Falling trigger selection register
    volatile uint32_t SWIER;         // Offset 0x10: Software interrupt event register
    volatile uint32_t PR;            // Offset 0x14: Pending register

} EXTI_RegDef_t;


/*
* Below is a peripheral Registers definition structure for SYSCFG
*/
typedef struct
{
    volatile uint32_t MEMRMP;        // Offset 0x00: Memory remap register
    volatile uint32_t PMC;           // Offset 0x04: Peripheral mode configuration register
    volatile uint32_t EXTICR[4];     // Offset 0x08-0x14: External interrupt configuration registers 1-4
    uint32_t RESERVED0[2];           // Offset 0x18-0x1C: Reserved
    volatile uint32_t CMPCR;         // Offset 0x20: Compensation cell control register
    uint32_t RESERVED1[2];           // Offset 0x24-0x28: Reserved
    volatile uint32_t CFGR;          // Offset 0x2C: Configuration register

} SYSCFG_RegDef_t;



/*
* Below is a peripheral Registers definition structure for USART / UART
* Please check your Device RM (Table 143. USART register map and reset values)
* NOTE: UART4 and UART5 use the same structure; their CR2 CLKEN/CPOL/CPHA/LBCL
*       bits and the GTPR register simply have no effect.
*/
typedef struct
{
    volatile uint32_t SR;            // Offset 0x00: Status register
    volatile uint32_t DR;            // Offset 0x04: Data register
    volatile uint32_t BRR;           // Offset 0x08: Baud rate register
    volatile uint32_t CR1;           // Offset 0x0C: Control register 1
    volatile uint32_t CR2;           // Offset 0x10: Control register 2
    volatile uint32_t CR3;           // Offset 0x14: Control register 3
    volatile uint32_t GTPR;          // Offset 0x18: Guard time and prescaler register

} USART_RegDef_t;


/*
* Below is a peripheral Registers definition structure for ADC
* Please check your Device RM (ADC register map)
* NOTE: ADC1, ADC2 and ADC3 each have their OWN copy of these registers.
*       The prescaler and the internal-channel enable bits live in the
*       SHARED common block below, not here.
*/
typedef struct
{
    volatile uint32_t SR;            // Offset 0x00: Status register
    volatile uint32_t CR1;           // Offset 0x04: Control register 1
    volatile uint32_t CR2;           // Offset 0x08: Control register 2
    volatile uint32_t SMPR1;         // Offset 0x0C: Sample time register 1, channels 10 to 18
    volatile uint32_t SMPR2;         // Offset 0x10: Sample time register 2, channels 0 to 9
    volatile uint32_t JOFR[4];       // Offset 0x14-0x20: Injected channel data offset registers
    volatile uint32_t HTR;           // Offset 0x24: Watchdog higher threshold register
    volatile uint32_t LTR;           // Offset 0x28: Watchdog lower threshold register
    volatile uint32_t SQR1;          // Offset 0x2C: Regular sequence register 1, length L and SQ13-16
    volatile uint32_t SQR2;          // Offset 0x30: Regular sequence register 2, SQ7-12
    volatile uint32_t SQR3;          // Offset 0x34: Regular sequence register 3, SQ1-6 (the FIRST conversions)
    volatile uint32_t JSQR;          // Offset 0x38: Injected sequence register
    volatile uint32_t JDR[4];        // Offset 0x3C-0x48: Injected data registers
    volatile uint32_t DR;            // Offset 0x4C: Regular data register, reading it clears EOC

} ADC_RegDef_t;


/*
* Below is a peripheral Registers definition structure for the ADC COMMON block
* One shared instance at 0x40012300 serving all three ADCs.
*/
typedef struct
{
    volatile uint32_t CSR;           // Offset 0x00: Common status register, all three ADCs
    volatile uint32_t CCR;           // Offset 0x04: Common control register, prescaler and TSVREFE
    volatile uint32_t CDR;           // Offset 0x08: Common regular data register for dual/triple mode

} ADC_Common_RegDef_t;


/*
* Below is a peripheral Registers definition structure for SPI
* Please check your Device RM (SPI register map)
* NOTE: SPI2 and SPI3 share this block with I2S; the I2SCFGR and I2SPR
*       registers are ignored while the peripheral runs in SPI mode.
*/
typedef struct
{
    volatile uint32_t CR1;           // Offset 0x00: Control register 1
    volatile uint32_t CR2;           // Offset 0x04: Control register 2
    volatile uint32_t SR;            // Offset 0x08: Status register
    volatile uint32_t DR;            // Offset 0x0C: Data register
    volatile uint32_t CRCPR;         // Offset 0x10: CRC polynomial register
    volatile uint32_t RXCRCR;        // Offset 0x14: Rx CRC register
    volatile uint32_t TXCRCR;        // Offset 0x18: Tx CRC register
    volatile uint32_t I2SCFGR;       // Offset 0x1C: I2S configuration register
    volatile uint32_t I2SPR;         // Offset 0x20: I2S prescaler register

} SPI_RegDef_t;


/*
* Below is a peripheral Registers definition structure for I2C
* Please check your Device RM (I2C register map)
*/
typedef struct
{
    volatile uint32_t CR1;           // Offset 0x00: Control register 1
    volatile uint32_t CR2;           // Offset 0x04: Control register 2
    volatile uint32_t OAR1;          // Offset 0x08: Own address register 1
    volatile uint32_t OAR2;          // Offset 0x0C: Own address register 2
    volatile uint32_t DR;            // Offset 0x10: Data register
    volatile uint32_t SR1;           // Offset 0x14: Status register 1
    volatile uint32_t SR2;           // Offset 0x18: Status register 2
    volatile uint32_t CCR;           // Offset 0x1C: Clock control register
    volatile uint32_t TRISE;         // Offset 0x20: Rise time register
    volatile uint32_t FLTR;          // Offset 0x24: Noise filter register

} I2C_RegDef_t;


/*
* Below is a Tx mailbox structure for the bxCAN peripheral
* Three identical mailboxes start at offset 0x180.
*/
typedef struct
{
    volatile uint32_t TIR;           // Tx mailbox identifier register
    volatile uint32_t TDTR;          // Tx mailbox data length and timestamp
    volatile uint32_t TDLR;          // Tx mailbox data low register, bytes 0 to 3
    volatile uint32_t TDHR;          // Tx mailbox data high register, bytes 4 to 7

} CAN_TxMailBox_t;


/*
* Below is an Rx FIFO mailbox structure for the bxCAN peripheral
* Two identical FIFOs start at offset 0x1B0.
*/
typedef struct
{
    volatile uint32_t RIR;           // Rx FIFO mailbox identifier register
    volatile uint32_t RDTR;          // Rx FIFO mailbox data length and timestamp
    volatile uint32_t RDLR;          // Rx FIFO mailbox data low register
    volatile uint32_t RDHR;          // Rx FIFO mailbox data high register

} CAN_FIFOMailBox_t;


/*
* Below is a filter bank register pair for the bxCAN peripheral
* 28 banks start at offset 0x240.
*/
typedef struct
{
    volatile uint32_t FR1;           // Filter bank register 1
    volatile uint32_t FR2;           // Filter bank register 2

} CAN_FilterRegister_t;


/*
* Below is a peripheral Registers definition structure for bxCAN
* Please check your Device RM (bxCAN register map). The reserved gaps
* below are real - do not remove them or every offset after them shifts.
*/
typedef struct
{
    volatile uint32_t MCR;                       // Offset 0x000: Master control register
    volatile uint32_t MSR;                       // Offset 0x004: Master status register
    volatile uint32_t TSR;                       // Offset 0x008: Transmit status register
    volatile uint32_t RF0R;                      // Offset 0x00C: Rx FIFO 0 register
    volatile uint32_t RF1R;                      // Offset 0x010: Rx FIFO 1 register
    volatile uint32_t IER;                       // Offset 0x014: Interrupt enable register
    volatile uint32_t ESR;                       // Offset 0x018: Error status register
    volatile uint32_t BTR;                       // Offset 0x01C: Bit timing register
    uint32_t RESERVED0[88];                      // Offset 0x020 to 0x17F: Reserved
    CAN_TxMailBox_t sTxMailBox[3];               // Offset 0x180: Three Tx mailboxes
    CAN_FIFOMailBox_t sFIFOMailBox[2];           // Offset 0x1B0: Two Rx FIFOs
    uint32_t RESERVED1[12];                      // Offset 0x1D0 to 0x1FF: Reserved
    volatile uint32_t FMR;                       // Offset 0x200: Filter master register
    volatile uint32_t FM1R;                      // Offset 0x204: Filter mode register
    uint32_t RESERVED2;                          // Offset 0x208: Reserved
    volatile uint32_t FS1R;                      // Offset 0x20C: Filter scale register
    uint32_t RESERVED3;                          // Offset 0x210: Reserved
    volatile uint32_t FFA1R;                     // Offset 0x214: Filter FIFO assignment
    uint32_t RESERVED4;                          // Offset 0x218: Reserved
    volatile uint32_t FA1R;                      // Offset 0x21C: Filter activation register
    uint32_t RESERVED5[8];                       // Offset 0x220 to 0x23F: Reserved
    CAN_FilterRegister_t sFilterRegister[28];    // Offset 0x240: 28 filter banks

} CAN_RegDef_t;

/*
*peripheral definitions ( Peripheral base addresses typecasted to xxx_RegDef_t)
*/

#define GPIOA   ((GPIO_RegDef_t*)GPIOA_BASEADDR)  // GPIOA @ 0x40020000
#define GPIOB   ((GPIO_RegDef_t*)GPIOB_BASEADDR)  // GPIOB @ 0x40020400
#define GPIOC   ((GPIO_RegDef_t*)GPIOC_BASEADDR)  // GPIOC @ 0x40020800
#define GPIOD   ((GPIO_RegDef_t*)GPIOD_BASEADDR)  // GPIOD @ 0x40020C00
#define GPIOE   ((GPIO_RegDef_t*)GPIOE_BASEADDR)  // GPIOE @ 0x40021000
#define GPIOF   ((GPIO_RegDef_t*)GPIOF_BASEADDR)  // GPIOF @ 0x40021400
#define GPIOG   ((GPIO_RegDef_t*)GPIOG_BASEADDR)  // GPIOG @ 0x40021800
#define GPIOH   ((GPIO_RegDef_t*)GPIOH_BASEADDR)  // GPIOH @ 0x40021C00

#define RCC    ((RCC_RegDef_t *)RCC_BASEADDR)  // RCC @ 0x40023800

#define EXTI   ((EXTI_RegDef_t *)EXTI_BASEADDR)      // EXTI @ 0x40013C00
#define SYSCFG ((SYSCFG_RegDef_t *)SYSCFG_BASEADDR)  // SYSCFG @ 0x40013800

#define ADC1   ((ADC_RegDef_t *)ADC1_BASEADDR)               // ADC1 @ 0x40012000 - APB2
#define ADC2   ((ADC_RegDef_t *)ADC2_BASEADDR)               // ADC2 @ 0x40012100 - APB2
#define ADC3   ((ADC_RegDef_t *)ADC3_BASEADDR)               // ADC3 @ 0x40012200 - APB2
#define ADC    ((ADC_Common_RegDef_t *)ADC_COMMON_BASEADDR)  // Shared block @ 0x40012300

#define SPI1   ((SPI_RegDef_t *)SPI1_BASEADDR)       // SPI1 @ 0x40013000 - APB2, fastest
#define SPI2   ((SPI_RegDef_t *)SPI2_BASEADDR)       // SPI2 @ 0x40003800 - APB1
#define SPI3   ((SPI_RegDef_t *)SPI3_BASEADDR)       // SPI3 @ 0x40003C00 - APB1
#define SPI4   ((SPI_RegDef_t *)SPI4_BASEADDR)       // SPI4 @ 0x40013400 - APB2

#define I2C1   ((I2C_RegDef_t *)I2C1_BASEADDR)       // I2C1 @ 0x40005400 - APB1
#define I2C2   ((I2C_RegDef_t *)I2C2_BASEADDR)       // I2C2 @ 0x40005800 - APB1
#define I2C3   ((I2C_RegDef_t *)I2C3_BASEADDR)       // I2C3 @ 0x40005C00 - APB1

#define CAN1   ((CAN_RegDef_t *)CAN1_BASEADDR)       // CAN1 @ 0x40006400 - APB1, master
#define CAN2   ((CAN_RegDef_t *)CAN2_BASEADDR)       // CAN2 @ 0x40006800 - APB1, slave

#define USART1 ((USART_RegDef_t *)USART1_BASEADDR)   // USART1 @ 0x40011000 - APB2
#define USART2 ((USART_RegDef_t *)USART2_BASEADDR)   // USART2 @ 0x40004400 - APB1 (Nucleo VCP)
#define USART3 ((USART_RegDef_t *)USART3_BASEADDR)   // USART3 @ 0x40004800 - APB1
#define UART4  ((USART_RegDef_t *)UART4_BASEADDR)    // UART4  @ 0x40004C00 - APB1
#define UART5  ((USART_RegDef_t *)UART5_BASEADDR)    // UART5  @ 0x40005000 - APB1
#define USART6 ((USART_RegDef_t *)USART6_BASEADDR)   // USART6 @ 0x40011400 - APB2


// ----------------------- Enable MAcros Starts ------------------------------------

/*
 * Clock Enable Macros for GPIOx peripherals - AHB1
 */
#define GPIOA_PCLK_EN()     (RCC->AHB1ENR |= (1U << 0))   // Enable GPIOA clock
#define GPIOB_PCLK_EN()     (RCC->AHB1ENR |= (1U << 1))   // Enable GPIOB clock
#define GPIOC_PCLK_EN()     (RCC->AHB1ENR |= (1U << 2))   // Enable GPIOC clock
#define GPIOD_PCLK_EN()     (RCC->AHB1ENR |= (1U << 3))   // Enable GPIOD clock
#define GPIOE_PCLK_EN()     (RCC->AHB1ENR |= (1U << 4))   // Enable GPIOE clock
#define GPIOF_PCLK_EN()     (RCC->AHB1ENR |= (1U << 5))   // Enable GPIOF clock
#define GPIOG_PCLK_EN()     (RCC->AHB1ENR |= (1U << 6))   // Enable GPIOG clock
#define GPIOH_PCLK_EN()     (RCC->AHB1ENR |= (1U << 7))   // Enable GPIOH clock


/*
 * Clock Enable Macros for other AHB1 peripherals
 */
#define CRC_PCLK_EN()       (RCC->AHB1ENR |= (1U << 12))  // Enable CRC clock
#define BKPSRAM_PCLK_EN()   (RCC->AHB1ENR |= (1U << 18))  // Enable Backup SRAM clock
#define DMA1_PCLK_EN()      (RCC->AHB1ENR |= (1U << 21))  // Enable DMA1 clock
#define DMA2_PCLK_EN()      (RCC->AHB1ENR |= (1U << 22))  // Enable DMA2 clock


/*
 * Clock Enable Macros for AHB2 peripherals
 */
#define OTGFS_PCLK_EN()     (RCC->AHB2ENR |= (1U << 7))   // Enable USB OTG FS clock


/*
 * Clock Enable Macros for TIMx peripherals - APB1
 */
#define TIM2_PCLK_EN()      (RCC->APB1ENR |= (1U << 0))   // Enable TIM2 clock
#define TIM3_PCLK_EN()      (RCC->APB1ENR |= (1U << 1))   // Enable TIM3 clock
#define TIM4_PCLK_EN()      (RCC->APB1ENR |= (1U << 2))   // Enable TIM4 clock
#define TIM5_PCLK_EN()      (RCC->APB1ENR |= (1U << 3))   // Enable TIM5 clock
#define TIM6_PCLK_EN()      (RCC->APB1ENR |= (1U << 4))   // Enable TIM6 clock
#define TIM7_PCLK_EN()      (RCC->APB1ENR |= (1U << 5))   // Enable TIM7 clock
#define TIM12_PCLK_EN()     (RCC->APB1ENR |= (1U << 6))   // Enable TIM12 clock
#define TIM13_PCLK_EN()     (RCC->APB1ENR |= (1U << 7))   // Enable TIM13 clock
#define TIM14_PCLK_EN()     (RCC->APB1ENR |= (1U << 8))   // Enable TIM14 clock


/*
 * Clock Enable Macro for WWDG peripheral - APB1
 */
#define WWDG_PCLK_EN()      (RCC->APB1ENR |= (1U << 11))  // Enable Window Watchdog clock


/*
 * Clock Enable Macros for SPIx peripherals - APB1
 */
#define SPI2_PCLK_EN()      (RCC->APB1ENR |= (1U << 14))  // Enable SPI2 clock
#define SPI3_PCLK_EN()      (RCC->APB1ENR |= (1U << 15))  // Enable SPI3 clock


/*
 * Clock Enable Macro for SPDIFRX - APB1
 */
#define SPDIFRX_PCLK_EN()   (RCC->APB1ENR |= (1U << 16))  // Enable S/PDIF receiver clock


/*
 * Clock Enable Macros for USART/UART peripherals - APB1
 */
#define USART2_PCLK_EN()    (RCC->APB1ENR |= (1U << 17))  // Enable USART2 clock
#define USART3_PCLK_EN()    (RCC->APB1ENR |= (1U << 18))  // Enable USART3 clock
#define UART4_PCLK_EN()     (RCC->APB1ENR |= (1U << 19))  // Enable UART4 clock
#define UART5_PCLK_EN()     (RCC->APB1ENR |= (1U << 20))  // Enable UART5 clock


/*
 * Clock Enable Macros for I2Cx peripherals - APB1
 */
#define I2C1_PCLK_EN()      (RCC->APB1ENR |= (1U << 21))  // Enable I2C1 clock
#define I2C2_PCLK_EN()      (RCC->APB1ENR |= (1U << 22))  // Enable I2C2 clock
#define I2C3_PCLK_EN()      (RCC->APB1ENR |= (1U << 23))  // Enable I2C3 clock
#define FMPI2C1_PCLK_EN()   (RCC->APB1ENR |= (1U << 24))  // Enable FM+ I2C1 clock


/*
 * Clock Enable Macros for CANx peripherals - APB1
 */
#define CAN1_PCLK_EN()      (RCC->APB1ENR |= (1U << 25))  // Enable CAN1 clock
#define CAN2_PCLK_EN()      (RCC->APB1ENR |= (1U << 26))  // Enable CAN2 clock


/*
 * Clock Enable Macro for CEC peripheral - APB1
 */
#define CEC_PCLK_EN()       (RCC->APB1ENR |= (1U << 27))  // Enable HDMI-CEC clock


/*
 * Clock Enable Macro for PWR peripheral - APB1
 */
#define PWR_PCLK_EN()       (RCC->APB1ENR |= (1U << 28))  // Enable Power Controller clock


/*
 * Clock Enable Macro for DAC peripheral - APB1
 */
#define DAC_PCLK_EN()       (RCC->APB1ENR |= (1U << 29))  // Enable DAC clock


/*
 * Clock Enable Macros for TIMx peripherals - APB2
 */
#define TIM1_PCLK_EN()      (RCC->APB2ENR |= (1U << 0))   // Enable TIM1 clock
#define TIM8_PCLK_EN()      (RCC->APB2ENR |= (1U << 1))   // Enable TIM8 clock
#define TIM9_PCLK_EN()      (RCC->APB2ENR |= (1U << 16))  // Enable TIM9 clock
#define TIM10_PCLK_EN()     (RCC->APB2ENR |= (1U << 17))  // Enable TIM10 clock
#define TIM11_PCLK_EN()     (RCC->APB2ENR |= (1U << 18))  // Enable TIM11 clock


/*
 * Clock Enable Macros for USARTx peripherals - APB2
 */
#define USART1_PCLK_EN()    (RCC->APB2ENR |= (1U << 4))   // Enable USART1 clock
#define USART6_PCLK_EN()    (RCC->APB2ENR |= (1U << 5))   // Enable USART6 clock


/*
 * Clock Enable Macros for ADCx peripherals - APB2
 */
#define ADC1_PCLK_EN()      (RCC->APB2ENR |= (1U << 8))   // Enable ADC1 clock
#define ADC2_PCLK_EN()      (RCC->APB2ENR |= (1U << 9))   // Enable ADC2 clock
#define ADC3_PCLK_EN()      (RCC->APB2ENR |= (1U << 10))  // Enable ADC3 clock


/*
 * Clock Enable Macro for SDIO peripheral - APB2
 */
#define SDIO_PCLK_EN()      (RCC->APB2ENR |= (1U << 11))  // Enable SDIO clock


/*
 * Clock Enable Macros for SPIx peripherals - APB2
 */
#define SPI1_PCLK_EN()      (RCC->APB2ENR |= (1U << 12))  // Enable SPI1 clock
#define SPI4_PCLK_EN()      (RCC->APB2ENR |= (1U << 13))  // Enable SPI4 clock


/*
 * Clock Enable Macro for SYSCFG peripheral - APB2
 */
#define SYSCFG_PCLK_EN()    (RCC->APB2ENR |= (1U << 14))  // Enable SYSCFG clock


/*
 * Clock Enable Macros for SAIx peripherals - APB2
 */
#define SAI1_PCLK_EN()      (RCC->APB2ENR |= (1U << 22))  // Enable SAI1 clock
#define SAI2_PCLK_EN()      (RCC->APB2ENR |= (1U << 23))  // Enable SAI2 clock

// ----------------- Disable Macros Starts --------------------------------------------

/*
 * Clock Disable Macros for GPIOx peripherals - AHB1
 */
#define GPIOA_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 0))   // Disable GPIOA clock
#define GPIOB_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 1))   // Disable GPIOB clock
#define GPIOC_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 2))   // Disable GPIOC clock
#define GPIOD_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 3))   // Disable GPIOD clock
#define GPIOE_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 4))   // Disable GPIOE clock
#define GPIOF_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 5))   // Disable GPIOF clock
#define GPIOG_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 6))   // Disable GPIOG clock
#define GPIOH_PCLK_DI()     (RCC->AHB1ENR &= ~(1U << 7))   // Disable GPIOH clock


/*
 * Clock Disable Macros for other AHB1 peripherals
 */
#define CRC_PCLK_DI()       (RCC->AHB1ENR &= ~(1U << 12))  // Disable CRC clock
#define BKPSRAM_PCLK_DI()   (RCC->AHB1ENR &= ~(1U << 18))  // Disable Backup SRAM clock
#define DMA1_PCLK_DI()      (RCC->AHB1ENR &= ~(1U << 21))  // Disable DMA1 clock
#define DMA2_PCLK_DI()      (RCC->AHB1ENR &= ~(1U << 22))  // Disable DMA2 clock


/*
 * Clock Disable Macros for AHB2 peripherals
 */
#define OTGFS_PCLK_DI()     (RCC->AHB2ENR &= ~(1U << 7))   // Disable USB OTG FS clock


/*
 * Clock Disable Macros for TIMx peripherals - APB1
 */
#define TIM2_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 0))   // Disable TIM2 clock
#define TIM3_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 1))   // Disable TIM3 clock
#define TIM4_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 2))   // Disable TIM4 clock
#define TIM5_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 3))   // Disable TIM5 clock
#define TIM6_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 4))   // Disable TIM6 clock
#define TIM7_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 5))   // Disable TIM7 clock
#define TIM12_PCLK_DI()     (RCC->APB1ENR &= ~(1U << 6))   // Disable TIM12 clock
#define TIM13_PCLK_DI()     (RCC->APB1ENR &= ~(1U << 7))   // Disable TIM13 clock
#define TIM14_PCLK_DI()     (RCC->APB1ENR &= ~(1U << 8))   // Disable TIM14 clock


/*
 * Clock Disable Macro for WWDG peripheral - APB1
 */
#define WWDG_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 11))  // Disable Window Watchdog clock


/*
 * Clock Disable Macros for SPIx peripherals - APB1
 */
#define SPI2_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 14))  // Disable SPI2 clock
#define SPI3_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 15))  // Disable SPI3 clock


/*
 * Clock Disable Macro for SPDIFRX peripheral - APB1
 */
#define SPDIFRX_PCLK_DI()   (RCC->APB1ENR &= ~(1U << 16))  // Disable S/PDIF receiver clock


/*
 * Clock Disable Macros for USART/UART peripherals - APB1
 */
#define USART2_PCLK_DI()    (RCC->APB1ENR &= ~(1U << 17))  // Disable USART2 clock
#define USART3_PCLK_DI()    (RCC->APB1ENR &= ~(1U << 18))  // Disable USART3 clock
#define UART4_PCLK_DI()     (RCC->APB1ENR &= ~(1U << 19))  // Disable UART4 clock
#define UART5_PCLK_DI()     (RCC->APB1ENR &= ~(1U << 20))  // Disable UART5 clock


/*
 * Clock Disable Macros for I2Cx peripherals - APB1
 */
#define I2C1_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 21))  // Disable I2C1 clock
#define I2C2_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 22))  // Disable I2C2 clock
#define I2C3_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 23))  // Disable I2C3 clock
#define FMPI2C1_PCLK_DI()   (RCC->APB1ENR &= ~(1U << 24))  // Disable FM+ I2C1 clock


/*
 * Clock Disable Macros for CANx peripherals - APB1
 */
#define CAN1_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 25))  // Disable CAN1 clock
#define CAN2_PCLK_DI()      (RCC->APB1ENR &= ~(1U << 26))  // Disable CAN2 clock


/*
 * Clock Disable Macro for CEC peripheral - APB1
 */
#define CEC_PCLK_DI()       (RCC->APB1ENR &= ~(1U << 27))  // Disable HDMI-CEC clock


/*
 * Clock Disable Macro for PWR peripheral - APB1
 */
#define PWR_PCLK_DI()       (RCC->APB1ENR &= ~(1U << 28))  // Disable Power Controller clock


/*
 * Clock Disable Macro for DAC peripheral - APB1
 */
#define DAC_PCLK_DI()       (RCC->APB1ENR &= ~(1U << 29))  // Disable DAC clock


/*
 * Clock Disable Macros for TIMx peripherals - APB2
 */
#define TIM1_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 0))   // Disable TIM1 clock
#define TIM8_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 1))   // Disable TIM8 clock
#define TIM9_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 16))  // Disable TIM9 clock
#define TIM10_PCLK_DI()     (RCC->APB2ENR &= ~(1U << 17))  // Disable TIM10 clock
#define TIM11_PCLK_DI()     (RCC->APB2ENR &= ~(1U << 18))  // Disable TIM11 clock


/*
 * Clock Disable Macros for USARTx peripherals - APB2
 */
#define USART1_PCLK_DI()    (RCC->APB2ENR &= ~(1U << 4))   // Disable USART1 clock
#define USART6_PCLK_DI()    (RCC->APB2ENR &= ~(1U << 5))   // Disable USART6 clock


/*
 * Clock Disable Macros for ADCx peripherals - APB2
 */
#define ADC1_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 8))   // Disable ADC1 clock
#define ADC2_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 9))   // Disable ADC2 clock
#define ADC3_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 10))  // Disable ADC3 clock


/*
 * Clock Disable Macro for SDIO peripheral - APB2
 */
#define SDIO_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 11))  // Disable SDIO clock


/*
 * Clock Disable Macros for SPIx peripherals - APB2
 */
#define SPI1_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 12))  // Disable SPI1 clock
#define SPI4_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 13))  // Disable SPI4 clock


/*
 * Clock Disable Macro for SYSCFG peripheral - APB2
 */
#define SYSCFG_PCLK_DI()    (RCC->APB2ENR &= ~(1U << 14))  // Disable SYSCFG clock


/*
 * Clock Disable Macros for SAIx peripherals - APB2
 */
#define SAI1_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 22))  // Disable SAI1 clock
#define SAI2_PCLK_DI()      (RCC->APB2ENR &= ~(1U << 23))  // Disable SAI2 clock


//------------ MAcros Definition Start -------------------------------

/*
 * Generic Macros
 */
#define ENABLE                  1U   // Enable peripheral/function
#define DISABLE                 0U   // Disable peripheral/function

#define SET                     ENABLE
#define RESET                   DISABLE

#define GPIO_PIN_SET            SET  // GPIO pin HIGH
#define GPIO_PIN_RESET          RESET // GPIO pin LOW

#define FLAG_SET                SET   // Status flag is set
#define FLAG_RESET              RESET // Status flag is clear


/*
 * GPIO Pin Number Macros
 */
#define GPIO_PIN_NO_0           0U   // GPIO pin 0
#define GPIO_PIN_NO_1           1U   // GPIO pin 1
#define GPIO_PIN_NO_2           2U   // GPIO pin 2
#define GPIO_PIN_NO_3           3U   // GPIO pin 3
#define GPIO_PIN_NO_4           4U   // GPIO pin 4
#define GPIO_PIN_NO_5           5U   // GPIO pin 5
#define GPIO_PIN_NO_6           6U   // GPIO pin 6
#define GPIO_PIN_NO_7           7U   // GPIO pin 7
#define GPIO_PIN_NO_8           8U   // GPIO pin 8
#define GPIO_PIN_NO_9           9U   // GPIO pin 9
#define GPIO_PIN_NO_10          10U  // GPIO pin 10
#define GPIO_PIN_NO_11          11U  // GPIO pin 11
#define GPIO_PIN_NO_12          12U  // GPIO pin 12
#define GPIO_PIN_NO_13          13U  // GPIO pin 13
#define GPIO_PIN_NO_14          14U  // GPIO pin 14
#define GPIO_PIN_NO_15          15U  // GPIO pin 15


/*
 * @GPIO_PIN_MODES
 * GPIO Pin Mode Macros
 */
#define GPIO_MODE_IN            0U   // GPIO input mode taken from refe manual
#define GPIO_MODE_OUT           1U   // GPIO output mode
#define GPIO_MODE_ALTFN         2U   // GPIO alternate-function mode
#define GPIO_MODE_ANALOG        3U   // GPIO analog mode

#define GPIO_MODE_IT_FT         4U   // Interrupt on falling edge
#define GPIO_MODE_IT_RT         5U   // Interrupt on rising edge
#define GPIO_MODE_IT_RFT        6U   // Interrupt on rising/falling edge


/*
 * GPIO Output Type Macros
 */
#define GPIO_OP_TYPE_PP         0U   // Push-pull output
#define GPIO_OP_TYPE_OD         1U   // Open-drain output


/*
 * @GPIO_PIN_SPEED
 * GPIO Output Speed Macros
 */
#define GPIO_SPEED_LOW          0U   // Low-speed output
#define GPIO_SPEED_MEDIUM       1U   // Medium-speed output
#define GPIO_SPEED_FAST         2U   // Fast-speed output
#define GPIO_SPEED_HIGH         3U   // High-speed output


/*
 * GPIO Pull-up / Pull-down Macros
 */
#define GPIO_NO_PUPD            0U   // No pull-up/pull-down
#define GPIO_PIN_PU             1U   // Internal pull-up
#define GPIO_PIN_PD             2U   // Internal pull-down


/*
 * GPIO Alternate Function Macros
 */
#define GPIO_AF0                0U   // Alternate Function 0
#define GPIO_AF1                1U   // Alternate Function 1
#define GPIO_AF2                2U   // Alternate Function 2
#define GPIO_AF3                3U   // Alternate Function 3
#define GPIO_AF4                4U   // Alternate Function 4
#define GPIO_AF5                5U   // Alternate Function 5
#define GPIO_AF6                6U   // Alternate Function 6
#define GPIO_AF7                7U   // Alternate Function 7
#define GPIO_AF8                8U   // Alternate Function 8
#define GPIO_AF9                9U   // Alternate Function 9
#define GPIO_AF10               10U  // Alternate Function 10
#define GPIO_AF11               11U  // Alternate Function 11
#define GPIO_AF12               12U  // Alternate Function 12
#define GPIO_AF13               13U  // Alternate Function 13
#define GPIO_AF14               14U  // Alternate Function 14
#define GPIO_AF15               15U  // Alternate Function 15


/*
 * Macros to reset GPIOx peripherals (set the reset bit, then clear it)
 */
#define GPIOA_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 0)); (RCC->AHB1RSTR &= ~(1U << 0)); }while(0)
#define GPIOB_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 1)); (RCC->AHB1RSTR &= ~(1U << 1)); }while(0)
#define GPIOC_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 2)); (RCC->AHB1RSTR &= ~(1U << 2)); }while(0)
#define GPIOD_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 3)); (RCC->AHB1RSTR &= ~(1U << 3)); }while(0)
#define GPIOE_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 4)); (RCC->AHB1RSTR &= ~(1U << 4)); }while(0)
#define GPIOF_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 5)); (RCC->AHB1RSTR &= ~(1U << 5)); }while(0)
#define GPIOG_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 6)); (RCC->AHB1RSTR &= ~(1U << 6)); }while(0)
#define GPIOH_REG_RESET()   do{ (RCC->AHB1RSTR |= (1U << 7)); (RCC->AHB1RSTR &= ~(1U << 7)); }while(0)


/*
 * Macros to reset USARTx / UARTx peripherals (set the reset bit, then clear it)
 * NOTE: USART1 and USART6 sit on APB2, the rest on APB1.
 */
#define USART1_REG_RESET()  do{ (RCC->APB2RSTR |= (1U << 4));  (RCC->APB2RSTR &= ~(1U << 4));  }while(0)
#define USART2_REG_RESET()  do{ (RCC->APB1RSTR |= (1U << 17)); (RCC->APB1RSTR &= ~(1U << 17)); }while(0)
#define USART3_REG_RESET()  do{ (RCC->APB1RSTR |= (1U << 18)); (RCC->APB1RSTR &= ~(1U << 18)); }while(0)
#define UART4_REG_RESET()   do{ (RCC->APB1RSTR |= (1U << 19)); (RCC->APB1RSTR &= ~(1U << 19)); }while(0)
#define UART5_REG_RESET()   do{ (RCC->APB1RSTR |= (1U << 20)); (RCC->APB1RSTR &= ~(1U << 20)); }while(0)
#define USART6_REG_RESET()  do{ (RCC->APB2RSTR |= (1U << 5));  (RCC->APB2RSTR &= ~(1U << 5));  }while(0)


/*
 * Macro to reset the ADC peripherals
 * WARNING: APB2RSTR bit 8 is a single ADCRST bit that resets ALL THREE ADCs
 *          at once. There is no way to reset just one of them.
 */
#define ADC_REG_RESET()     do{ (RCC->APB2RSTR |= (1U << 8));  (RCC->APB2RSTR &= ~(1U << 8));  }while(0)


/*
 * Macros to reset SPIx peripherals
 * SPI1 and SPI4 sit on APB2, SPI2 and SPI3 on APB1.
 */
#define SPI1_REG_RESET()    do{ (RCC->APB2RSTR |= (1U << 12)); (RCC->APB2RSTR &= ~(1U << 12)); }while(0)
#define SPI2_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 14)); (RCC->APB1RSTR &= ~(1U << 14)); }while(0)
#define SPI3_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 15)); (RCC->APB1RSTR &= ~(1U << 15)); }while(0)
#define SPI4_REG_RESET()    do{ (RCC->APB2RSTR |= (1U << 13)); (RCC->APB2RSTR &= ~(1U << 13)); }while(0)


/*
 * Macros to reset I2Cx peripherals - all three are on APB1
 */
#define I2C1_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 21)); (RCC->APB1RSTR &= ~(1U << 21)); }while(0)
#define I2C2_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 22)); (RCC->APB1RSTR &= ~(1U << 22)); }while(0)
#define I2C3_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 23)); (RCC->APB1RSTR &= ~(1U << 23)); }while(0)


/*
 * Macros to reset CANx peripherals
 * WARNING: CAN2 is a SLAVE of CAN1 on this device. CAN1 owns the shared
 *          filter bank block, so CAN1's clock must be enabled before CAN2
 *          can be used at all, even if CAN1 itself is never used.
 */
#define CAN1_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 25)); (RCC->APB1RSTR &= ~(1U << 25)); }while(0)
#define CAN2_REG_RESET()    do{ (RCC->APB1RSTR |= (1U << 26)); (RCC->APB1RSTR &= ~(1U << 26)); }while(0)


/*
 * Returns the port code (0..7) for a given GPIO base address.
 * Used to program SYSCFG_EXTICR during interrupt configuration.
 */
#define GPIO_BASEADDR_TO_CODE(x)   ( (x == GPIOA) ? 0U :\
                                     (x == GPIOB) ? 1U :\
                                     (x == GPIOC) ? 2U :\
                                     (x == GPIOD) ? 3U :\
                                     (x == GPIOE) ? 4U :\
                                     (x == GPIOF) ? 5U :\
                                     (x == GPIOG) ? 6U :\
                                     (x == GPIOH) ? 7U : 0U )


/*
 * ARM Cortex-M4 Processor NVIC ISERx register addresses (Interrupt Set-Enable)
 */
#define NVIC_ISER0          ((volatile uint32_t*)0xE000E100U)
#define NVIC_ISER1          ((volatile uint32_t*)0xE000E104U)
#define NVIC_ISER2          ((volatile uint32_t*)0xE000E108U)
#define NVIC_ISER3          ((volatile uint32_t*)0xE000E10CU)


/*
 * ARM Cortex-M4 Processor NVIC ICERx register addresses (Interrupt Clear-Enable)
 */
#define NVIC_ICER0          ((volatile uint32_t*)0xE000E180U)
#define NVIC_ICER1          ((volatile uint32_t*)0xE000E184U)
#define NVIC_ICER2          ((volatile uint32_t*)0xE000E188U)
#define NVIC_ICER3          ((volatile uint32_t*)0xE000E18CU)


/*
 * ARM Cortex-M4 Processor NVIC Priority register base address
 */
#define NVIC_PR_BASE_ADDR   ((volatile uint32_t*)0xE000E400U)


/*
 * Number of priority bits implemented in the NVIC_IPR register (STM32F4 : 4)
 */
#define NO_PR_BITS_IMPLEMENTED   4U


/*
 * IRQ Numbers for EXTI - STM32F446xx
 */
#define IRQ_NO_EXTI0            6U   // EXTI line 0 interrupt
#define IRQ_NO_EXTI1            7U   // EXTI line 1 interrupt
#define IRQ_NO_EXTI2            8U   // EXTI line 2 interrupt
#define IRQ_NO_EXTI3            9U   // EXTI line 3 interrupt
#define IRQ_NO_EXTI4            10U  // EXTI line 4 interrupt

#define IRQ_NO_EXTI9_5          23U  // EXTI lines 5-9 interrupt
#define IRQ_NO_EXTI15_10        40U  // EXTI lines 10-15 interrupt


/*
 * IRQ Numbers for USART / UART - STM32F446xx
 * Each USART has its own vector, unlike EXTI where lines are shared.
 */
#define IRQ_NO_USART1           37U  // USART1 global interrupt
#define IRQ_NO_USART2           38U  // USART2 global interrupt
#define IRQ_NO_USART3           39U  // USART3 global interrupt
#define IRQ_NO_UART4            52U  // UART4  global interrupt
#define IRQ_NO_UART5            53U  // UART5  global interrupt
#define IRQ_NO_USART6           71U  // USART6 global interrupt


/*
 * IRQ Number for ADC - STM32F446xx
 * NOTE: ADC1, ADC2 and ADC3 SHARE this single vector. The handler must read
 *       the common CSR, or each ADC's SR, to work out which one fired.
 */
#define IRQ_NO_ADC              18U  // ADC1, ADC2 and ADC3 global interrupt


/*
 * IRQ Numbers for SPI - STM32F446xx
 * Each SPI has its own vector.
 */
#define IRQ_NO_SPI1             35U  // SPI1 global interrupt
#define IRQ_NO_SPI2             36U  // SPI2 global interrupt
#define IRQ_NO_SPI3             51U  // SPI3 global interrupt
#define IRQ_NO_SPI4             84U  // SPI4 global interrupt


/*
 * IRQ Numbers for I2C - STM32F446xx
 * NOTE: every I2C has TWO vectors. Events (start, address, byte transfer)
 *       and errors (bus error, arbitration lost, NACK) are reported
 *       separately, so a working driver must enable BOTH.
 */
#define IRQ_NO_I2C1_EV          31U  // I2C1 event interrupt
#define IRQ_NO_I2C1_ER          32U  // I2C1 error interrupt
#define IRQ_NO_I2C2_EV          33U  // I2C2 event interrupt
#define IRQ_NO_I2C2_ER          34U  // I2C2 error interrupt
#define IRQ_NO_I2C3_EV          72U  // I2C3 event interrupt
#define IRQ_NO_I2C3_ER          73U  // I2C3 error interrupt


/*
 * IRQ Numbers for CAN - STM32F446xx
 * Each controller has FOUR vectors: transmit, two receive FIFOs, and
 * status change or error.
 */
#define IRQ_NO_CAN1_TX          19U  // CAN1 Tx mailbox empty
#define IRQ_NO_CAN1_RX0         20U  // CAN1 Rx FIFO 0
#define IRQ_NO_CAN1_RX1         21U  // CAN1 Rx FIFO 1
#define IRQ_NO_CAN1_SCE         22U  // CAN1 status change and error
#define IRQ_NO_CAN2_TX          63U  // CAN2 Tx mailbox empty
#define IRQ_NO_CAN2_RX0         64U  // CAN2 Rx FIFO 0
#define IRQ_NO_CAN2_RX1         65U  // CAN2 Rx FIFO 1
#define IRQ_NO_CAN2_SCE         66U  // CAN2 status change and error

/*
 * Macros for all the possible NVIC priority levels
 * STM32F4 implements only the upper 4 bits of each priority byte,
 * so 16 distinct levels exist. 0 is the most urgent.
 */
#define NVIC_IRQ_PRI0    0U
#define NVIC_IRQ_PRI15   15U


/******************************************************************************
 *        Bit position definitions of USART peripheral registers
 ******************************************************************************/

/*
 * Bit position definitions : USART_SR (Status Register)
 */
#define USART_SR_PE             0U   // Parity error
#define USART_SR_FE             1U   // Framing error
#define USART_SR_NE             2U   // Noise detected flag
#define USART_SR_ORE            3U   // Overrun error
#define USART_SR_IDLE           4U   // Idle line detected
#define USART_SR_RXNE           5U   // Read data register not empty
#define USART_SR_TC             6U   // Transmission complete
#define USART_SR_TXE            7U   // Transmit data register empty
#define USART_SR_LBD            8U   // LIN break detection flag
#define USART_SR_CTS            9U   // CTS flag


/*
 * Bit position definitions : USART_DR (Data Register)
 * Bits [8:0] hold the data. Bits [31:9] are reserved.
 */
#define USART_DR_DR             0U   // Data value


/*
 * Bit position definitions : USART_BRR (Baud Rate Register)
 */
#define USART_BRR_DIV_FRACTION  0U   // Bits [3:0]  : fraction of USARTDIV
#define USART_BRR_DIV_MANTISSA  4U   // Bits [15:4] : mantissa of USARTDIV


/*
 * Bit position definitions : USART_CR1 (Control Register 1)
 */
#define USART_CR1_SBK           0U   // Send break
#define USART_CR1_RWU           1U   // Receiver wakeup
#define USART_CR1_RE            2U   // Receiver enable
#define USART_CR1_TE            3U   // Transmitter enable
#define USART_CR1_IDLEIE        4U   // IDLE interrupt enable
#define USART_CR1_RXNEIE        5U   // RXNE interrupt enable
#define USART_CR1_TCIE          6U   // Transmission complete interrupt enable
#define USART_CR1_TXEIE         7U   // TXE interrupt enable
#define USART_CR1_PEIE          8U   // PE interrupt enable
#define USART_CR1_PS            9U   // Parity selection : 0 = even, 1 = odd
#define USART_CR1_PCE           10U  // Parity control enable
#define USART_CR1_WAKE          11U  // Wakeup method
#define USART_CR1_M             12U  // Word length : 0 = 8 data bits, 1 = 9 data bits
#define USART_CR1_UE            13U  // USART enable
#define USART_CR1_OVER8         15U  // Oversampling mode : 0 = by 16, 1 = by 8


/*
 * Bit position definitions : USART_CR2 (Control Register 2)
 */
#define USART_CR2_ADD           0U   // Bits [3:0] : address of the USART node
#define USART_CR2_LBDL          5U   // LIN break detection length
#define USART_CR2_LBDIE         6U   // LIN break detection interrupt enable
#define USART_CR2_LBCL          8U   // Last bit clock pulse
#define USART_CR2_CPHA          9U   // Clock phase
#define USART_CR2_CPOL          10U  // Clock polarity
#define USART_CR2_CLKEN         11U  // Clock enable - the S in USART
#define USART_CR2_STOP          12U  // Bits [13:12] : number of stop bits
#define USART_CR2_LINEN         14U  // LIN mode enable


/*
 * Bit position definitions : USART_CR3 (Control Register 3)
 */
#define USART_CR3_EIE           0U   // Error interrupt enable
#define USART_CR3_IREN          1U   // IrDA mode enable
#define USART_CR3_IRLP          2U   // IrDA low power
#define USART_CR3_HDSEL         3U   // Half-duplex selection
#define USART_CR3_NACK          4U   // Smartcard NACK enable
#define USART_CR3_SCEN          5U   // Smartcard mode enable
#define USART_CR3_DMAR          6U   // DMA enable receiver
#define USART_CR3_DMAT          7U   // DMA enable transmitter
#define USART_CR3_RTSE          8U   // RTS enable
#define USART_CR3_CTSE          9U   // CTS enable
#define USART_CR3_CTSIE         10U  // CTS interrupt enable
#define USART_CR3_ONEBIT        11U  // One sample bit method enable


/*
 * Bit position definitions : USART_GTPR (Guard time and Prescaler Register)
 */
#define USART_GTPR_PSC          0U   // Bits [7:0]  : prescaler value
#define USART_GTPR_GT           8U   // Bits [15:8] : guard time value



/******************************************************************************
 *        Bit position definitions of ADC peripheral registers
 ******************************************************************************/

/*
 * Bit position definitions : ADC_SR (Status Register)
 */
#define ADC_SR_AWD              0U   // Analog watchdog flag
#define ADC_SR_EOC              1U   // Regular channel end of conversion
#define ADC_SR_JEOC             2U   // Injected channel end of conversion
#define ADC_SR_JSTRT            3U   // Injected channel start flag
#define ADC_SR_STRT             4U   // Regular channel start flag
#define ADC_SR_OVR              5U   // Overrun, a result was lost


/*
 * Bit position definitions : ADC_CR1 (Control Register 1)
 */
#define ADC_CR1_AWDCH           0U   // Bits [4:0]   : analog watchdog channel select
#define ADC_CR1_EOCIE           5U   // End of conversion interrupt enable
#define ADC_CR1_AWDIE           6U   // Analog watchdog interrupt enable
#define ADC_CR1_JEOCIE          7U   // Injected end of conversion interrupt enable
#define ADC_CR1_SCAN            8U   // Scan mode, convert the whole sequence
#define ADC_CR1_AWDSGL          9U   // Watchdog on a single channel
#define ADC_CR1_JAUTO           10U  // Automatic injected group conversion
#define ADC_CR1_DISCEN          11U  // Discontinuous mode on regular channels
#define ADC_CR1_JDISCEN         12U  // Discontinuous mode on injected channels
#define ADC_CR1_DISCNUM         13U  // Bits [15:13] : discontinuous mode channel count
#define ADC_CR1_JAWDEN          22U  // Watchdog enable on injected channels
#define ADC_CR1_AWDEN           23U  // Watchdog enable on regular channels
#define ADC_CR1_RES             24U  // Bits [25:24] : resolution, 00=12b 01=10b 10=8b 11=6b
#define ADC_CR1_OVRIE           26U  // Overrun interrupt enable


/*
 * Bit position definitions : ADC_CR2 (Control Register 2)
 */
#define ADC_CR2_ADON            0U   // A/D converter ON, needs a settling delay
#define ADC_CR2_CONT            1U   // Continuous conversion
#define ADC_CR2_DMA             8U   // Direct memory access mode
#define ADC_CR2_DDS             9U   // DMA disable selection for continuous requests
#define ADC_CR2_EOCS            10U  // End of conversion selection
#define ADC_CR2_ALIGN           11U  // Data alignment, 0 = right, 1 = left
#define ADC_CR2_JEXTSEL         16U  // Bits [19:16] : injected external trigger select
#define ADC_CR2_JEXTEN          20U  // Bits [21:20] : injected trigger edge
#define ADC_CR2_JSWSTART        22U  // Start conversion of injected channels
#define ADC_CR2_EXTSEL          24U  // Bits [27:24] : regular external trigger select
#define ADC_CR2_EXTEN           28U  // Bits [29:28] : regular trigger edge
#define ADC_CR2_SWSTART         30U  // Start conversion of regular channels


/*
 * Bit position definitions : ADC_SMPR1 and ADC_SMPR2 (Sample Time Registers)
 * Each channel owns a 3-bit field. SMPR2 covers channels 0 to 9,
 * SMPR1 covers channels 10 to 18.
 */
#define ADC_SMPR_FIELD_WIDTH    3U   // Bits per channel


/*
 * Bit position definitions : ADC_SQR1 / SQR2 / SQR3 (Regular Sequence Registers)
 * NOTE: SQ1, the FIRST conversion, lives in SQR3 - the numbering runs
 *       backwards from what most students expect.
 */
#define ADC_SQR1_L              20U  // Bits [23:20] : sequence length MINUS ONE
#define ADC_SQR3_SQ1            0U   // Bits [4:0]   : first conversion channel
#define ADC_SQR_FIELD_WIDTH     5U   // Bits per sequence entry


/*
 * Bit position definitions : ADC_CCR (Common Control Register)
 */
#define ADC_CCR_MULTI           0U   // Bits [4:0]   : multi ADC mode selection
#define ADC_CCR_DELAY           8U   // Bits [11:8]  : delay between two sampling phases
#define ADC_CCR_DDS             13U  // DMA disable selection for multi ADC mode
#define ADC_CCR_DMA             14U  // Bits [15:14] : direct memory access mode for multi ADC
#define ADC_CCR_ADCPRE          16U  // Bits [17:16] : prescaler, 00=/2 01=/4 10=/6 11=/8
#define ADC_CCR_VBATE           22U  // VBAT channel enable
#define ADC_CCR_TSVREFE         23U  // Temperature sensor and VREFINT channel enable



/******************************************************************************
 *        Bit position definitions of SPI peripheral registers
 ******************************************************************************/

/*
 * Bit position definitions : SPI_CR1 (Control Register 1)
 */
#define SPI_CR1_CPHA            0U   // Clock phase : which edge samples the data
#define SPI_CR1_CPOL            1U   // Clock polarity : idle level of SCLK
#define SPI_CR1_MSTR            2U   // Master selection : 1 = master, 0 = slave
#define SPI_CR1_BR              3U   // Bits [5:3] : baud rate divider, minimum is /2
#define SPI_CR1_SPE             6U   // SPI peripheral enable
#define SPI_CR1_LSBFIRST        7U   // Frame format : 0 = MSB first, 1 = LSB first
#define SPI_CR1_SSI             8U   // Internal slave select
#define SPI_CR1_SSM             9U   // Software slave management
#define SPI_CR1_RXONLY          10U  // Receive only mode
#define SPI_CR1_DFF             11U  // Data frame format : 0 = 8-bit, 1 = 16-bit
#define SPI_CR1_CRCNEXT         12U  // Transmit CRC next
#define SPI_CR1_CRCEN           13U  // Hardware CRC calculation enable
#define SPI_CR1_BIDIOE          14U  // Output enable in bidirectional mode
#define SPI_CR1_BIDIMODE        15U  // Bidirectional data mode enable


/*
 * Bit position definitions : SPI_CR2 (Control Register 2)
 */
#define SPI_CR2_RXDMAEN         0U   // Rx buffer DMA enable
#define SPI_CR2_TXDMAEN         1U   // Tx buffer DMA enable
#define SPI_CR2_SSOE            2U   // SS output enable, for hardware NSS
#define SPI_CR2_FRF             4U   // Frame format : 0 = Motorola, 1 = TI
#define SPI_CR2_ERRIE           5U   // Error interrupt enable
#define SPI_CR2_RXNEIE          6U   // Rx buffer not empty interrupt enable
#define SPI_CR2_TXEIE           7U   // Tx buffer empty interrupt enable


/*
 * Bit position definitions : SPI_SR (Status Register)
 */
#define SPI_SR_RXNE             0U   // Receive buffer not empty
#define SPI_SR_TXE              1U   // Transmit buffer empty
#define SPI_SR_CHSIDE           2U   // Channel side, I2S mode only
#define SPI_SR_UDR              3U   // Underrun flag, slave mode only
#define SPI_SR_CRCERR           4U   // CRC error flag
#define SPI_SR_MODF             5U   // Mode fault, another master drove NSS low
#define SPI_SR_OVR              6U   // Overrun flag
#define SPI_SR_BSY              7U   // Busy flag, the shift register is still working
#define SPI_SR_FRE              8U   // Frame format error


/******************************************************************************
 *        Bit position definitions of I2C peripheral registers
 ******************************************************************************/

/*
 * Bit position definitions : I2C_CR1 (Control Register 1)
 */
#define I2C_CR1_PE              0U   // Peripheral enable
#define I2C_CR1_SMBUS           1U   // SMBus mode
#define I2C_CR1_SMBTYPE         3U   // SMBus type
#define I2C_CR1_ENARP           4U   // ARP enable
#define I2C_CR1_ENPEC           5U   // PEC enable
#define I2C_CR1_ENGC            6U   // General call enable
#define I2C_CR1_NOSTRETCH       7U   // Clock stretching disable, slave mode
#define I2C_CR1_START           8U   // Generate a START condition
#define I2C_CR1_STOP            9U   // Generate a STOP condition
#define I2C_CR1_ACK             10U  // Acknowledge enable
#define I2C_CR1_POS             11U  // Acknowledge and PEC position
#define I2C_CR1_PEC             12U  // Packet error checking
#define I2C_CR1_ALERT           13U  // SMBus alert
#define I2C_CR1_SWRST           15U  // Software reset


/*
 * Bit position definitions : I2C_CR2 (Control Register 2)
 */
#define I2C_CR2_FREQ            0U   // Bits [5:0] : APB1 frequency in MHz, NOT a divider
#define I2C_CR2_ITERREN         8U   // Error interrupt enable
#define I2C_CR2_ITEVTEN         9U   // Event interrupt enable
#define I2C_CR2_ITBUFEN         10U  // Buffer interrupt enable
#define I2C_CR2_DMAEN           11U  // DMA requests enable
#define I2C_CR2_LAST            12U  // DMA last transfer


/*
 * Bit position definitions : I2C_OAR1 (Own Address Register 1)
 */
#define I2C_OAR1_ADD0           0U   // Bit 0 of a 10-bit address
#define I2C_OAR1_ADD71          1U   // Bits [7:1] : 7-bit own address
#define I2C_OAR1_ADD98          8U   // Bits [9:8] : high bits of a 10-bit address
#define I2C_OAR1_ADDMODE        15U  // Addressing mode : 0 = 7-bit, 1 = 10-bit


/*
 * Bit position definitions : I2C_SR1 (Status Register 1)
 */
#define I2C_SR1_SB              0U   // Start bit generated, master mode
#define I2C_SR1_ADDR            1U   // Address sent or matched
#define I2C_SR1_BTF             2U   // Byte transfer finished
#define I2C_SR1_ADD10           3U   // 10-bit header sent
#define I2C_SR1_STOPF           4U   // Stop detected, slave mode
#define I2C_SR1_RXNE            6U   // Data register not empty, receiver
#define I2C_SR1_TXE             7U   // Data register empty, transmitter
#define I2C_SR1_BERR            8U   // Bus error
#define I2C_SR1_ARLO            9U   // Arbitration lost
#define I2C_SR1_AF              10U  // Acknowledge failure, the slave did not respond
#define I2C_SR1_OVR             11U  // Overrun or underrun
#define I2C_SR1_PECERR          12U  // PEC error
#define I2C_SR1_TIMEOUT         14U  // Timeout or Tlow error
#define I2C_SR1_SMBALERT        15U  // SMBus alert


/*
 * Bit position definitions : I2C_SR2 (Status Register 2)
 * NOTE: reading SR1 then SR2 is the sequence that clears the ADDR flag.
 */
#define I2C_SR2_MSL             0U   // Master or slave
#define I2C_SR2_BUSY            1U   // Bus busy
#define I2C_SR2_TRA             2U   // Transmitter or receiver
#define I2C_SR2_GENCALL         4U   // General call address received
#define I2C_SR2_SMBDEFAULT      5U   // SMBus device default address
#define I2C_SR2_SMBHOST         6U   // SMBus host header
#define I2C_SR2_DUALF           7U   // Dual flag
#define I2C_SR2_PEC             8U   // Bits [15:8] : packet error checking register


/*
 * Bit position definitions : I2C_CCR (Clock Control Register)
 */
#define I2C_CCR_CCR             0U   // Bits [11:0] : clock control divider
#define I2C_CCR_DUTY            14U  // Fast mode duty cycle
#define I2C_CCR_FS              15U  // Master mode selection : 0 = standard, 1 = fast


/******************************************************************************
 *        Bit position definitions of CAN peripheral registers
 ******************************************************************************/

/*
 * Bit position definitions : CAN_MCR (Master Control Register)
 */
#define CAN_MCR_INRQ            0U   // Initialisation request
#define CAN_MCR_SLEEP           1U   // Sleep mode request, SET after reset
#define CAN_MCR_TXFP            2U   // Transmit FIFO priority
#define CAN_MCR_RFLM            3U   // Receive FIFO locked mode
#define CAN_MCR_NART            4U   // No automatic retransmission
#define CAN_MCR_AWUM            5U   // Automatic wakeup mode
#define CAN_MCR_ABOM            6U   // Automatic bus-off management
#define CAN_MCR_TTCM            7U   // Time triggered communication mode
#define CAN_MCR_RESET           15U  // bxCAN software master reset
#define CAN_MCR_DBF             16U  // Debug freeze


/*
 * Bit position definitions : CAN_MSR (Master Status Register)
 */
#define CAN_MSR_INAK            0U   // Initialisation acknowledge
#define CAN_MSR_SLAK            1U   // Sleep acknowledge
#define CAN_MSR_ERRI            2U   // Error interrupt
#define CAN_MSR_WKUI            3U   // Wakeup interrupt
#define CAN_MSR_SLAKI           4U   // Sleep acknowledge interrupt
#define CAN_MSR_TXM             8U   // Transmit mode
#define CAN_MSR_RXM             9U   // Receive mode


/*
 * Bit position definitions : CAN_TSR (Transmit Status Register)
 */
#define CAN_TSR_RQCP0           0U   // Request completed, mailbox 0
#define CAN_TSR_TXOK0           1U   // Transmission OK, mailbox 0
#define CAN_TSR_ABRQ0           7U   // Abort request, mailbox 0
#define CAN_TSR_RQCP1           8U   // Request completed, mailbox 1
#define CAN_TSR_TXOK1           9U   // Transmission OK, mailbox 1
#define CAN_TSR_ABRQ1           15U  // Abort request, mailbox 1
#define CAN_TSR_RQCP2           16U  // Request completed, mailbox 2
#define CAN_TSR_TXOK2           17U  // Transmission OK, mailbox 2
#define CAN_TSR_ABRQ2           23U  // Abort request, mailbox 2
#define CAN_TSR_CODE            24U  // Bits [25:24] : next empty mailbox number
#define CAN_TSR_TME0            26U  // Transmit mailbox 0 empty
#define CAN_TSR_TME1            27U  // Transmit mailbox 1 empty
#define CAN_TSR_TME2            28U  // Transmit mailbox 2 empty


/*
 * Bit position definitions : CAN_RF0R and CAN_RF1R (Receive FIFO registers)
 */
#define CAN_RFR_FMP             0U   // Bits [1:0] : messages pending in the FIFO
#define CAN_RFR_FULL            3U   // FIFO full
#define CAN_RFR_FOVR            4U   // FIFO overrun
#define CAN_RFR_RFOM            5U   // Release FIFO output mailbox


/*
 * Bit position definitions : CAN_IER (Interrupt Enable Register)
 */
#define CAN_IER_TMEIE           0U   // Transmit mailbox empty interrupt enable
#define CAN_IER_FMPIE0          1U   // FIFO 0 message pending interrupt enable
#define CAN_IER_FFIE0           2U   // FIFO 0 full interrupt enable
#define CAN_IER_FOVIE0          3U   // FIFO 0 overrun interrupt enable
#define CAN_IER_FMPIE1          4U   // FIFO 1 message pending interrupt enable
#define CAN_IER_FFIE1           5U   // FIFO 1 full interrupt enable
#define CAN_IER_FOVIE1          6U   // FIFO 1 overrun interrupt enable
#define CAN_IER_EWGIE           8U   // Error warning interrupt enable
#define CAN_IER_EPVIE           9U   // Error passive interrupt enable
#define CAN_IER_BOFIE           10U  // Bus-off interrupt enable
#define CAN_IER_LECIE           11U  // Last error code interrupt enable
#define CAN_IER_ERRIE           15U  // Error interrupt enable
#define CAN_IER_WKUIE           16U  // Wakeup interrupt enable
#define CAN_IER_SLKIE           17U  // Sleep interrupt enable


/*
 * Bit position definitions : CAN_BTR (Bit Timing Register)
 * Bit rate = PCLK1 / (BRP + 1) / (1 + TS1 + 1 + TS2 + 1)
 */
#define CAN_BTR_BRP             0U   // Bits [9:0]   : baud rate prescaler
#define CAN_BTR_TS1             16U  // Bits [19:16] : time segment 1
#define CAN_BTR_TS2             20U  // Bits [22:20] : time segment 2
#define CAN_BTR_SJW             24U  // Bits [25:24] : resynchronisation jump width
#define CAN_BTR_LBKM            30U  // Loopback mode, useful for self test
#define CAN_BTR_SILM            31U  // Silent mode, listen only


/*
 * Bit position definitions : CAN Tx mailbox identifier register (TIR)
 */
#define CAN_TIR_TXRQ            0U   // Transmit mailbox request
#define CAN_TIR_RTR             1U   // Remote transmission request
#define CAN_TIR_IDE             2U   // Identifier extension : 0 = standard, 1 = extended
#define CAN_TIR_EXID            3U   // Bits [20:3]  : extended identifier
#define CAN_TIR_STID            21U  // Bits [31:21] : standard identifier


/*
 * Bit position definitions : CAN Rx FIFO identifier register (RIR)
 */
#define CAN_RIR_RTR             1U   // Remote transmission request
#define CAN_RIR_IDE             2U   // Identifier extension
#define CAN_RIR_EXID            3U   // Bits [20:3]  : extended identifier
#define CAN_RIR_STID            21U  // Bits [31:21] : standard identifier


/*
 * Bit position definitions : CAN_FMR (Filter Master Register)
 */
#define CAN_FMR_FINIT           0U   // Filter initialisation mode
#define CAN_FMR_CAN2SB          8U   // Bits [13:8] : CAN2 start filter bank


//------------ MAcros Definition End -------------------------------


#include "stm32f446xx_rcc_driver.h"  //clock services shared by USART, I2C, SPI and USB
#include "stm32f446xx_adc_driver.h"  //ADC driver, uses the RCC clock services above
#include "stm32f446xx_spi_driver.h"  //SPI driver
#include "stm32f446xx_i2c_driver.h"  //I2C driver
#include "stm32f446xx_can_driver.h"  //CAN driver
#include "stm32f446xx_gpio_driver.h" //this help in apps main.c by just including stm32f446.h
#include "stm32f446xx_usart_driver.h" //same idea for the USART driver

#endif /* INC_STM32F446XX_H_ */
