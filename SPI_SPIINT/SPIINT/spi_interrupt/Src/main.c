#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_rcc_driver.h"

#define LED_PIN   GPIO_PIN_NO_5
#define CS_PIN    GPIO_PIN_NO_12
#define CMD_GET   0xAAU             /* any byte, the slave ignores it */

/* SysTick registers, used only for an accurate millisecond delay */
#define SYST_CSR  (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR  (*(volatile uint32_t *)0xE000E014U)
#define SYST_CVR  (*(volatile uint32_t *)0xE000E018U)

SPI_Handle_t spi;                   /* global, the ISR needs it */
static USART_Handle_t usart;
static volatile uint8_t spi_done = 0U;
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

static void uart_print(const char *s)
{
    uint32_t len = 0U;

    while (s[len] != '\0')
        len++;

    USART_SendData(&usart, (uint8_t *)s, len);
}

static void uart_print_num(uint32_t n)
{
    char buf[11];
    uint8_t i = 10U;

    buf[i] = '\0';
    do
    {
        i--;
        buf[i] = (char)('0' + (n % 10U));
        n /= 10U;
    } while (n > 0U);

    uart_print(&buf[i]);
}

/* one byte out, one byte in, interrupt driven */
static uint8_t spi_exchange(uint8_t out)
{
    spi_tx_byte = out;
    spi_rx_byte = 0U;
    spi_done    = 0U;

    GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_RESET);

    SPI_ReceiveDataIT(&spi, &spi_rx_byte, 1U);   /* arm receive first */
    SPI_SendDataIT(&spi, &spi_tx_byte, 1U);      /* sending starts the clock */

    while (spi_done == 0U);                      /* ISR does the work meanwhile */
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_BSY) == FLAG_SET);

    GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_SET);

    return spi_rx_byte;
}

/* SPI2 interrupt, hands over to the driver */
void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&spi);
}

/* the driver calls this from the ISR */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEvent)
{
    (void)pSPIHandle;

    if (AppEvent == SPI_EVENT_RX_CMPLT)
        spi_done = 1U;
}

int main(void)
{
    uint8_t count;
    uint32_t poll = 0U;

    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_PeriClockControl(GPIOB, ENABLE);

    pin_cfg(GPIOA, LED_PIN,         GPIO_MODE_OUT,   GPIO_NO_PUPD, 0U);        /* LED */
    pin_cfg(GPIOA, GPIO_PIN_NO_2,   GPIO_MODE_ALTFN, GPIO_PIN_PU,  GPIO_AF7);  /* USART2 TX */
    pin_cfg(GPIOB, CS_PIN,          GPIO_MODE_OUT,   GPIO_NO_PUPD, 0U);        /* chip select */
    pin_cfg(GPIOB, GPIO_PIN_NO_13,  GPIO_MODE_ALTFN, GPIO_NO_PUPD, GPIO_AF5);  /* SCK */
    pin_cfg(GPIOB, GPIO_PIN_NO_14,  GPIO_MODE_ALTFN, GPIO_PIN_PD,  GPIO_AF5);  /* MISO */
    pin_cfg(GPIOB, GPIO_PIN_NO_15,  GPIO_MODE_ALTFN, GPIO_NO_PUPD, GPIO_AF5);  /* MOSI */

    GPIO_WriteToOutputPin(GPIOB, CS_PIN, GPIO_PIN_SET);                        /* CS idle high */

    /* USART2: 115200, 8N1, transmit only */
    usart.pUSARTx = USART2;
    usart.USART_Config.USART_Mode          = USART_MODE_ONLY_TX;
    usart.USART_Config.USART_Baud          = USART_STD_BAUD_115200;
    usart.USART_Config.USART_NoOfStopBits  = USART_STOPBITS_1;
    usart.USART_Config.USART_WordLength    = USART_WORDLEN_8BITS;
    usart.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    usart.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    USART_Init(&usart);
    USART_PeripheralControl(USART2, ENABLE);

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

    SPI_IRQConfig(IRQ_NO_SPI2, 1U, ENABLE);
    SPI_PeripheralControl(SPI2, ENABLE);         /* enable only after init */

    delay_ms(100);

    uart_print("\r\n=== SPI master ready ===\r\n");
    delay_ms(500);

    uart_print("PCLK1 = ");
    uart_print_num(RCC_GetPCLK1Value());
    uart_print(" Hz\r\n");
    delay_ms(500);

    uart_print("Press the blue button on the slave board\r\n");
    delay_ms(1000);

    while (1)
    {
        spi_exchange(CMD_GET);                   /* 1st read: slave loads its latest count */
        delay_ms(20);
        count = spi_exchange(CMD_GET);           /* 2nd read: this is the fresh count */

        poll++;
        uart_print("Poll ");
        uart_print_num(poll);
        uart_print(" : button count = ");
        uart_print_num(count);
        uart_print("\r\n");

        GPIO_ToggleOutputPin(GPIOA, LED_PIN);    /* heartbeat */
        delay_ms(1000);                          /* one message per second */
    }
}
