#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include "stm32f446xx_rcc_driver.h"

#define LED_PIN   GPIO_PIN_NO_5
#define BTN_PIN   GPIO_PIN_NO_13    /* PC13, blue user button, low when pressed */

/* SysTick registers, used only for an accurate millisecond delay */
#define SYST_CSR  (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR  (*(volatile uint32_t *)0xE000E014U)
#define SYST_CVR  (*(volatile uint32_t *)0xE000E018U)

SPI_Handle_t spi;                   /* global, the ISR needs it */
static volatile uint8_t count = 0U;
static volatile uint8_t button_flag = 0U;
static uint8_t spi_tx_byte;
static uint8_t spi_rx_byte;

/* exact delay, based on the real HCLK from the RCC driver */
static void delay_ms(uint32_t ms)
{
    SYST_RVR = (RCC_GetHCLKValue() / 1000U) - 1U;
    SYST_CVR = 0U;
    SYST_CSR = 5U;                              /* enable, core clock, no interrupt */

    while (ms--)
    {
        while ((SYST_CSR & (1U << 16)) == 0U);  /* wait 1 ms */
    }

    SYST_CSR = 0U;
}

/* configure one GPIO pin in a single call */
static void pin_cfg(GPIO_RegDef_t *port, uint8_t num, uint8_t mode, uint8_t pupd, uint8_t af)
{
    GPIO_Handle_t h;

    h.pGPIOx = port;
    h.GPIO_PinConfig.GPIO_PinNumber      = num;
    h.GPIO_PinConfig.GPIO_PinMode        = mode;
    h.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_HIGH;
    h.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    h.GPIO_PinConfig.GPIO_PinPuPdControl = pupd;
    h.GPIO_PinConfig.GPIO_PinAltFunMode  = af;
    GPIO_Init(&h);
}

/* button interrupt: just clear it and raise a flag */
void EXTI15_10_IRQHandler(void)
{
    GPIO_IRQHandling(BTN_PIN);
    button_flag = 1U;
}

/* SPI2 interrupt, hands over to the driver */
void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&spi);
}

/* the driver calls this from the ISR */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEvent)
{
    if (AppEvent == SPI_EVENT_RX_CMPLT)
    {
        /* master finished a transfer: load the newest count for the next one */
        spi_tx_byte = count;
        SPI_ReceiveDataIT(pSPIHandle, &spi_rx_byte, 1U);
        SPI_SendDataIT(pSPIHandle, &spi_tx_byte, 1U);
    }
}

int main(void)
{
    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_PeriClockControl(GPIOB, ENABLE);
    GPIO_PeriClockControl(GPIOC, ENABLE);

    pin_cfg(GPIOA, LED_PIN,         GPIO_MODE_OUT,   GPIO_NO_PUPD, 0U);        /* LED */
    pin_cfg(GPIOB, GPIO_PIN_NO_12,  GPIO_MODE_ALTFN, GPIO_PIN_PU,  GPIO_AF5);  /* NSS, from master CS */
    pin_cfg(GPIOB, GPIO_PIN_NO_13,  GPIO_MODE_ALTFN, GPIO_PIN_PD,  GPIO_AF5);  /* SCK */
    pin_cfg(GPIOB, GPIO_PIN_NO_14,  GPIO_MODE_ALTFN, GPIO_NO_PUPD, GPIO_AF5);  /* MISO */
    pin_cfg(GPIOB, GPIO_PIN_NO_15,  GPIO_MODE_ALTFN, GPIO_NO_PUPD, GPIO_AF5);  /* MOSI */

    /* user button PC13, interrupt on falling edge (press) */
    pin_cfg(GPIOC, BTN_PIN, GPIO_MODE_IT_FT, GPIO_NO_PUPD, 0U);
    GPIO_IRQConfig(IRQ_NO_EXTI15_10, 5U, ENABLE);

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

    SPI_IRQConfig(IRQ_NO_SPI2, 1U, ENABLE);
    SPI_PeripheralControl(SPI2, ENABLE);         /* enable only after init */

    /* get the first reply ready before the master asks */
    spi_tx_byte = count;
    SPI_ReceiveDataIT(&spi, &spi_rx_byte, 1U);
    SPI_SendDataIT(&spi, &spi_tx_byte, 1U);

    while (1)
    {
        if (button_flag != 0U)
        {
            delay_ms(50);                        /* wait out the contact bounce */
            button_flag = 0U;

            if (GPIO_ReadFromInputPin(GPIOC, BTN_PIN) == 0U)   /* still pressed */
            {
                count++;
                GPIO_ToggleOutputPin(GPIOA, LED_PIN);          /* LED shows a press was counted */
            }
        }
    }
}
