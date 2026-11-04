/*
 * stm32f446xx_spi_driver.c
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_spi_driver.h"

/* Local helpers used only by SPI_IRQHandling - not part of the public API */
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);


/*********************************************************************
 * @fn      		  - SPI_PeriClockControl
 *
 * @brief             - Enables or disables peripheral clock for the SPI
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - SPI1 and SPI4 are on APB2, SPI2 and SPI3 on APB1
 *********************************************************************/
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pSPIx == SPI1)
        {
            SPI1_PCLK_EN();
        }
        else if(pSPIx == SPI2)
        {
            SPI2_PCLK_EN();
        }
        else if(pSPIx == SPI3)
        {
            SPI3_PCLK_EN();
        }
        else if(pSPIx == SPI4)
        {
            SPI4_PCLK_EN();
        }
    }
    else
    {
        if(pSPIx == SPI1)
        {
            SPI1_PCLK_DI();
        }
        else if(pSPIx == SPI2)
        {
            SPI2_PCLK_DI();
        }
        else if(pSPIx == SPI3)
        {
            SPI3_PCLK_DI();
        }
        else if(pSPIx == SPI4)
        {
            SPI4_PCLK_DI();
        }
    }
}


/*********************************************************************
 * @fn      		  - SPI_Init
 *
 * @brief             - Initialises a SPI peripheral from its handle
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *
 * @Note              - Does NOT set SPE. Call SPI_PeripheralControl
 *                      afterwards, in the same way USART_Init leaves UE
 *                      alone. Configuring a running peripheral is
 *                      undefined behaviour.
 *********************************************************************/
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
    uint32_t tempreg = 0;

    /* Enable the clock for the given SPI peripheral */
    SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

    /*
     * 1. Device mode - master or slave
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_DeviceMode << SPI_CR1_MSTR);

    /*
     * 2. Bus configuration
     */
    if(pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_FD)
    {
        /* Full duplex: clear BIDIMODE so MOSI and MISO work independently */
        tempreg &= ~(1U << SPI_CR1_BIDIMODE);
    }
    else if(pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_HD)
    {
        /* Half duplex: one bidirectional line */
        tempreg |= (1U << SPI_CR1_BIDIMODE);
    }
    else if(pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
    {
        /* Two lines, but the master never drives MOSI */
        tempreg &= ~(1U << SPI_CR1_BIDIMODE);
        tempreg |=  (1U << SPI_CR1_RXONLY);
    }

    /*
     * 3. Serial clock speed - the baud rate divider
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_SclkSpeed << SPI_CR1_BR);

    /*
     * 4. Data frame format - 8 or 16 bits per transfer
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_DFF << SPI_CR1_DFF);

    /*
     * 5. Clock polarity - the idle level of SCLK
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_CPOL << SPI_CR1_CPOL);

    /*
     * 6. Clock phase - which edge samples the data
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_CPHA << SPI_CR1_CPHA);

    /*
     * 7. Software slave management
     */
    tempreg |= (pSPIHandle->SPI_Config.SPI_SSM << SPI_CR1_SSM);

    pSPIHandle->pSPIx->CR1 = tempreg;

    /*
     * 8. With SSM enabled in master mode, SSI must be driven HIGH.
     *    If SSI is left low the hardware thinks another master has pulled
     *    NSS down, sets the MODF flag and silently clears MSTR - the
     *    classic "my SPI does nothing" bug.
     */
    if((pSPIHandle->SPI_Config.SPI_SSM == SPI_SSM_EN) &&
       (pSPIHandle->SPI_Config.SPI_DeviceMode == SPI_DEVICE_MODE_MASTER))
    {
        SPI_SSIConfig(pSPIHandle->pSPIx, ENABLE);
    }

    /* Driver starts idle */
    pSPIHandle->TxState = SPI_READY;
    pSPIHandle->RxState = SPI_READY;
}


/*********************************************************************
 * @fn      		  - SPI_DeInit
 *
 * @brief             - Resets the SPI peripheral registers
 *
 * @param[in]         - Base address of the SPI peripheral
 *
 * @return            - None
 *********************************************************************/
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if(pSPIx == SPI1)
    {
        SPI1_REG_RESET();
    }
    else if(pSPIx == SPI2)
    {
        SPI2_REG_RESET();
    }
    else if(pSPIx == SPI3)
    {
        SPI3_REG_RESET();
    }
    else if(pSPIx == SPI4)
    {
        SPI4_REG_RESET();
    }
}


/*********************************************************************
 * @fn      		  - SPI_GetFlagStatus
 *
 * @brief             - Reads one flag from the status register
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - Flag mask, from the SPI_FLAG_xxx macros
 *
 * @return            - FLAG_SET or FLAG_RESET
 *********************************************************************/
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
    if(pSPIx->SR & FlagName)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}


/*********************************************************************
 * @fn      		  - SPI_PeripheralControl
 *
 * @brief             - Sets or clears the SPE bit, switching the SPI on or off
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Call this AFTER SPI_Init. Before disabling, wait for
 *                      the BSY flag to clear or the last frame is truncated.
 *********************************************************************/
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pSPIx->CR1 |= (1U << SPI_CR1_SPE);
    }
    else
    {
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE);
    }
}


/*********************************************************************
 * @fn      		  - SPI_SSIConfig
 *
 * @brief             - Drives the internal slave select level when SSM is on
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - ENABLE drives SSI high, DISABLE drives it low
 *
 * @return            - None
 *
 * @Note              - Only meaningful when SSM = 1. In master mode SSI must
 *                      be HIGH, otherwise the hardware reports a mode fault.
 *********************************************************************/
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pSPIx->CR1 |= (1U << SPI_CR1_SSI);
    }
    else
    {
        pSPIx->CR1 &= ~(1U << SPI_CR1_SSI);
    }
}


/*********************************************************************
 * @fn      		  - SPI_SSOEConfig
 *
 * @brief             - Enables the hardware NSS output
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Only used when SSM = 0. With SSOE set, the NSS pin is
 *                      driven low automatically while SPE is set. Most real
 *                      drivers ignore this and use a plain GPIO instead,
 *                      because software control gives precise timing around
 *                      the frame and scales to any number of slaves.
 *********************************************************************/
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pSPIx->CR2 |= (1U << SPI_CR2_SSOE);
    }
    else
    {
        pSPIx->CR2 &= ~(1U << SPI_CR2_SSOE);
    }
}


/*********************************************************************
 * @fn      		  - SPI_SendData
 *
 * @brief             - Blocking transmit of Len bytes
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - Address of the transmit buffer
 * @param[in]         - Number of BYTES to send
 *
 * @return            - None
 *
 * @Note              - This is a blocking call. Remember that SPI always
 *                      receives a byte for every byte it sends; this
 *                      function discards what comes back. If you need the
 *                      reply, use SPI_ReceiveData afterwards or the
 *                      interrupt based APIs.
 *                      In 16-bit mode Len must be EVEN - two bytes leave
 *                      per frame.
 *********************************************************************/
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
    while(Len > 0U)
    {
        /* Wait until the transmit buffer is free */
        while(SPI_GetFlagStatus(pSPIx, SPI_FLAG_TXE) == FLAG_RESET);

        if(pSPIx->CR1 & (1U << SPI_CR1_DFF))
        {
            /* 16-bit frame : load two bytes and consume two from the buffer */
            pSPIx->DR = *((uint16_t*)pTxBuffer);
            Len--;
            Len--;
            pTxBuffer += 2;
        }
        else
        {
            /* 8-bit frame */
            pSPIx->DR = *pTxBuffer;
            Len--;
            pTxBuffer++;
        }
    }
}


/*********************************************************************
 * @fn      		  - SPI_ReceiveData
 *
 * @brief             - Blocking receive of Len bytes
 *
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - Address of the receive buffer
 * @param[in]         - Number of BYTES to receive
 *
 * @return            - None
 *
 * @Note              - In MASTER mode the clock only runs while something is
 *                      being transmitted, so the caller must already have
 *                      queued dummy bytes, or use SPI_SendData with a dummy
 *                      buffer alongside this call. This is the practical
 *                      consequence of the shift register ring: you cannot
 *                      read without writing.
 *********************************************************************/
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
    while(Len > 0U)
    {
        /* Wait until a complete frame has arrived */
        while(SPI_GetFlagStatus(pSPIx, SPI_FLAG_RXNE) == FLAG_RESET);

        if(pSPIx->CR1 & (1U << SPI_CR1_DFF))
        {
            /* 16-bit frame : store two bytes */
            *((uint16_t*)pRxBuffer) = (uint16_t)pSPIx->DR;
            Len--;
            Len--;
            pRxBuffer += 2;
        }
        else
        {
            /* 8-bit frame */
            *pRxBuffer = (uint8_t)pSPIx->DR;
            Len--;
            pRxBuffer++;
        }
    }
}


/*********************************************************************
 * @fn      		  - SPI_SendDataIT
 *
 * @brief             - Starts an interrupt driven transmission
 *
 * @param[in]         - Pointer to SPI handle
 * @param[in]         - Address of the transmit buffer
 * @param[in]         - Number of bytes to send
 *
 * @return            - Previous Tx state (SPI_READY if the transfer started)
 *
 * @Note              - Returns immediately. Completion is reported through
 *                      SPI_ApplicationEventCallback with SPI_EVENT_TX_CMPLT.
 *********************************************************************/
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->TxState;

    if(state != SPI_BUSY_IN_TX)
    {
        pSPIHandle->pTxBuffer = pTxBuffer;
        pSPIHandle->TxLen     = Len;
        pSPIHandle->TxState   = SPI_BUSY_IN_TX;

        /* Enable the TXE interrupt - the ISR now feeds the peripheral */
        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_TXEIE);
    }

    return state;
}


/*********************************************************************
 * @fn      		  - SPI_ReceiveDataIT
 *
 * @brief             - Starts an interrupt driven reception
 *
 * @param[in]         - Pointer to SPI handle
 * @param[in]         - Address of the receive buffer
 * @param[in]         - Number of bytes to receive
 *
 * @return            - Previous Rx state (SPI_READY if the transfer started)
 *********************************************************************/
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->RxState;

    if(state != SPI_BUSY_IN_RX)
    {
        pSPIHandle->pRxBuffer = pRxBuffer;
        pSPIHandle->RxLen     = Len;
        pSPIHandle->RxState   = SPI_BUSY_IN_RX;

        /* Enable the RXNE interrupt */
        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_RXNEIE);
    }

    return state;
}


/*********************************************************************
 * @fn      		  - SPI_IRQConfig
 *
 * @brief             - Enables/disables the IRQ and sets its priority
 *
 * @param[in]         - IRQ number, use IRQ_NO_SPIx
 * @param[in]         - IRQ priority, 0 is the most urgent
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Identical in structure to GPIO_IRQConfig and
 *                      USART_IRQConfig; only the IRQ number differs
 *********************************************************************/
void SPI_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi)
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

    /*
     * Configure IRQ priority
     */
    uint8_t iprx         = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;

    uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
    *(NVIC_PR_BASE_ADDR + iprx) |=  (IRQPriority << shift_amount);
}


/*********************************************************************
 * @fn      		  - SPI_IRQHandling
 *
 * @brief             - Single entry point for every SPI interrupt source
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *
 * @Note              - Each source is checked against BOTH its status flag
 *                      and its enable bit, the same pattern used in
 *                      USART_IRQHandling
 *********************************************************************/
void SPI_IRQHandling(SPI_Handle_t *pSPIHandle)
{
    uint32_t temp1, temp2;

    /*************** Check for TXE (transmit buffer empty) ***************/

    temp1 = pSPIHandle->pSPIx->SR  & (1U << SPI_SR_TXE);
    temp2 = pSPIHandle->pSPIx->CR2 & (1U << SPI_CR2_TXEIE);

    if(temp1 && temp2)
    {
        spi_txe_interrupt_handle(pSPIHandle);
    }

    /*************** Check for RXNE (receive buffer not empty) ***************/

    temp1 = pSPIHandle->pSPIx->SR  & (1U << SPI_SR_RXNE);
    temp2 = pSPIHandle->pSPIx->CR2 & (1U << SPI_CR2_RXNEIE);

    if(temp1 && temp2)
    {
        spi_rxne_interrupt_handle(pSPIHandle);
    }

    /*************** Check for overrun ***************/

    temp1 = pSPIHandle->pSPIx->SR  & (1U << SPI_SR_OVR);
    temp2 = pSPIHandle->pSPIx->CR2 & (1U << SPI_CR2_ERRIE);

    if(temp1 && temp2)
    {
        spi_ovr_err_interrupt_handle(pSPIHandle);
    }
}


/*********************************************************************
 * @fn      		  - spi_txe_interrupt_handle
 *
 * @brief             - Feeds one frame into DR when the buffer goes empty
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *
 * @Note              - Local helper, called only from SPI_IRQHandling
 *********************************************************************/
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(pSPIHandle->pSPIx->CR1 & (1U << SPI_CR1_DFF))
    {
        /* 16-bit frame */
        pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
        pSPIHandle->TxLen--;
        pSPIHandle->TxLen--;
        pSPIHandle->pTxBuffer += 2;
    }
    else
    {
        /* 8-bit frame */
        pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
        pSPIHandle->TxLen--;
        pSPIHandle->pTxBuffer++;
    }

    if(pSPIHandle->TxLen == 0U)
    {
        /* Nothing left to feed - stop asking for TXE interrupts */
        SPI_CloseTransmission(pSPIHandle);

        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
    }
}


/*********************************************************************
 * @fn      		  - spi_rxne_interrupt_handle
 *
 * @brief             - Stores one frame from DR when a frame arrives
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *
 * @Note              - Local helper, called only from SPI_IRQHandling
 *********************************************************************/
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(pSPIHandle->pSPIx->CR1 & (1U << SPI_CR1_DFF))
    {
        /* 16-bit frame */
        *((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen--;
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer += 2;
    }
    else
    {
        /* 8-bit frame */
        *pSPIHandle->pRxBuffer = (uint8_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer++;
    }

    if(pSPIHandle->RxLen == 0U)
    {
        SPI_CloseReception(pSPIHandle);

        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
    }
}


/*********************************************************************
 * @fn      		  - spi_ovr_err_interrupt_handle
 *
 * @brief             - Clears the overrun flag and reports it
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *
 * @Note              - Local helper. The flag is only cleared if no
 *                      transmission is in progress, because clearing it
 *                      requires reading DR and that would consume a frame
 *                      the application still wants.
 *********************************************************************/
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
    {
        SPI_ClearOVRFlag(pSPIHandle->pSPIx);
    }

    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}


/*********************************************************************
 * @fn      		  - SPI_ClearOVRFlag
 *
 * @brief             - Clears the overrun flag
 *
 * @param[in]         - Base address of the SPI peripheral
 *
 * @return            - None
 *
 * @Note              - The overrun flag is cleared by a read of DR followed
 *                      by a read of SR. Both reads are required, and both
 *                      results are deliberately discarded.
 *********************************************************************/
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    volatile uint32_t temp;

    temp = pSPIx->DR;
    temp = pSPIx->SR;

    (void)temp;                                  // silence an unused-variable warning
}


/*********************************************************************
 * @fn      		  - SPI_CloseTransmission
 *
 * @brief             - Ends an interrupt driven transmission early
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *********************************************************************/
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE);

    pSPIHandle->pTxBuffer = NULL;
    pSPIHandle->TxLen     = 0U;
    pSPIHandle->TxState   = SPI_READY;
}


/*********************************************************************
 * @fn      		  - SPI_CloseReception
 *
 * @brief             - Ends an interrupt driven reception early
 *
 * @param[in]         - Pointer to SPI handle
 *
 * @return            - None
 *********************************************************************/
void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE);

    pSPIHandle->pRxBuffer = NULL;
    pSPIHandle->RxLen     = 0U;
    pSPIHandle->RxState   = SPI_READY;
}


/*********************************************************************
 * @fn      		  - SPI_ApplicationEventCallback
 *
 * @brief             - Weak default callback
 *
 * @param[in]         - Pointer to SPI handle
 * @param[in]         - Event code, from the SPI_EVENT_xxx macros
 *
 * @return            - None
 *
 * @Note              - The application should override this. Declared weak so
 *                      the project still links if it does not.
 *********************************************************************/
__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,
                                                        uint8_t AppEvent)
{
    (void)pSPIHandle;
    (void)AppEvent;

    /* Intentionally empty - override in the application */
}
