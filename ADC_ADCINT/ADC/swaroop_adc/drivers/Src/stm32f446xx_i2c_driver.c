/*
 * stm32f446xx_i2c_driver.c
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_i2c_driver.h"
#include "stm32f446xx_rcc_driver.h"   //supplies PCLK1, needed for CCR and TRISE

/* Local helpers - not part of the public API */
static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);


/*********************************************************************
 * @fn      		  - I2C_GenerateStartCondition
 *
 * @brief             - Sets the START bit, taking ownership of the bus
 *
 * @param[in]         - Base address of the I2C peripheral
 *
 * @return            - None
 *
 * @Note              - Local helper. The hardware sets the SB flag once the
 *                      start condition has actually appeared on the bus.
 *********************************************************************/
static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1U << I2C_CR1_START);
}


/*********************************************************************
 * @fn      		  - I2C_GenerateStopCondition
 *
 * @brief             - Sets the STOP bit, releasing the bus
 *
 * @param[in]         - Base address of the I2C peripheral
 *
 * @return            - None
 *********************************************************************/
void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1U << I2C_CR1_STOP);
}


/*********************************************************************
 * @fn      		  - I2C_ExecuteAddressPhaseWrite
 *
 * @brief             - Puts the slave address plus the WRITE bit on the bus
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - 7-bit slave address
 *
 * @return            - None
 *
 * @Note              - Local helper. The 7-bit address occupies bits [7:1]
 *                      of the byte, and bit 0 carries the direction:
 *                      0 = write, 1 = read.
 *********************************************************************/
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    SlaveAddr = (uint8_t)(SlaveAddr << 1);        // make room for the R/W bit
    SlaveAddr &= (uint8_t)~(1U);                  // clear bit 0 : this is a WRITE

    pI2Cx->DR = SlaveAddr;
}


/*********************************************************************
 * @fn      		  - I2C_ExecuteAddressPhaseRead
 *
 * @brief             - Puts the slave address plus the READ bit on the bus
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - 7-bit slave address
 *
 * @return            - None
 *
 * @Note              - Local helper
 *********************************************************************/
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    SlaveAddr = (uint8_t)(SlaveAddr << 1);        // make room for the R/W bit
    SlaveAddr |= 1U;                              // set bit 0 : this is a READ

    pI2Cx->DR = SlaveAddr;
}


/*********************************************************************
 * @fn      		  - I2C_ClearADDRFlag
 *
 * @brief             - Clears the ADDR flag, which unblocks the transfer
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Local helper. ADDR is cleared by reading SR1 and then
 *                      reading SR2 - BOTH reads are required and both
 *                      results are discarded. Until ADDR is cleared the
 *                      hardware stretches SCL and the bus stalls, which is
 *                      why a driver that forgets this appears to hang.
 *
 *                      The single-byte read case is special: ACK must be
 *                      disabled BEFORE clearing ADDR, otherwise the
 *                      hardware acknowledges a byte the master does not
 *                      want.
 *********************************************************************/
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
    volatile uint32_t dummy;

    /* Is this device currently acting as the master? */
    if(pI2CHandle->pI2Cx->SR2 & (1U << I2C_SR2_MSL))
    {
        if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            if(pI2CHandle->RxSize == 1U)
            {
                /* Only one byte expected - stop acknowledging first */
                I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

                dummy = pI2CHandle->pI2Cx->SR1;
                dummy = pI2CHandle->pI2Cx->SR2;
                (void)dummy;
            }
            else
            {
                dummy = pI2CHandle->pI2Cx->SR1;
                dummy = pI2CHandle->pI2Cx->SR2;
                (void)dummy;
            }
        }
        else
        {
            dummy = pI2CHandle->pI2Cx->SR1;
            dummy = pI2CHandle->pI2Cx->SR2;
            (void)dummy;
        }
    }
    else
    {
        /* Slave mode - the sequence is the same */
        dummy = pI2CHandle->pI2Cx->SR1;
        dummy = pI2CHandle->pI2Cx->SR2;
        (void)dummy;
    }
}


/*********************************************************************
 * @fn      		  - I2C_PeriClockControl
 *
 * @brief             - Enables or disables peripheral clock for the I2C
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - All three I2C peripherals sit on APB1
 *********************************************************************/
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pI2Cx == I2C1)
        {
            I2C1_PCLK_EN();
        }
        else if(pI2Cx == I2C2)
        {
            I2C2_PCLK_EN();
        }
        else if(pI2Cx == I2C3)
        {
            I2C3_PCLK_EN();
        }
    }
    else
    {
        if(pI2Cx == I2C1)
        {
            I2C1_PCLK_DI();
        }
        else if(pI2Cx == I2C2)
        {
            I2C2_PCLK_DI();
        }
        else if(pI2Cx == I2C3)
        {
            I2C3_PCLK_DI();
        }
    }
}


/*********************************************************************
 * @fn      		  - I2C_Init
 *
 * @brief             - Initialises an I2C peripheral from its handle
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Does NOT set PE. Call I2C_PeripheralControl
 *                      afterwards. The ACK bit is applied there too,
 *                      because the hardware ignores writes to ACK while
 *                      PE is clear.
 *********************************************************************/
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    uint32_t tempreg = 0;
    uint32_t pclk1;
    uint16_t ccr_value;

    /* Enable the clock for the given I2C peripheral */
    I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

    pclk1 = RCC_GetPCLK1Value();                  // every I2C is on APB1

    /*
     * 1. CR2 FREQ field - the APB1 frequency in MHz.
     *    This is NOT a divider. The peripheral needs to know the input
     *    clock in order to compute its internal timings.
     */
    tempreg = 0;
    tempreg |= (pclk1 / 1000000U);
    pI2CHandle->pI2Cx->CR2 = (tempreg & 0x3FU);

    /*
     * 2. Own address, used only when this device acts as a slave
     */
    tempreg = 0;
    tempreg |= ((uint32_t)pI2CHandle->I2C_Config.I2C_DeviceAddress << I2C_OAR1_ADD71);
    tempreg |= (1U << 14);                        // bit 14 must be kept at 1 by software
    pI2CHandle->pI2Cx->OAR1 = tempreg;

    /*
     * 3. CCR - the clock control register, which sets SCL frequency
     */
    tempreg = 0;

    if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        /* Standard mode: Thigh = Tlow, so CCR = PCLK1 / (2 x SCL) */
        ccr_value = (uint16_t)(pclk1 / (2U * pI2CHandle->I2C_Config.I2C_SCLSpeed));

        tempreg |= (ccr_value & 0xFFFU);
    }
    else
    {
        /* Fast mode */
        tempreg |= (1U << I2C_CCR_FS);
        tempreg |= ((uint32_t)pI2CHandle->I2C_Config.I2C_FMDutyCycle << I2C_CCR_DUTY);

        if(pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
        {
            ccr_value = (uint16_t)(pclk1 / (3U * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        }
        else
        {
            ccr_value = (uint16_t)(pclk1 / (25U * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        }

        tempreg |= (ccr_value & 0xFFFU);
    }

    pI2CHandle->pI2Cx->CCR = tempreg;

    /*
     * 4. TRISE - the maximum allowed SCL rise time, in PCLK1 periods.
     *    Standard mode allows 1000 ns, fast mode only 300 ns.
     */
    if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        tempreg = (pclk1 / 1000000U) + 1U;        // 1000 ns
    }
    else
    {
        tempreg = ((pclk1 * 300U) / 1000000000U) + 1U;   // 300 ns
    }

    pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3FU);

    /* Driver starts idle */
    pI2CHandle->TxRxState = I2C_READY;
}


/*********************************************************************
 * @fn      		  - I2C_DeInit
 *
 * @brief             - Resets the I2C peripheral registers
 *
 * @param[in]         - Base address of the I2C peripheral
 *
 * @return            - None
 *********************************************************************/
void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if(pI2Cx == I2C1)
    {
        I2C1_REG_RESET();
    }
    else if(pI2Cx == I2C2)
    {
        I2C2_REG_RESET();
    }
    else if(pI2Cx == I2C3)
    {
        I2C3_REG_RESET();
    }
}


/*********************************************************************
 * @fn      		  - I2C_GetFlagStatus
 *
 * @brief             - Reads one flag from status register 1
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - Flag mask, from the I2C_FLAG_xxx macros
 *
 * @return            - FLAG_SET or FLAG_RESET
 *********************************************************************/
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
    if(pI2Cx->SR1 & FlagName)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}


/*********************************************************************
 * @fn      		  - I2C_PeripheralControl
 *
 * @brief             - Sets or clears the PE bit, switching the I2C on or off
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Call this AFTER I2C_Init. Acknowledging is enabled
 *                      here as well, because the ACK bit only takes effect
 *                      once PE is already set.
 *********************************************************************/
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pI2Cx->CR1 |= (1U << I2C_CR1_PE);

        /* Now that PE is set, acknowledging can be turned on */
        pI2Cx->CR1 |= (1U << I2C_CR1_ACK);
    }
    else
    {
        pI2Cx->CR1 &= ~(1U << I2C_CR1_PE);
    }
}


/*********************************************************************
 * @fn      		  - I2C_ManageAcking
 *
 * @brief             - Enables or disables automatic acknowledgement
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - I2C_ACK_ENABLE or I2C_ACK_DISABLE
 *
 * @return            - None
 *
 * @Note              - The master must stop acknowledging before the LAST
 *                      byte it wants to receive, so the slave knows to stop
 *                      sending. Getting this wrong by one byte is a very
 *                      common I2C bug.
 *********************************************************************/
void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == I2C_ACK_ENABLE)
    {
        pI2Cx->CR1 |= (1U << I2C_CR1_ACK);
    }
    else
    {
        pI2Cx->CR1 &= ~(1U << I2C_CR1_ACK);
    }
}


/*********************************************************************
 * @fn      		  - I2C_MasterSendData
 *
 * @brief             - Blocking master transmission of Len bytes
 *
 * @param[in]         - Pointer to I2C handle
 * @param[in]         - Address of the transmit buffer
 * @param[in]         - Number of bytes to send
 * @param[in]         - 7-bit slave address
 * @param[in]         - I2C_ENABLE_SR to hold the bus, I2C_DISABLE_SR to stop
 *
 * @return            - None
 *
 * @Note              - Blocking. The sequence is: START, address + write,
 *                      clear ADDR, then feed bytes while TXE is set, and
 *                      finally STOP unless a repeated start was requested.
 *********************************************************************/
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len,
                        uint8_t SlaveAddr, uint8_t Sr)
{
    /* 1. Take the bus */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    /* 2. Wait until the start condition has actually appeared */
    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) == FLAG_RESET);

    /* 3. Send the slave address with the write bit */
    I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

    /* 4. Wait for the slave to acknowledge its address */
    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) == FLAG_RESET);

    /* 5. Clear ADDR, which releases the stretched clock */
    I2C_ClearADDRFlag(pI2CHandle);

    /* 6. Send every byte, waiting for room each time */
    while(Len > 0U)
    {
        while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) == FLAG_RESET);

        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        Len--;
    }

    /* 7. Wait for the last byte to be fully shifted out.
     *    TXE alone is not enough - BTF confirms the shift register is
     *    empty as well, the same distinction as TXE versus TC on the USART. */
    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) == FLAG_RESET);
    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF) == FLAG_RESET);

    /* 8. Release the bus, unless the caller wants a repeated start */
    if(Sr == I2C_DISABLE_SR)
    {
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
    }
}


/*********************************************************************
 * @fn      		  - I2C_MasterReceiveData
 *
 * @brief             - Blocking master reception of Len bytes
 *
 * @param[in]         - Pointer to I2C handle
 * @param[in]         - Address of the receive buffer
 * @param[in]         - Number of bytes to receive
 * @param[in]         - 7-bit slave address
 * @param[in]         - I2C_ENABLE_SR or I2C_DISABLE_SR
 *
 * @return            - None
 *
 * @Note              - Reading one byte and reading many bytes need
 *                      different handling, because ACK must be turned off
 *                      before the final byte arrives. Both cases are
 *                      handled below.
 *********************************************************************/
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len,
                           uint8_t SlaveAddr, uint8_t Sr)
{
    /* 1. Take the bus */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) == FLAG_RESET);

    /* 2. Send the slave address with the read bit */
    I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

    while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) == FLAG_RESET);

    if(Len == 1U)
    {
        /* Single byte: stop acknowledging BEFORE clearing ADDR */
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

        I2C_ClearADDRFlag(pI2CHandle);

        while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE) == FLAG_RESET);

        if(Sr == I2C_DISABLE_SR)
        {
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        }

        *pRxBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
    }
    else
    {
        I2C_ClearADDRFlag(pI2CHandle);

        for(uint32_t i = Len; i > 0U; i--)
        {
            while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE) == FLAG_RESET);

            if(i == 2U)
            {
                /* Two bytes left: stop acknowledging so the slave knows
                 * the next byte is the last one it should send */
                I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

                if(Sr == I2C_DISABLE_SR)
                {
                    I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                }
            }

            *pRxBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
            pRxBuffer++;
        }
    }

    /* Restore acknowledging for the next transfer */
    if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
    }
}


/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             - Starts an interrupt driven master transmission
 *
 * @param[in]         - Pointer to I2C handle
 * @param[in]         - Address of the transmit buffer
 * @param[in]         - Number of bytes to send
 * @param[in]         - 7-bit slave address
 * @param[in]         - I2C_ENABLE_SR or I2C_DISABLE_SR
 *
 * @return            - Previous state (I2C_READY if the transfer started)
 *
 * @Note              - Returns immediately. The transfer then advances one
 *                      step per interrupt, driven by I2C_EV_IRQHandling.
 *********************************************************************/
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len,
                             uint8_t SlaveAddr, uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pTxBuffer = pTxBuffer;
        pI2CHandle->TxLen     = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
        pI2CHandle->DevAddr   = SlaveAddr;
        pI2CHandle->Sr        = Sr;

        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        /* Enable the three interrupt sources the state machine needs */
        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITBUFEN);
        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITEVTEN);
        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITERREN);
    }

    return busystate;
}


/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             - Starts an interrupt driven master reception
 *
 * @param[in]         - Pointer to I2C handle
 * @param[in]         - Address of the receive buffer
 * @param[in]         - Number of bytes to receive
 * @param[in]         - 7-bit slave address
 * @param[in]         - I2C_ENABLE_SR or I2C_DISABLE_SR
 *
 * @return            - Previous state (I2C_READY if the transfer started)
 *********************************************************************/
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len,
                                uint8_t SlaveAddr, uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pRxBuffer = pRxBuffer;
        pI2CHandle->RxLen     = Len;
        pI2CHandle->RxSize    = Len;              // needed later to spot the last byte
        pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
        pI2CHandle->DevAddr   = SlaveAddr;
        pI2CHandle->Sr        = Sr;

        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITBUFEN);
        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITEVTEN);
        pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITERREN);
    }

    return busystate;
}


/*********************************************************************
 * @fn      		  - I2C_CloseSendData
 *
 * @brief             - Ends an interrupt driven transmission
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *********************************************************************/
void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->TxLen     = 0U;
}


/*********************************************************************
 * @fn      		  - I2C_CloseReceiveData
 *
 * @brief             - Ends an interrupt driven reception
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *********************************************************************/
void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->RxLen     = 0U;
    pI2CHandle->RxSize    = 0U;

    /* Restore acknowledging for the next transfer */
    if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
    }
}


/*********************************************************************
 * @fn      		  - I2C_SlaveSendData
 *
 * @brief             - Sends a single byte while acting as a slave
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - The byte to send
 *
 * @return            - None
 *
 * @Note              - Called from the application callback when the event
 *                      I2C_EV_DATA_REQ is reported
 *********************************************************************/
void I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data)
{
    pI2Cx->DR = data;
}


/*********************************************************************
 * @fn      		  - I2C_SlaveReceiveData
 *
 * @brief             - Reads a single byte while acting as a slave
 *
 * @param[in]         - Base address of the I2C peripheral
 *
 * @return            - The received byte
 *
 * @Note              - Called from the application callback when the event
 *                      I2C_EV_DATA_RCV is reported
 *********************************************************************/
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx)
{
    return (uint8_t)pI2Cx->DR;
}


/*********************************************************************
 * @fn      		  - I2C_SlaveEnableDisableCallbackEvents
 *
 * @brief             - Turns the three slave-mode interrupt sources on or off
 *
 * @param[in]         - Base address of the I2C peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *********************************************************************/
void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pI2Cx->CR2 |= (1U << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 |= (1U << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 |= (1U << I2C_CR2_ITERREN);
    }
    else
    {
        pI2Cx->CR2 &= ~(1U << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 &= ~(1U << I2C_CR2_ITERREN);
    }
}


/*********************************************************************
 * @fn      		  - I2C_IRQConfig
 *
 * @brief             - Enables/disables the IRQ and sets its priority
 *
 * @param[in]         - IRQ number, use IRQ_NO_I2Cx_EV or IRQ_NO_I2Cx_ER
 * @param[in]         - IRQ priority, 0 is the most urgent
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Every I2C has TWO vectors, events and errors, so this
 *                      function must be called TWICE per peripheral
 *********************************************************************/
void I2C_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi)
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
 * @fn      		  - I2C_MasterHandleTXEInterrupt
 *
 * @brief             - Feeds one byte into DR during an interrupt transfer
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Local helper
 *********************************************************************/
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle)
{
    if(pI2CHandle->TxLen > 0U)
    {
        pI2CHandle->pI2Cx->DR = *pI2CHandle->pTxBuffer;
        pI2CHandle->pTxBuffer++;
        pI2CHandle->TxLen--;
    }
}


/*********************************************************************
 * @fn      		  - I2C_MasterHandleRXNEInterrupt
 *
 * @brief             - Stores one byte from DR during an interrupt transfer
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Local helper. Acknowledging is turned off when two
 *                      bytes remain, so the final byte is NACKed as the
 *                      protocol requires.
 *********************************************************************/
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle)
{
    if(pI2CHandle->RxSize == 1U)
    {
        *pI2CHandle->pRxBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
        pI2CHandle->RxLen--;
    }
    else if(pI2CHandle->RxSize > 1U)
    {
        if(pI2CHandle->RxLen == 2U)
        {
            /* Two bytes left - stop acknowledging now */
            I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);
        }

        *pI2CHandle->pRxBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
        pI2CHandle->pRxBuffer++;
        pI2CHandle->RxLen--;
    }

    if(pI2CHandle->RxLen == 0U)
    {
        /* Reception finished - release the bus unless holding it */
        if(pI2CHandle->Sr == I2C_DISABLE_SR)
        {
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        }

        I2C_CloseReceiveData(pI2CHandle);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
    }
}


/*********************************************************************
 * @fn      		  - I2C_EV_IRQHandling
 *
 * @brief             - Handles every EVENT interrupt source
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Wire this to the I2Cx_EV_IRQHandler in your
 *                      application. Errors arrive on a SEPARATE vector and
 *                      are handled by I2C_ER_IRQHandling below.
 *********************************************************************/
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1, temp2, temp3;

    temp1 = pI2CHandle->pI2Cx->CR2 & (1U << I2C_CR2_ITEVTEN);
    temp2 = pI2CHandle->pI2Cx->CR2 & (1U << I2C_CR2_ITBUFEN);

    /*************** SB : start condition generated, master only ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_SB);

    if(temp1 && temp3)
    {
        /* SB is cleared by writing the address into DR */
        if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        }
        else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        }
    }

    /*************** ADDR : address sent or matched ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_ADDR);

    if(temp1 && temp3)
    {
        /* Must be cleared or the hardware stretches SCL forever */
        I2C_ClearADDRFlag(pI2CHandle);
    }

    /*************** BTF : byte transfer finished ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_BTF);

    if(temp1 && temp3)
    {
        if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            /* Only close when TXE is also set and nothing remains */
            if(pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_TXE))
            {
                if(pI2CHandle->TxLen == 0U)
                {
                    if(pI2CHandle->Sr == I2C_DISABLE_SR)
                    {
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                    }

                    I2C_CloseSendData(pI2CHandle);

                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
                }
            }
        }
    }

    /*************** STOPF : stop detected, slave mode only ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_STOPF);

    if(temp1 && temp3)
    {
        /* STOPF is cleared by reading SR1 then writing to CR1 */
        pI2CHandle->pI2Cx->CR1 |= 0x0000U;

        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
    }

    /*************** TXE : data register empty ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_TXE);

    if(temp1 && temp2 && temp3)
    {
        if(pI2CHandle->pI2Cx->SR2 & (1U << I2C_SR2_MSL))
        {
            /* Master mode */
            if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
            {
                I2C_MasterHandleTXEInterrupt(pI2CHandle);
            }
        }
        else
        {
            /* Slave mode - only act if we are the transmitter */
            if(pI2CHandle->pI2Cx->SR2 & (1U << I2C_SR2_TRA))
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
            }
        }
    }

    /*************** RXNE : data register not empty ***************/

    temp3 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_RXNE);

    if(temp1 && temp2 && temp3)
    {
        if(pI2CHandle->pI2Cx->SR2 & (1U << I2C_SR2_MSL))
        {
            /* Master mode */
            if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
            {
                I2C_MasterHandleRXNEInterrupt(pI2CHandle);
            }
        }
        else
        {
            /* Slave mode - only act if we are the receiver */
            if(!(pI2CHandle->pI2Cx->SR2 & (1U << I2C_SR2_TRA)))
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
            }
        }
    }
}


/*********************************************************************
 * @fn      		  - I2C_ER_IRQHandling
 *
 * @brief             - Handles every ERROR interrupt source
 *
 * @param[in]         - Pointer to I2C handle
 *
 * @return            - None
 *
 * @Note              - Wire this to the I2Cx_ER_IRQHandler. The most common
 *                      error in practice is AF, acknowledge failure, which
 *                      means no device on the bus responded to the address -
 *                      usually a wrong address, a missing pull-up, or an
 *                      unpowered slave.
 *********************************************************************/
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1, temp2;

    /* Is the error interrupt enabled at all? */
    temp2 = pI2CHandle->pI2Cx->CR2 & (1U << I2C_CR2_ITERREN);

    /*************** Bus error ***************/

    temp1 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_BERR);

    if(temp1 && temp2)
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_BERR);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
    }

    /*************** Arbitration lost ***************/

    temp1 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_ARLO);

    if(temp1 && temp2)
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_ARLO);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
    }

    /*************** Acknowledge failure ***************/

    temp1 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_AF);

    if(temp1 && temp2)
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_AF);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
    }

    /*************** Overrun or underrun ***************/

    temp1 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_OVR);

    if(temp1 && temp2)
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_OVR);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
    }

    /*************** Timeout ***************/

    temp1 = pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_TIMEOUT);

    if(temp1 && temp2)
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_TIMEOUT);

        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
    }
}


/*********************************************************************
 * @fn      		  - I2C_ApplicationEventCallback
 *
 * @brief             - Weak default callback
 *
 * @param[in]         - Pointer to I2C handle
 * @param[in]         - Event code, from the I2C_EV_xxx or I2C_ERROR_xxx macros
 *
 * @return            - None
 *
 * @Note              - The application should override this. Declared weak so
 *                      the project still links if it does not.
 *********************************************************************/
__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,
                                                        uint8_t AppEvent)
{
    (void)pI2CHandle;
    (void)AppEvent;

    /* Intentionally empty - override in the application */
}
