#include "stm32f446xx.h"

static void delay(void)
{
    for(volatile uint32_t i = 0; i < 300000; i++);
}

int main(void)
{
    GPIO_Handle_t led;
    CAN_Handle_t  can;
    CAN_Message_t tx, rx;
    uint8_t mailbox, ok, i;

    /* LED on PA5 (Nucleo user LED) */
    led.pGPIOx = GPIOA;
    led.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;
    led.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_LOW;
    led.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&led);

    /* CAN1 in loopback, 500 kbit/s on the 16 MHz HSI: 16M / 2 / (1+13+2) */
    can.pCANx = CAN1;
    can.CAN_Config.CAN_Prescaler = 2;
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

    if(CAN_Init(&can) != CAN_OK) { while(1); }

    CAN_AcceptAllFilter(0, CAN_FIFO_0);      // no filter = no reception

    if(CAN_Start(CAN1) != CAN_OK) { while(1); }

    /* message to send */
    tx.StdId = 0x123;
    tx.ExtId = 0;
    tx.IDE   = CAN_ID_STD;
    tx.RTR   = CAN_RTR_DATA;
    tx.DLC   = 4;
    tx.Data[0] = 0x00;
    tx.Data[1] = 0xDE;
    tx.Data[2] = 0xAD;
    tx.Data[3] = 0xBE;

    while(1)
    {
        tx.Data[0]++;                        // changes every round so you can see it live

        CAN_AddTxMessage(CAN1, &tx, &mailbox);

        while(CAN_GetRxFifoFillLevel(CAN1, CAN_FIFO_0) == 0);
        CAN_GetRxMessage(CAN1, CAN_FIFO_0, &rx);

        /* did we get back exactly what we sent? */
        ok = (rx.StdId == tx.StdId) && (rx.DLC == tx.DLC);
        for(i = 0; i < tx.DLC; i++)
        {
            if(rx.Data[i] != tx.Data[i]) ok = 0;
        }

        if(ok)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);   // blinks = working
            delay();
        }
        else
        {
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, GPIO_PIN_SET);  // stuck on = mismatch
            while(1);
        }
    }
}
