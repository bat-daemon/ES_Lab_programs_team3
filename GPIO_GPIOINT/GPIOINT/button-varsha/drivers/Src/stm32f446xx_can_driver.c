/*
 * stm32f446xx_can_driver.c
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_can_driver.h"

/*
 * How long to spin while waiting for a hardware acknowledge bit.
 * CAN mode changes are not instantaneous - the controller must observe a
 * quiet bus before it will join or leave, so a bounded wait is needed
 * rather than an unbounded one.
 */
#define CAN_TIMEOUT_COUNT       1000000U


/*********************************************************************
 * @fn      		  - CAN_PeriClockControl
 *
 * @brief             - Enables or disables peripheral clock for the CAN
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Both controllers are on APB1.
 *                      IMPORTANT: CAN2 is a SLAVE of CAN1 on this device.
 *                      CAN1 owns the shared filter bank block, so CAN1's
 *                      clock must be enabled before CAN2 can be used at
 *                      all - even in an application that never uses CAN1.
 *********************************************************************/
void CAN_PeriClockControl(CAN_RegDef_t *pCANx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pCANx == CAN1)
        {
            CAN1_PCLK_EN();
        }
        else if(pCANx == CAN2)
        {
            CAN1_PCLK_EN();                       // required: CAN2 depends on CAN1
            CAN2_PCLK_EN();
        }
    }
    else
    {
        if(pCANx == CAN1)
        {
            CAN1_PCLK_DI();
        }
        else if(pCANx == CAN2)
        {
            CAN2_PCLK_DI();
        }
    }
}


/*********************************************************************
 * @fn      		  - CAN_Init
 *
 * @brief             - Initialises a CAN peripheral from its handle
 *
 * @param[in]         - Pointer to CAN handle
 *
 * @return            - CAN_OK, or CAN_TIMEOUT if the controller never
 *                      acknowledged the mode change
 *
 * @Note              - The controller wakes from reset in SLEEP mode, so
 *                      the sleep bit must be cleared before anything else
 *                      will work. Bit timing can only be written while the
 *                      controller is in INITIALISATION mode, which is why
 *                      the sequence below is strictly ordered.
 *                      Call CAN_Start afterwards to join the bus.
 *********************************************************************/
uint8_t CAN_Init(CAN_Handle_t *pCANHandle)
{
    uint32_t timeout;
    uint32_t tempreg = 0;

    /* Enable the clock for the given CAN peripheral */
    CAN_PeriClockControl(pCANHandle->pCANx, ENABLE);

    /*
     * 1. Leave sleep mode. The hardware powers up asleep, and every other
     *    setting is ignored until it wakes.
     */
    pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_SLEEP);

    timeout = CAN_TIMEOUT_COUNT;
    while((pCANHandle->pCANx->MSR & (1U << CAN_MSR_SLAK)) && (timeout > 0U))
    {
        timeout--;
    }
    if(timeout == 0U)
    {
        return CAN_TIMEOUT;                       // never woke up
    }

    /*
     * 2. Request initialisation mode. Bit timing is write protected
     *    outside this mode.
     */
    pCANHandle->pCANx->MCR |= (1U << CAN_MCR_INRQ);

    timeout = CAN_TIMEOUT_COUNT;
    while((!(pCANHandle->pCANx->MSR & (1U << CAN_MSR_INAK))) && (timeout > 0U))
    {
        timeout--;
    }
    if(timeout == 0U)
    {
        return CAN_TIMEOUT;                       // never entered init mode
    }

    /*
     * 3. Behaviour options in MCR
     */
    if(pCANHandle->CAN_Config.CAN_TTCM == ENABLE)
    {
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_TTCM);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_TTCM);
    }

    if(pCANHandle->CAN_Config.CAN_ABOM == ENABLE)
    {
        /* Recover automatically after a bus-off condition */
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_ABOM);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_ABOM);
    }

    if(pCANHandle->CAN_Config.CAN_AWUM == ENABLE)
    {
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_AWUM);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_AWUM);
    }

    if(pCANHandle->CAN_Config.CAN_NART == ENABLE)
    {
        /* Do NOT retry a failed transmission. Off by default, because
         * automatic retry is the behaviour the CAN standard expects. */
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_NART);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_NART);
    }

    if(pCANHandle->CAN_Config.CAN_RFLM == ENABLE)
    {
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_RFLM);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_RFLM);
    }

    if(pCANHandle->CAN_Config.CAN_TXFP == ENABLE)
    {
        pCANHandle->pCANx->MCR |= (1U << CAN_MCR_TXFP);
    }
    else
    {
        pCANHandle->pCANx->MCR &= ~(1U << CAN_MCR_TXFP);
    }

    /*
     * 4. Bit timing in BTR.
     *
     *    Bit rate = PCLK1 / Prescaler / (1 + TS1 + TS2)
     *
     *    Example on a 45 MHz APB1 for 500 kbit/s:
     *        Prescaler = 5, TS1 = 12 tq, TS2 = 5 tq
     *        45e6 / 5 / (1 + 12 + 5) = 500000
     *
     *    Every node on the bus must agree on the resulting bit rate, and
     *    their sample points should be reasonably close together.
     */
    tempreg = 0;
    tempreg |= ((uint32_t)pCANHandle->CAN_Config.CAN_SJW << CAN_BTR_SJW);
    tempreg |= ((uint32_t)pCANHandle->CAN_Config.CAN_TS1 << CAN_BTR_TS1);
    tempreg |= ((uint32_t)pCANHandle->CAN_Config.CAN_TS2 << CAN_BTR_TS2);
    tempreg |= ((pCANHandle->CAN_Config.CAN_Prescaler - 1U) & 0x3FFU);

    /*
     * 5. Test mode bits, if loopback or silent was requested
     */
    if((pCANHandle->CAN_Config.CAN_Mode == CAN_MODE_LOOPBACK) ||
       (pCANHandle->CAN_Config.CAN_Mode == CAN_MODE_SILENT_LOOPBACK))
    {
        tempreg |= (1U << CAN_BTR_LBKM);
    }

    if((pCANHandle->CAN_Config.CAN_Mode == CAN_MODE_SILENT) ||
       (pCANHandle->CAN_Config.CAN_Mode == CAN_MODE_SILENT_LOOPBACK))
    {
        tempreg |= (1U << CAN_BTR_SILM);
    }

    pCANHandle->pCANx->BTR = tempreg;

    return CAN_OK;
}


/*********************************************************************
 * @fn      		  - CAN_DeInit
 *
 * @brief             - Resets the CAN peripheral registers
 *
 * @param[in]         - Base address of the CAN peripheral
 *
 * @return            - None
 *********************************************************************/
void CAN_DeInit(CAN_RegDef_t *pCANx)
{
    if(pCANx == CAN1)
    {
        CAN1_REG_RESET();
    }
    else if(pCANx == CAN2)
    {
        CAN2_REG_RESET();
    }
}


/*********************************************************************
 * @fn      		  - CAN_Start
 *
 * @brief             - Leaves initialisation mode and joins the bus
 *
 * @param[in]         - Base address of the CAN peripheral
 *
 * @return            - CAN_OK, or CAN_TIMEOUT if the bus never went quiet
 *
 * @Note              - The controller will not join until it has observed
 *                      11 consecutive recessive bits, so this can legitimately
 *                      time out if the bus is stuck dominant - for example a
 *                      shorted pair, or a missing transceiver in normal mode.
 *********************************************************************/
uint8_t CAN_Start(CAN_RegDef_t *pCANx)
{
    uint32_t timeout;

    pCANx->MCR &= ~(1U << CAN_MCR_INRQ);          // request normal mode

    timeout = CAN_TIMEOUT_COUNT;
    while((pCANx->MSR & (1U << CAN_MSR_INAK)) && (timeout > 0U))
    {
        timeout--;
    }

    if(timeout == 0U)
    {
        return CAN_TIMEOUT;
    }

    return CAN_OK;
}


/*********************************************************************
 * @fn      		  - CAN_Stop
 *
 * @brief             - Re-enters initialisation mode, leaving the bus
 *
 * @param[in]         - Base address of the CAN peripheral
 *
 * @return            - CAN_OK, or CAN_TIMEOUT
 *********************************************************************/
uint8_t CAN_Stop(CAN_RegDef_t *pCANx)
{
    uint32_t timeout;

    pCANx->MCR |= (1U << CAN_MCR_INRQ);

    timeout = CAN_TIMEOUT_COUNT;
    while((!(pCANx->MSR & (1U << CAN_MSR_INAK))) && (timeout > 0U))
    {
        timeout--;
    }

    if(timeout == 0U)
    {
        return CAN_TIMEOUT;
    }

    return CAN_OK;
}


/*********************************************************************
 * @fn      		  - CAN_ConfigFilter
 *
 * @brief             - Programs one filter bank
 *
 * @param[in]         - Filter bank number, 0 to 27
 * @param[in]         - CAN_FILTER_MODE_IDMASK or CAN_FILTER_MODE_IDLIST
 * @param[in]         - CAN_FILTER_SCALE_16BIT or CAN_FILTER_SCALE_32BIT
 * @param[in]         - Identifier to match, already shifted into position
 * @param[in]         - Mask, or a second identifier in list mode
 * @param[in]         - CAN_FIFO_0 or CAN_FIFO_1
 *
 * @return            - None
 *
 * @Note              - A CAN node receives NOTHING until at least one
 *                      filter bank is configured and activated. This is the
 *                      single most common reason a newly written CAN driver
 *                      transmits correctly but never receives anything.
 *
 *                      All filter banks live in CAN1's register block and
 *                      are shared with CAN2, which is why this function
 *                      always writes through CAN1.
 *********************************************************************/
void CAN_ConfigFilter(uint8_t FilterBank, uint8_t FilterMode, uint8_t FilterScale,
                      uint32_t FilterId, uint32_t FilterMask, uint8_t FifoAssign)
{
    if(FilterBank > 27U)
    {
        return;                                   // illegal bank, do nothing
    }

    /* 1. Enter filter initialisation mode */
    CAN1->FMR |= (1U << CAN_FMR_FINIT);

    /* 2. Deactivate the bank before changing it */
    CAN1->FA1R &= ~(1U << FilterBank);

    /* 3. Scale : one 32-bit filter, or two 16-bit filters */
    if(FilterScale == CAN_FILTER_SCALE_32BIT)
    {
        CAN1->FS1R |= (1U << FilterBank);
    }
    else
    {
        CAN1->FS1R &= ~(1U << FilterBank);
    }

    /* 4. Mode : mask accepts a range, list accepts exact identifiers */
    if(FilterMode == CAN_FILTER_MODE_IDLIST)
    {
        CAN1->FM1R |= (1U << FilterBank);
    }
    else
    {
        CAN1->FM1R &= ~(1U << FilterBank);
    }

    /* 5. The identifier and the mask themselves */
    CAN1->sFilterRegister[FilterBank].FR1 = FilterId;
    CAN1->sFilterRegister[FilterBank].FR2 = FilterMask;

    /* 6. Which FIFO a match is delivered to */
    if(FifoAssign == CAN_FIFO_1)
    {
        CAN1->FFA1R |= (1U << FilterBank);
    }
    else
    {
        CAN1->FFA1R &= ~(1U << FilterBank);
    }

    /* 7. Activate the bank */
    CAN1->FA1R |= (1U << FilterBank);

    /* 8. Leave filter initialisation mode */
    CAN1->FMR &= ~(1U << CAN_FMR_FINIT);
}


/*********************************************************************
 * @fn      		  - CAN_AcceptAllFilter
 *
 * @brief             - Configures one bank to accept EVERY message
 *
 * @param[in]         - Filter bank number, 0 to 27
 * @param[in]         - CAN_FIFO_0 or CAN_FIFO_1
 *
 * @return            - None
 *
 * @Note              - A mask of zero means "do not care about any bit", so
 *                      every identifier matches. This is the right starting
 *                      point for a first experiment: get messages flowing,
 *                      then narrow the filter once that works.
 *********************************************************************/
void CAN_AcceptAllFilter(uint8_t FilterBank, uint8_t FifoAssign)
{
    CAN_ConfigFilter(FilterBank,
                     CAN_FILTER_MODE_IDMASK,
                     CAN_FILTER_SCALE_32BIT,
                     0x00000000U,                 // identifier : anything
                     0x00000000U,                 // mask : do not care about any bit
                     FifoAssign);
}


/*********************************************************************
 * @fn      		  - CAN_IsTxMailboxFree
 *
 * @brief             - Reports which transmit mailbox is available
 *
 * @param[in]         - Base address of the CAN peripheral
 *
 * @return            - 0, 1 or 2, or CAN_TX_NO_MAILBOX if all are busy
 *
 * @Note              - There are three mailboxes, so up to three messages
 *                      can be queued for transmission at once
 *********************************************************************/
uint8_t CAN_IsTxMailboxFree(CAN_RegDef_t *pCANx)
{
    if(pCANx->TSR & (1U << CAN_TSR_TME0))
    {
        return CAN_TX_MAILBOX_0;
    }
    else if(pCANx->TSR & (1U << CAN_TSR_TME1))
    {
        return CAN_TX_MAILBOX_1;
    }
    else if(pCANx->TSR & (1U << CAN_TSR_TME2))
    {
        return CAN_TX_MAILBOX_2;
    }

    return CAN_TX_NO_MAILBOX;
}


/*********************************************************************
 * @fn      		  - CAN_AddTxMessage
 *
 * @brief             - Queues one message into a free transmit mailbox
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - Pointer to the message to send
 * @param[out]        - Receives the mailbox number that was used
 *
 * @return            - CAN_OK, or CAN_ERROR if no mailbox was free
 *
 * @Note              - This does NOT wait for the message to leave. The
 *                      hardware transmits it when the bus is next idle and
 *                      arbitration is won. A message with a numerically
 *                      LOWER identifier wins arbitration against a higher
 *                      one, so identifier choice is effectively a priority
 *                      decision made at design time.
 *********************************************************************/
uint8_t CAN_AddTxMessage(CAN_RegDef_t *pCANx, CAN_Message_t *pMessage, uint8_t *pMailbox)
{
    uint8_t  mailbox;
    uint32_t tempreg = 0;

    mailbox = CAN_IsTxMailboxFree(pCANx);

    if(mailbox == CAN_TX_NO_MAILBOX)
    {
        return CAN_ERROR;                         // all three are still busy
    }

    if(pMailbox != NULL)
    {
        *pMailbox = mailbox;
    }

    /*
     * 1. Identifier register. Standard IDs sit in bits [31:21], extended
     *    IDs occupy bits [31:3]. The TXRQ bit is written last, separately.
     */
    if(pMessage->IDE == CAN_ID_STD)
    {
        tempreg = ((pMessage->StdId & 0x7FFU) << CAN_TIR_STID);
    }
    else
    {
        tempreg  = ((pMessage->ExtId & 0x1FFFFFFFU) << CAN_TIR_EXID);
        tempreg |= (1U << CAN_TIR_IDE);
    }

    if(pMessage->RTR == CAN_RTR_REMOTE)
    {
        tempreg |= (1U << CAN_TIR_RTR);
    }

    pCANx->sTxMailBox[mailbox].TIR = tempreg;

    /*
     * 2. Data length code. A CAN frame carries at most 8 bytes.
     */
    pCANx->sTxMailBox[mailbox].TDTR = (pMessage->DLC & 0x0FU);

    /*
     * 3. Payload, packed into two 32-bit registers
     */
    pCANx->sTxMailBox[mailbox].TDLR = ((uint32_t)pMessage->Data[3] << 24) |
                                      ((uint32_t)pMessage->Data[2] << 16) |
                                      ((uint32_t)pMessage->Data[1] << 8)  |
                                      ((uint32_t)pMessage->Data[0]);

    pCANx->sTxMailBox[mailbox].TDHR = ((uint32_t)pMessage->Data[7] << 24) |
                                      ((uint32_t)pMessage->Data[6] << 16) |
                                      ((uint32_t)pMessage->Data[5] << 8)  |
                                      ((uint32_t)pMessage->Data[4]);

    /*
     * 4. Request transmission. Everything above must already be in place,
     *    because the hardware may start sending immediately.
     */
    pCANx->sTxMailBox[mailbox].TIR |= (1U << CAN_TIR_TXRQ);

    return CAN_OK;
}


/*********************************************************************
 * @fn      		  - CAN_GetRxFifoFillLevel
 *
 * @brief             - Reports how many messages are waiting in a FIFO
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - CAN_FIFO_0 or CAN_FIFO_1
 *
 * @return            - 0 to 3
 *
 * @Note              - Each FIFO holds three messages. Poll this in a
 *                      blocking application, or use the interrupt instead.
 *********************************************************************/
uint8_t CAN_GetRxFifoFillLevel(CAN_RegDef_t *pCANx, uint8_t Fifo)
{
    if(Fifo == CAN_FIFO_0)
    {
        return (uint8_t)(pCANx->RF0R & 0x3U);
    }

    return (uint8_t)(pCANx->RF1R & 0x3U);
}


/*********************************************************************
 * @fn      		  - CAN_GetRxMessage
 *
 * @brief             - Reads one message out of a receive FIFO
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - CAN_FIFO_0 or CAN_FIFO_1
 * @param[out]        - Pointer to the message structure to fill
 *
 * @return            - CAN_OK, or CAN_ERROR if the FIFO was empty
 *
 * @Note              - The RFOM bit MUST be written at the end to release
 *                      the mailbox. Forget it and the same message is read
 *                      forever and no new ones are accepted - the CAN
 *                      equivalent of failing to clear EXTI_PR.
 *********************************************************************/
uint8_t CAN_GetRxMessage(CAN_RegDef_t *pCANx, uint8_t Fifo, CAN_Message_t *pMessage)
{
    uint32_t rir, rdtr, rdlr, rdhr;

    if(CAN_GetRxFifoFillLevel(pCANx, Fifo) == 0U)
    {
        return CAN_ERROR;                         // nothing to read
    }

    rir  = pCANx->sFIFOMailBox[Fifo].RIR;
    rdtr = pCANx->sFIFOMailBox[Fifo].RDTR;
    rdlr = pCANx->sFIFOMailBox[Fifo].RDLR;
    rdhr = pCANx->sFIFOMailBox[Fifo].RDHR;

    /* 1. Identifier type and value */
    pMessage->IDE = (uint8_t)((rir >> CAN_RIR_IDE) & 0x1U);

    if(pMessage->IDE == CAN_ID_STD)
    {
        pMessage->StdId = (rir >> CAN_RIR_STID) & 0x7FFU;
        pMessage->ExtId = 0U;
    }
    else
    {
        pMessage->StdId = 0U;
        pMessage->ExtId = (rir >> CAN_RIR_EXID) & 0x1FFFFFFFU;
    }

    /* 2. Frame type : data or remote */
    pMessage->RTR = (uint8_t)((rir >> CAN_RIR_RTR) & 0x1U);

    /* 3. Length and which filter bank matched */
    pMessage->DLC = (uint8_t)(rdtr & 0x0FU);
    pMessage->FMI = (uint8_t)((rdtr >> 8) & 0xFFU);

    /* 4. Payload, unpacked from the two data registers */
    pMessage->Data[0] = (uint8_t)(rdlr & 0xFFU);
    pMessage->Data[1] = (uint8_t)((rdlr >> 8) & 0xFFU);
    pMessage->Data[2] = (uint8_t)((rdlr >> 16) & 0xFFU);
    pMessage->Data[3] = (uint8_t)((rdlr >> 24) & 0xFFU);
    pMessage->Data[4] = (uint8_t)(rdhr & 0xFFU);
    pMessage->Data[5] = (uint8_t)((rdhr >> 8) & 0xFFU);
    pMessage->Data[6] = (uint8_t)((rdhr >> 16) & 0xFFU);
    pMessage->Data[7] = (uint8_t)((rdhr >> 24) & 0xFFU);

    /* 5. Release the FIFO output mailbox so the next message can arrive */
    if(Fifo == CAN_FIFO_0)
    {
        pCANx->RF0R |= (1U << CAN_RFR_RFOM);
    }
    else
    {
        pCANx->RF1R |= (1U << CAN_RFR_RFOM);
    }

    return CAN_OK;
}


/*********************************************************************
 * @fn      		  - CAN_InterruptEnable
 *
 * @brief             - Sets one enable bit in the interrupt enable register
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - Bit position, from the CAN_IER_xxx macros
 *
 * @return            - None
 *********************************************************************/
void CAN_InterruptEnable(CAN_RegDef_t *pCANx, uint32_t InterruptBit)
{
    pCANx->IER |= (1U << InterruptBit);
}


/*********************************************************************
 * @fn      		  - CAN_InterruptDisable
 *
 * @brief             - Clears one enable bit in the interrupt enable register
 *
 * @param[in]         - Base address of the CAN peripheral
 * @param[in]         - Bit position, from the CAN_IER_xxx macros
 *
 * @return            - None
 *********************************************************************/
void CAN_InterruptDisable(CAN_RegDef_t *pCANx, uint32_t InterruptBit)
{
    pCANx->IER &= ~(1U << InterruptBit);
}


/*********************************************************************
 * @fn      		  - CAN_IRQConfig
 *
 * @brief             - Enables/disables the IRQ and sets its priority
 *
 * @param[in]         - IRQ number, use IRQ_NO_CANx_TX, _RX0, _RX1 or _SCE
 * @param[in]         - IRQ priority, 0 is the most urgent
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Each controller has FOUR separate vectors, so this
 *                      may need to be called up to four times per peripheral
 *********************************************************************/
void CAN_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(IRQNumber <= 31U)
        {
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if(IRQNumber <= 63U)
        {
            *NVIC_ISER1 |= (1U << (IRQNumber % 32U));
        }
        else if(IRQNumber <= 95U)
        {
            *NVIC_ISER2 |= (1U << (IRQNumber % 32U));
        }
    }
    else
    {
        if(IRQNumber <= 31U)
        {
            *NVIC_ICER0 |= (1U << IRQNumber);
        }
        else if(IRQNumber <= 63U)
        {
            *NVIC_ICER1 |= (1U << (IRQNumber % 32U));
        }
        else if(IRQNumber <= 95U)
        {
            *NVIC_ICER2 |= (1U << (IRQNumber % 32U));
        }
    }

    uint8_t iprx         = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;

    uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
    *(NVIC_PR_BASE_ADDR + iprx) |=  (IRQPriority << shift_amount);
}


/*********************************************************************
 * @fn      		  - CAN_TX_IRQHandling
 *
 * @brief             - Handles the transmit mailbox empty interrupt
 *
 * @param[in]         - Pointer to CAN handle
 *
 * @return            - None
 *
 * @Note              - Wire this to CANx_TX_IRQHandler. The RQCP flags are
 *                      cleared by writing 1 to them.
 *********************************************************************/
void CAN_TX_IRQHandling(CAN_Handle_t *pCANHandle)
{
    uint8_t reported = 0U;

    if(pCANHandle->pCANx->TSR & (1U << CAN_TSR_RQCP0))
    {
        pCANHandle->pCANx->TSR |= (1U << CAN_TSR_RQCP0);   // write 1 to clear
        reported = 1U;
    }

    if(pCANHandle->pCANx->TSR & (1U << CAN_TSR_RQCP1))
    {
        pCANHandle->pCANx->TSR |= (1U << CAN_TSR_RQCP1);
        reported = 1U;
    }

    if(pCANHandle->pCANx->TSR & (1U << CAN_TSR_RQCP2))
    {
        pCANHandle->pCANx->TSR |= (1U << CAN_TSR_RQCP2);
        reported = 1U;
    }

    if(reported)
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_EVENT_TX_CMPLT);
    }
}


/*********************************************************************
 * @fn      		  - CAN_RX0_IRQHandling
 *
 * @brief             - Handles the FIFO 0 message pending interrupt
 *
 * @param[in]         - Pointer to CAN handle
 *
 * @return            - None
 *
 * @Note              - Wire this to CANx_RX0_IRQHandler. The application
 *                      callback must call CAN_GetRxMessage, otherwise the
 *                      FIFO stays full and the interrupt re-fires forever.
 *********************************************************************/
void CAN_RX0_IRQHandling(CAN_Handle_t *pCANHandle)
{
    if(CAN_GetRxFifoFillLevel(pCANHandle->pCANx, CAN_FIFO_0) > 0U)
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_EVENT_RX_FIFO0);
    }
}


/*********************************************************************
 * @fn      		  - CAN_RX1_IRQHandling
 *
 * @brief             - Handles the FIFO 1 message pending interrupt
 *
 * @param[in]         - Pointer to CAN handle
 *
 * @return            - None
 *********************************************************************/
void CAN_RX1_IRQHandling(CAN_Handle_t *pCANHandle)
{
    if(CAN_GetRxFifoFillLevel(pCANHandle->pCANx, CAN_FIFO_1) > 0U)
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_EVENT_RX_FIFO1);
    }
}


/*********************************************************************
 * @fn      		  - CAN_SCE_IRQHandling
 *
 * @brief             - Handles status change and error interrupts
 *
 * @param[in]         - Pointer to CAN handle
 *
 * @return            - None
 *
 * @Note              - Wire this to CANx_SCE_IRQHandler.
 *                      BUS OFF is the serious one: after 255 transmit
 *                      errors the controller disconnects itself from the
 *                      bus entirely and stops both sending and receiving.
 *                      In practice it usually means a wiring fault, a
 *                      bit-rate mismatch, or missing termination.
 *********************************************************************/
void CAN_SCE_IRQHandling(CAN_Handle_t *pCANHandle)
{
    uint32_t esr = pCANHandle->pCANx->ESR;

    if(esr & (1U << 2))                           // BOFF : bus off
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_ERROR_BOF);
    }
    else if(esr & (1U << 1))                      // EPVF : error passive
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_ERROR_EPV);
    }
    else if(esr & (1U << 0))                      // EWGF : error warning
    {
        CAN_ApplicationEventCallback(pCANHandle, CAN_ERROR_EWG);
    }

    /* ERRI is cleared by writing 1 to it */
    pCANHandle->pCANx->MSR |= (1U << CAN_MSR_ERRI);
}


/*********************************************************************
 * @fn      		  - CAN_ApplicationEventCallback
 *
 * @brief             - Weak default callback
 *
 * @param[in]         - Pointer to CAN handle
 * @param[in]         - Event code, from the CAN_EVENT_xxx or CAN_ERROR_xxx macros
 *
 * @return            - None
 *
 * @Note              - The application should override this. Declared weak so
 *                      the project still links if it does not.
 *********************************************************************/
__attribute__((weak)) void CAN_ApplicationEventCallback(CAN_Handle_t *pCANHandle,
                                                        uint8_t AppEvent)
{
    (void)pCANHandle;
    (void)AppEvent;

    /* Intentionally empty - override in the application */
}
