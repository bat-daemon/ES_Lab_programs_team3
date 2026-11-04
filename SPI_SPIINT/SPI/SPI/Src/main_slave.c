/*
 * main_slave.c
 *
 *  Created on: Oct 30, 2026
 *      Author: swaro
 */
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"

#define LED_PIN   GPIO_PIN_NO_5
#define REPLY     0xA5U             /* byte sent back to the master */

/* LED solid ON = unexpected byte from master */
static void error_halt(void)
{
    GPIO_WriteToOutputPin(GPIOA, LED_PIN, GPIO_PIN_SET);
    while (1);
}

int main(void)
{
    GPIO_Handle_t pin;
    SPI_Handle_t spi;
    uint8_t reply = REPLY;
    uint8_t expected = 0x00U;
    uint8_t got;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_PeriClockControl(GPIOB, ENABLE);

    /* LED on PA5 */
    pin.pGPIOx = GPIOA;
    pin.GPIO_PinConfig.GPIO_PinNumber      = LED_PIN;
    pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
    pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_LOW;
    pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode  = 0U;
    GPIO_Init(&pin);

    /* PB12 NSS, PB13 SCK, PB14 MISO, PB15 MOSI -> all AF5 */
    pin.pGPIOx = GPIOB;
    pin.GPIO_PinConfig.GPIO_PinMode       = GPIO_MODE_ALTFN;
    pin.GPIO_PinConfig.GPIO_PinSpeed      = GPIO_SPEED_HIGH;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode = GPIO_AF5;

    /* NSS pulled up so it reads "not selected" if the master is not connected */
    pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_12;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    GPIO_Init(&pin);

    /* SCK pulled down, idle level for mode 0 */
    pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_13;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
    GPIO_Init(&pin);

    pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_14;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GPIO_Init(&pin);

    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&pin);

    /* SPI2: slave, full duplex, mode 0, 8-bit, hardware NSS.
       The clock speed setting is ignored in slave mode, the master drives SCK. */
    spi.pSPIx = SPI2;
    spi.SPI_Config.SPI_DeviceMode = SPI_DEVICE_MODE_SLAVE;
    spi.SPI_Config.SPI_BusConfig  = SPI_BUS_CONFIG_FD;
    spi.SPI_Config.SPI_SclkSpeed  = SPI_SCLK_SPEED_DIV16;
    spi.SPI_Config.SPI_DFF        = SPI_DFF_8BITS;
    spi.SPI_Config.SPI_CPOL       = SPI_CPOL_LOW;
    spi.SPI_Config.SPI_CPHA       = SPI_CPHA_LOW;
    spi.SPI_Config.SPI_SSM        = SPI_SSM_DI;
    SPI_Init(&spi);

    SPI_PeripheralControl(SPI2, ENABLE);

    while (1)
    {
        expected += 0x11U;

        /* load the reply first, the master may clock at any time */
        SPI_SendData(SPI2, &reply, 1U);

        /* wait for the master's byte */
        SPI_ReceiveData(SPI2, &got, 1U);

        if (got == expected)
            GPIO_ToggleOutputPin(GPIOA, LED_PIN);   /* blink = master's byte was right */
        else
            error_halt();
    }
}

