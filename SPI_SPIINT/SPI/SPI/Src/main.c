//Master

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"

#define LED_PIN   GPIO_PIN_NO_5
#define CS_PIN    GPIO_PIN_NO_12    /* PB12, manual chip select */
#define REPLY     0xA5U             /* byte the slave must answer with */

static void delay(volatile uint32_t count)
{
    while (count--);
}

/* LED solid ON = wrong reply from slave */
static void error_halt(void)
{
    GPIO_WriteToOutputPin(GPIOA, LED_PIN, GPIO_PIN_SET);
    while (1);
}

int main(void)
{
    GPIO_Handle_t pin;
    SPI_Handle_t spi;
    uint8_t sent = 0x00U;
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

    /* PB12 chip select, idle high */
    pin.pGPIOx = GPIOB;
    pin.GPIO_PinConfig.GPIO_PinNumber = CS_PIN;
    pin.GPIO_PinConfig.GPIO_PinSpeed  = GPIO_SPEED_HIGH;
    GPIO_Init(&pin);
    GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_SET);

    /* PB13 SCK and PB15 MOSI -> AF5 */
    pin.GPIO_PinConfig.GPIO_PinMode       = GPIO_MODE_ALTFN;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode = GPIO_AF5;

    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&pin);

    pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&pin);

    /* PB14 MISO -> AF5, pull-down so a missing slave reads 0 and shows as an error */
    pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_14;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
    GPIO_Init(&pin);

    /* SPI2: master, full duplex, mode 0, 8-bit, 16 MHz / 16 = 1 MHz */
    spi.pSPIx = SPI2;
    spi.SPI_Config.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    spi.SPI_Config.SPI_BusConfig  = SPI_BUS_CONFIG_FD;
    spi.SPI_Config.SPI_SclkSpeed  = SPI_SCLK_SPEED_DIV16;
    spi.SPI_Config.SPI_DFF        = SPI_DFF_8BITS;
    spi.SPI_Config.SPI_CPOL       = SPI_CPOL_LOW;
    spi.SPI_Config.SPI_CPHA       = SPI_CPHA_LOW;
    spi.SPI_Config.SPI_SSM        = SPI_SSM_EN;
    SPI_Init(&spi);

    SPI_PeripheralControl(SPI2, ENABLE);

    delay(2000000U);                           /* give the slave time to get ready */

    while (1)
    {
        sent += 0x11U;                         /* 0x11, 0x22, 0x33 ... */

        GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_RESET);

        SPI_SendData(SPI2, &sent, 1U);
        SPI_ReceiveData(SPI2, &got, 1U);

        while (SPI_GetFlagStatus(SPI2, SPI_FLAG_BSY) == FLAG_SET);
        GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_SET);

        if (got == REPLY)
            GPIO_ToggleOutputPin(GPIOA, LED_PIN);   /* blink = slave answered correctly */
        else
            error_halt();

        delay(500000U);
    }
}
