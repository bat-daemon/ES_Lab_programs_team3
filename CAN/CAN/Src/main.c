#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_can_driver.h"

/* CAN1 pins: PA12 = TX, PA11 = RX (both AF9) */
#define CAN_TX_PIN   GPIO_PIN_NO_12
#define CAN_RX_PIN   GPIO_PIN_NO_11
#define LED_PIN      GPIO_PIN_NO_5

static void delay(volatile uint32_t count)
{
    while (count--);
}

/* LED solid ON = something failed */
static void error_halt(void)
{
    GPIO_WriteToOutputPin(GPIOA, LED_PIN, GPIO_PIN_SET);
    while (1);
}

int main(void)
{
    GPIO_Handle_t pin;
    CAN_Handle_t can;
    CAN_Message_t tx, rx;
    uint8_t mailbox;
    uint32_t timeout;

    GPIO_PeriClockControl(GPIOA, ENABLE);

    /* LED on PA5 */
    pin.pGPIOx = GPIOA;
    pin.GPIO_PinConfig.GPIO_PinNumber      = LED_PIN;
    pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
    pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_LOW;
    pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode  = 0U;
    GPIO_Init(&pin);

    /* CAN TX pin PA12 -> AF9 */
    pin.GPIO_PinConfig.GPIO_PinNumber      = CAN_TX_PIN;
    pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
    pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_HIGH;
    pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    pin.GPIO_PinConfig.GPIO_PinAltFunMode  = GPIO_AF9;
    GPIO_Init(&pin);

    /* CAN RX pin PA11 -> AF9 (ignored in loopback, set for completeness) */
    pin.GPIO_PinConfig.GPIO_PinNumber      = CAN_RX_PIN;
    GPIO_Init(&pin);

    /* Bit rate = 16 MHz / 2 / (1 + 13 + 2) = 500 kbit/s (default HSI clock) */
    can.pCANx = CAN1;
    can.CAN_Config.CAN_Prescaler = 2U;
    can.CAN_Config.CAN_Mode      = CAN_MODE_LOOPBACK;
    can.CAN_Config.CAN_SJW       = CAN_SJW_1TQ;
    can.CAN_Config.CAN_TS1       = CAN_TS1_13TQ;
    can.CAN_Config.CAN_TS2       = CAN_TS2_2TQ;
    can.CAN_Config.CAN_TTCM      = DISABLE;
    can.CAN_Config.CAN_ABOM      = DISABLE;
    can.CAN_Config.CAN_AWUM      = DISABLE;
    can.CAN_Config.CAN_NART      = DISABLE;
    can.CAN_Config.CAN_RFLM      = DISABLE;
    can.CAN_Config.CAN_TXFP      = DISABLE;

    if (CAN_Init(&can) != CAN_OK)
        error_halt();

    CAN_AcceptAllFilter(0U, CAN_FIFO_0);

    if (CAN_Start(CAN1) != CAN_OK)
        error_halt();

    /* Standard frame, ID 0x123, 4 bytes */
    tx.StdId = 0x123U;
    tx.ExtId = 0U;
    tx.IDE   = CAN_ID_STD;
    tx.RTR   = CAN_RTR_DATA;
    tx.DLC   = 4U;
    tx.Data[0] = 0x00U;
    tx.Data[1] = 0xDEU;
    tx.Data[2] = 0xADU;
    tx.Data[3] = 0xBEU;

    while (1)
    {
        tx.Data[0]++;                      /* so each frame looks different */

        if (CAN_AddTxMessage(CAN1, &tx, &mailbox) != CAN_OK)
            error_halt();

        /* wait for the looped-back copy */
        timeout = 1000000U;
        while (CAN_GetRxFifoFillLevel(CAN1, CAN_FIFO_0) == 0U)
        {
            if (--timeout == 0U)
                error_halt();
        }

        CAN_GetRxMessage(CAN1, CAN_FIFO_0, &rx);

        if ((rx.StdId == tx.StdId) && (rx.DLC == tx.DLC) &&
            (rx.Data[0] == tx.Data[0]) && (rx.Data[3] == tx.Data[3]))
        {
            GPIO_ToggleOutputPin(GPIOA, LED_PIN);   /* blink = loopback OK */
        }
        else
        {
            error_halt();
        }

        delay(500000U);                    /* gap between frames */
    }
}
