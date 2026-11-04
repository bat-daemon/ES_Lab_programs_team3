# EMBEDDED_03 - Embedded Systems Lab (Team 3)

Lab programs for the Embedded Systems course, written in C for the
**STM32 NUCLEO-F446RE** (STM32F446RE, Cortex-M4). All programs use the
instructor-provided bare-metal peripheral drivers instead of HAL.

## Repository structure

Each lab has its own folder, and inside it every team member has a folder
with their own submission.

```
EMBEDDED_03/
└── Team_3/
    ├── ADC_ADCINT/
    ├── CAN/
    ├── GPIO_GPIOINT/
    ├── I2C/
    ├── SORT_SEARCH_SERIES/
    ├── SPI_SPIINT/
    └── UART_UARTINT/
        ├── member_1/
        ├── member_2/
        └── member_3/
```

The same three member folders exist inside every lab folder.

## Labs

| Folder | Topic |
|--------|-------|
| `GPIO_GPIOINT` | GPIO output and input, button interrupt (EXTI) |
| `UART_UARTINT` | USART communication, polling and interrupt based |
| `ADC_ADCINT` | ADC conversion, polling and interrupt based |
| `SPI_SPIINT` | SPI master and slave communication, interrupt based |
| `I2C` | I2C communication |
| `CAN` | CAN communication (loopback, no transceiver needed) |
| `SORT_SEARCH_SERIES` | Sorting, searching and series programs |

## Drivers used

The drivers are shared across the labs. Each peripheral has a header and a
source file:

| Driver | Files |
|--------|-------|
| Device header (registers, base addresses, macros) | `stm32f446xx.h` |
| GPIO | `stm32f446xx_gpio_driver.h/.c` |
| RCC (clock services) | `stm32f446xx_rcc_driver.h/.c` |
| USART | `stm32f446xx_usart_driver.h/.c` |
| ADC | `stm32f446xx_adc_driver.h/.c` |
| SPI | `stm32f446xx_spi_driver.h/.c` |
| I2C | `stm32f446xx_i2c_driver.h/.c` |
| CAN | `stm32f446xx_can_driver.h/.c` |

`stm32f446xx.h` includes all the driver headers, so an application only
needs to include that one file first.

## Hardware

- STM32 NUCLEO-F446RE (one board, or two for the SPI master and slave demo)
- Default clock: 16 MHz HSI, no PLL configured
- UART output goes through the ST-LINK virtual COM port (USART2, PA2),
  115200 baud, 8N1
- Jumper wires and a common ground for two-board labs

## How to build and run

1. Create a project for the STM32F446RE in your IDE.
2. Copy the driver `.h` files into `Inc/` and the `.c` files into `Src/`.
3. Copy the lab's `main.c` from the required member folder into `Src/`.
4. Build, flash, and open a serial terminal at 115200 baud where the lab
   prints over UART.

For the two-board SPI lab, flash the slave first, then the master, and press
RESET on both if only one is restarted later.

## Notes

- Include `stm32f446xx.h` before any driver header.
- Each peripheral driver enables its own clock inside its `Init` function.
- Always enable the peripheral (`SPE`, `UE`, `ADON`, `PE`) after `Init`.