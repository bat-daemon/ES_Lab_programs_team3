/*
 * stm32f446xx_usart_driver.c
 *
 *  Created on: Aug 26, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_usart_driver.h"


/*
 * Oscillator values for this board.
 * HSI is the 16 MHz internal RC that runs after reset.
 * HSE on the NUCLEO-F446RE is 8 MHz supplied by the ST-LINK, not a crystal.
 */
#define HSI_VALUE       16000000U
#define HSE_VALUE       8000000U

/* AHB and APB prescaler lookup tables, indexed by the RCC_CFGR field */
static uint16_t AHB_PreScaler[8]  = {2, 4, 8, 16, 64, 128, 256, 512};
static uint8_t  APB_PreScaler[4]  = {2, 4, 8, 16};

/* Local helpers - not part of the public API */
static uint32_t RCC_GetPLLOutputClock(void);
static uint32_t RCC_GetPCLK1Value(void);
static uint32_t RCC_GetPCLK2Value(void);


/*********************************************************************
 * @fn      		  - RCC_GetPLLOutputClock
 *
 * @brief             - Computes the PLL output frequency from PLLCFGR
 *
 * @return            - PLL output clock in Hz
 *
 * @Note              - VCO   = source / PLLM * PLLN
 *                      SYSCLK = VCO / PLLP
 *********************************************************************/
static uint32_t RCC_GetPLLOutputClock(void)
{
    uint32_t pllsource, pllm, plln, pllp, vco;

    pllm = (RCC->PLLCFGR >> 0) & 0x3FU;         // Bits [5:0]
    plln = (RCC->PLLCFGR >> 6) & 0x1FFU;        // Bits [14:6]
    pllp = (((RCC->PLLCFGR >> 16) & 0x3U) + 1U) * 2U;   // Bits [17:16] -> 2,4,6,8

    pllsource = (RCC->PLLCFGR >> 22) & 0x1U;    // Bit 22 : 0 = HSI, 1 = HSE

    if(pllm == 0U)
    {
        return HSI_VALUE;                        // guard against divide by zero
    }

    vco = ((pllsource ? HSE_VALUE : HSI_VALUE) / pllm) * plln;

    return (vco / pllp);
}


/*********************************************************************
 * @fn      		  - RCC_GetPCLK1Value
 *
 * @brief             - Returns the APB1 peripheral clock frequency
 *
 * @return            - PCLK1 in Hz
 *
 * @Note              - APB1 is limited to 45 MHz on the STM32F446
 *********************************************************************/
static uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t pclk1, SystemClk;
    uint8_t clksrc, temp, ahbp, apb1p;

    /* Which clock is driving the system? RCC_CFGR SWS bits [3:2] */
    clksrc = (uint8_t)((RCC->CFGR >> 2) & 0x3U);

    if(clksrc == 0U)
    {
        SystemClk = HSI_VALUE;                   // HSI selected
    }
    else if(clksrc == 1U)
    {
        SystemClk = HSE_VALUE;                   // HSE selected
    }
    else
    {
        SystemClk = RCC_GetPLLOutputClock();     // PLL selected
    }

    /* AHB prescaler : RCC_CFGR HPRE bits [7:4] */
    temp = (uint8_t)((RCC->CFGR >> 4) & 0xFU);
    ahbp = (temp < 8U) ? 1U : AHB_PreScaler[temp - 8U];

    /* APB1 prescaler : RCC_CFGR PPRE1 bits [12:10] */
    temp  = (uint8_t)((RCC->CFGR >> 10) & 0x7U);
    apb1p = (temp < 4U) ? 1U : APB_PreScaler[temp - 4U];

    pclk1 = (SystemClk / ahbp) / apb1p;

    return pclk1;
}


/*********************************************************************
 * @fn      		  - RCC_GetPCLK2Value
 *
 * @brief             - Returns the APB2 peripheral clock frequency
 *
 * @return            - PCLK2 in Hz
 *
 * @Note              - APB2 is limited to 90 MHz on the STM32F446, which is
 *                      why USART1 and USART6 reach higher baud rates
 *********************************************************************/
static uint32_t RCC_GetPCLK2Value(void)
{
    uint32_t pclk2, SystemClk;
    uint8_t clksrc, temp, ahbp, apb2p;

    clksrc = (uint8_t)((RCC->CFGR >> 2) & 0x3U);

    if(clksrc == 0U)
    {
        SystemClk = HSI_VALUE;
    }
    else if(clksrc == 1U)
    {
        SystemClk = HSE_VALUE;
    }
    else
    {
        SystemClk = RCC_GetPLLOutputClock();
    }

    temp = (uint8_t)((RCC->CFGR >> 4) & 0xFU);
    ahbp = (temp < 8U) ? 1U : AHB_PreScaler[temp - 8U];

    /* APB2 prescaler : RCC_CFGR PPRE2 bits [15:13] */
    temp  = (uint8_t)((RCC->CFGR >> 13) & 0x7U);
    apb2p = (temp < 4U) ? 1U : APB_PreScaler[temp - 4U];

    pclk2 = (SystemClk / ahbp) / apb2p;

    return pclk2;
}


/*********************************************************************
 * @fn      		  - USART_PeriClockControl
 *
 * @brief             - Enables or disables peripheral clock for the USART
 *
 * @param[in]         - Base address of the USART peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *********************************************************************/
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pUSARTx == USART1)
        {
            USART1_PCLK_EN();
        }
        else if(pUSARTx == USART2)
        {
            USART2_PCLK_EN();
        }
        else if(pUSARTx == USART3)
        {
            USART3_PCLK_EN();
        }
        else if(pUSARTx == UART4)
        {
            UART4_PCLK_EN();
        }
        else if(pUSARTx == UART5)
        {
            UART5_PCLK_EN();
        }
        else if(pUSARTx == USART6)
        {
            USART6_PCLK_EN();
        }
    }
    else
    {
        if(pUSARTx == USART1)
        {
            USART1_PCLK_DI();
        }
        else if(pUSARTx == USART2)
        {
            USART2_PCLK_DI();
        }
        else if(pUSARTx == USART3)
        {
            USART3_PCLK_DI();
        }
        else if(pUSARTx == UART4)
        {
            UART4_PCLK_DI();
        }
        else if(pUSARTx == UART5)
        {
            UART5_PCLK_DI();
        }
        else if(pUSARTx == USART6)
        {
            USART6_PCLK_DI();
        }
    }
}


/*********************************************************************
 * @fn      		  - USART_SetBaudRate
 *
 * @brief             - Computes and writes the BRR register
 *
 * @param[in]         - Base address of the USART peripheral
 * @param[in]         - Desired baud rate
 *
 * @return            - None
 *
 * @Note              - USARTDIV = fCK / (8 x (2 - OVER8) x BaudRate)
 *                      The value is held as a 12-bit mantissa and a 4-bit
 *                      fraction. Everything below is done in integer maths,
 *                      scaled by 100, to avoid using floating point.
 *********************************************************************/
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate)
{
    uint32_t PCLKx;
    uint32_t usartdiv;
    uint32_t M_part, F_part;
    uint32_t tempreg = 0;

    /* USART1 and USART6 are on APB2, everything else on APB1 */
    if((pUSARTx == USART1) || (pUSARTx == USART6))
    {
        PCLKx = RCC_GetPCLK2Value();
    }
    else
    {
        PCLKx = RCC_GetPCLK1Value();
    }

    /* usartdiv is scaled by 100 so the fraction survives integer division */
    if(pUSARTx->CR1 & (1U << USART_CR1_OVER8))
    {
        /* OVER8 = 1 : oversampling by 8 */
        usartdiv = ((25U * PCLKx) / (2U * BaudRate));
    }
    else
    {
        /* OVER8 = 0 : oversampling by 16 */
        usartdiv = ((25U * PCLKx) / (4U * BaudRate));
    }

    /* Mantissa is the whole part */
    M_part = usartdiv / 100U;
    tempreg |= (M_part << USART_BRR_DIV_MANTISSA);

    /* Fraction is what remains, converted into the 3 or 4 bit field.
     * The +50 performs rounding to nearest rather than truncation. */
    F_part = (usartdiv - (M_part * 100U));

    if(pUSARTx->CR1 & (1U << USART_CR1_OVER8))
    {
        F_part = (((F_part * 8U) + 50U) / 100U) & 0x07U;
    }
    else
    {
        F_part = (((F_part * 16U) + 50U) / 100U) & 0x0FU;
    }

    tempreg |= F_part;

    pUSARTx->BRR = tempreg;
}


/*********************************************************************
 * @fn      		  - USART_Init
 *
 * @brief             - Initialises a USART peripheral from its handle
 *
 * @param[in]         - Pointer to USART handle
 *
 * @return            - None
 *
 * @Note              - The peripheral clock must already be enabled
 *********************************************************************/
void USART_Init(USART_Handle_t *pUSARTHandle)
{
    uint32_t tempreg = 0;

    /* Enable the clock for the given USART peripheral */
    USART_PeriClockControl(pUSARTHandle->pUSARTx, ENABLE);


    /******************** Configuration of CR1 ********************/

    /*
     * 1. Enable the transmitter, the receiver, or both
     */
    if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX)
    {
        tempreg |= (1U << USART_CR1_RE);
    }
    else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX)
    {
        tempreg |= (1U << USART_CR1_TE);
    }
    else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX)
    {
        tempreg |= ((1U << USART_CR1_RE) | (1U << USART_CR1_TE));
    }

    /*
     * 2. Word length
     */
    tempreg |= (pUSARTHandle->USART_Config.USART_WordLength << USART_CR1_M);

    /*
     * 3. Parity control
     */
    if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_EVEN)
    {
        tempreg |= (1U << USART_CR1_PCE);        // PS stays 0 for even parity
    }
    else if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_ODD)
    {
        tempreg |= (1U << USART_CR1_PCE);
        tempreg |= (1U << USART_CR1_PS);
    }

    pUSARTHandle->pUSARTx->CR1 = tempreg;


    /******************** Configuration of CR2 ********************/

    tempreg = 0;

    /*
     * 4. Number of stop bits
     */
    tempreg |= (pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP);

    pUSARTHandle->pUSARTx->CR2 = tempreg;


    /******************** Configuration of CR3 ********************/

    tempreg = 0;

    /*
     * 5. Hardware flow control
     */
    if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS)
    {
        tempreg |= (1U << USART_CR3_CTSE);
    }
    else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS)
    {
        tempreg |= (1U << USART_CR3_RTSE);
    }
    else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
    {
        tempreg |= ((1U << USART_CR3_CTSE) | (1U << USART_CR3_RTSE));
    }

    pUSARTHandle->pUSARTx->CR3 = tempreg;


    /******************** Baud rate ********************/

    /*
     * 6. Program the BRR register.
     *    Done last because it reads the OVER8 bit set up in CR1 above.
     */
    USART_SetBaudRate(pUSARTHandle->pUSARTx,
                      pUSARTHandle->USART_Config.USART_Baud);

    /* Driver starts idle */
    pUSARTHandle->TxBusyState = USART_READY;
    pUSARTHandle->RxBusyState = USART_READY;
}


/*********************************************************************
 * @fn      		  - USART_DeInit
 *
 * @brief             - Resets the USART peripheral registers
 *
 * @param[in]         - Base address of the USART peripheral
 *
 * @return            - None
 *********************************************************************/
void USART_DeInit(USART_RegDef_t *pUSARTx)
{
    if(pUSARTx == USART1)
    {
        USART1_REG_RESET();
    }
    else if(pUSARTx == USART2)
    {
        USART2_REG_RESET();
    }
    else if(pUSARTx == USART3)
    {
        USART3_REG_RESET();
    }
    else if(pUSARTx == UART4)
    {
        UART4_REG_RESET();
    }
    else if(pUSARTx == UART5)
    {
        UART5_REG_RESET();
    }
    else if(pUSARTx == USART6)
    {
        USART6_REG_RESET();
    }
}


/*********************************************************************
 * @fn      		  - USART_PeripheralControl
 *
 * @brief             - Sets or clears the UE bit, switching the USART on or off
 *
 * @return            - None
 *
 * @Note              - Call this AFTER USART_Init, otherwise the configuration
 *                      is written while the peripheral is already running
 *********************************************************************/
void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pUSARTx->CR1 |= (1U << USART_CR1_UE);
    }
    else
    {
        pUSARTx->CR1 &= ~(1U << USART_CR1_UE);
    }
}


/*********************************************************************
 * @fn      		  - USART_GetFlagStatus
 *
 * @brief             - Reads one flag from the status register
 *
 * @return            - FLAG_SET or FLAG_RESET
 *********************************************************************/
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName)
{
    if(pUSARTx->SR & FlagName)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}


/*********************************************************************
 * @fn      		  - USART_ClearFlag
 *
 * @brief             - Clears a status flag
 *
 * @return            - None
 *
 * @Note              - Only applicable to flags the software may clear
 *                      directly, such as TC and LBD. RXNE, TXE and the error
 *                      flags are cleared by the hardware sequence of reading
 *                      SR then reading or writing DR.
 *********************************************************************/
void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint32_t FlagName)
{
    pUSARTx->SR &= ~(FlagName);
}


/*********************************************************************
 * @fn      		  - USART_SendData
 *
 * @brief             - Blocking transmit of Len bytes
 *
 * @param[in]         - Pointer to USART handle
 * @param[in]         - Address of the transmit buffer
 * @param[in]         - Number of bytes to send
 *
 * @return            - None
 *
 * @Note              - This is a blocking call: it returns only once the last
 *                      byte has physically left the shift register
 *********************************************************************/
void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint16_t *pdata;
    uint32_t i;

    for(i = 0; i < Len; i++)
    {
        /* Wait until the transmit data register is free */
        while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TXE) == FLAG_RESET);

        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            /* 9-bit word: load two bytes and mask to 9 bits */
            pdata = (uint16_t*)pTxBuffer;
            pUSARTHandle->pUSARTx->DR = (*pdata & (uint16_t)0x01FF);

            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                /* All 9 bits are data, so two bytes were consumed */
                pTxBuffer += 2;
            }
            else
            {
                /* The 9th bit is the parity bit inserted by hardware */
                pTxBuffer++;
            }
        }
        else
        {
            /* 8-bit word */
            pUSARTHandle->pUSARTx->DR = (*pTxBuffer & (uint8_t)0xFF);
            pTxBuffer++;
        }
    }

    /* Wait for the final byte to be shifted out completely */
    while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TC) == FLAG_RESET);
}


/*********************************************************************
 * @fn      		  - USART_ReceiveData
 *
 * @brief             - Blocking receive of Len bytes
 *
 * @return            - None
 *********************************************************************/
void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint32_t i;

    for(i = 0; i < Len; i++)
    {
        /* Wait until a byte has arrived */
        while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_RXNE) == FLAG_RESET);

        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                /* All 9 bits are data */
                *((uint16_t*)pRxBuffer) = (uint16_t)(pUSARTHandle->pUSARTx->DR & (uint16_t)0x01FF);
                pRxBuffer += 2;
            }
            else
            {
                /* 8 data bits, the 9th was parity and is discarded */
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
                pRxBuffer++;
            }
        }
        else
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                /* All 8 bits are data */
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
            }
            else
            {
                /* 7 data bits, the 8th was parity */
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0x7F);
            }

            pRxBuffer++;
        }
    }
}


/*********************************************************************
 * @fn      		  - USART_SendDataIT
 *
 * @brief             - Starts an interrupt driven transmission
 *
 * @return            - Previous Tx state (USART_READY if the transfer started)
 *
 * @Note              - Returns immediately. Completion is reported through
 *                      USART_ApplicationEventCallback with USART_EVENT_TX_CMPLT
 *********************************************************************/
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t txstate = pUSARTHandle->TxBusyState;

    if(txstate != USART_BUSY_IN_TX)
    {
        pUSARTHandle->TxLen       = Len;
        pUSARTHandle->pTxBuffer   = pTxBuffer;
        pUSARTHandle->TxBusyState = USART_BUSY_IN_TX;

        /* Enable the TXE interrupt - the ISR now feeds the peripheral */
        pUSARTHandle->pUSARTx->CR1 |= (1U << USART_CR1_TXEIE);

        /* Enable the TC interrupt - fires once the last byte is gone */
        pUSARTHandle->pUSARTx->CR1 |= (1U << USART_CR1_TCIE);
    }

    return txstate;
}


/*********************************************************************
 * @fn      		  - USART_ReceiveDataIT
 *
 * @brief             - Starts an interrupt driven reception
 *
 * @return            - Previous Rx state (USART_READY if the transfer started)
 *********************************************************************/
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t rxstate = pUSARTHandle->RxBusyState;

    if(rxstate != USART_BUSY_IN_RX)
    {
        pUSARTHandle->RxLen       = Len;
        pUSARTHandle->pRxBuffer   = pRxBuffer;
        pUSARTHandle->RxBusyState = USART_BUSY_IN_RX;

        (void)pUSARTHandle->pUSARTx->DR;         // dummy read clears a stale RXNE

        /* Enable the RXNE interrupt */
        pUSARTHandle->pUSARTx->CR1 |= (1U << USART_CR1_RXNEIE);
    }

    return rxstate;
}


/*********************************************************************
 * @fn      		  - USART_IRQConfig
 *
 * @brief             - Enables/disables the IRQ and sets its priority
 *
 * @return            - None
 *
 * @Note              - Identical in structure to GPIO_IRQConfig; only the
 *                      IRQ number differs
 *********************************************************************/
void USART_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi)
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
 * @fn      		  - USART_IRQHandling
 *
 * @brief             - Single entry point for all USART interrupt sources
 *
 * @param[in]         - Pointer to USART handle
 *
 * @return            - None
 *
 * @Note              - Every USART event shares one vector, so the handler
 *                      must test each flag against its own enable bit
 *********************************************************************/
void USART_IRQHandling(USART_Handle_t *pUSARTHandle)
{
    uint32_t temp1, temp2;
    uint16_t *pdata;

    /*************** Check for TC (transmission complete) ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_TC);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1U << USART_CR1_TCIE);

    if(temp1 && temp2)
    {
        /* Only finish up if every byte has been handed over */
        if(pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
        {
            if(pUSARTHandle->TxLen == 0U)
            {
                /* Clear TC by reading SR then writing SR */
                pUSARTHandle->pUSARTx->SR &= ~(1U << USART_SR_TC);

                pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TCIE);

                pUSARTHandle->TxBusyState = USART_READY;
                pUSARTHandle->pTxBuffer   = NULL;
                pUSARTHandle->TxLen       = 0U;

                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_TX_CMPLT);
            }
        }
    }


    /*************** Check for TXE (transmit register empty) ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_TXE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1U << USART_CR1_TXEIE);

    if(temp1 && temp2)
    {
        if(pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
        {
            if(pUSARTHandle->TxLen > 0U)
            {
                if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
                {
                    pdata = (uint16_t*)pUSARTHandle->pTxBuffer;
                    pUSARTHandle->pUSARTx->DR = (*pdata & (uint16_t)0x01FF);

                    if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
                    {
                        pUSARTHandle->pTxBuffer += 2;
                        pUSARTHandle->TxLen     -= 2;
                    }
                    else
                    {
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->TxLen--;
                    }
                }
                else
                {
                    pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer & (uint8_t)0xFF);
                    pUSARTHandle->pTxBuffer++;
                    pUSARTHandle->TxLen--;
                }
            }

            if(pUSARTHandle->TxLen == 0U)
            {
                /* Nothing left to feed - stop asking for TXE interrupts */
                pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TXEIE);
            }
        }
    }


    /*************** Check for RXNE (data received) ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_RXNE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1U << USART_CR1_RXNEIE);

    if(temp1 && temp2)
    {
        if(pUSARTHandle->RxBusyState == USART_BUSY_IN_RX)
        {
            if(pUSARTHandle->RxLen > 0U)
            {
                if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
                {
                    if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
                    {
                        *((uint16_t*)pUSARTHandle->pRxBuffer) =
                                (uint16_t)(pUSARTHandle->pUSARTx->DR & (uint16_t)0x01FF);

                        pUSARTHandle->pRxBuffer += 2;
                        pUSARTHandle->RxLen     -= 2;
                    }
                    else
                    {
                        *pUSARTHandle->pRxBuffer =
                                (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);

                        pUSARTHandle->pRxBuffer++;
                        pUSARTHandle->RxLen--;
                    }
                }
                else
                {
                    if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
                    {
                        *pUSARTHandle->pRxBuffer =
                                (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
                    }
                    else
                    {
                        *pUSARTHandle->pRxBuffer =
                                (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0x7F);
                    }

                    pUSARTHandle->pRxBuffer++;
                    pUSARTHandle->RxLen--;
                }
            }

            if(pUSARTHandle->RxLen == 0U)
            {
                pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_RXNEIE);
                pUSARTHandle->RxBusyState = USART_READY;

                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_RX_CMPLT);
            }
        }
    }


    /*************** Check for CTS (flow control line changed) ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_CTS);
    temp2 = pUSARTHandle->pUSARTx->CR3 & (1U << USART_CR3_CTSIE);

    if(temp1 && temp2)
    {
        pUSARTHandle->pUSARTx->SR &= ~(1U << USART_SR_CTS);

        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_CTS);
    }


    /*************** Check for IDLE line detection ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_IDLE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1U << USART_CR1_IDLEIE);

    if(temp1 && temp2)
    {
        /* IDLE is cleared by reading SR then reading DR */
        (void)pUSARTHandle->pUSARTx->SR;
        (void)pUSARTHandle->pUSARTx->DR;

        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_IDLE);
    }


    /*************** Check for Overrun error ***************/

    temp1 = pUSARTHandle->pUSARTx->SR  & (1U << USART_SR_ORE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1U << USART_CR1_RXNEIE);

    if(temp1 && temp2)
    {
        /* Left for the application to clear, since clearing ORE also
         * discards the byte sitting in DR */
        USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
    }


    /*************** Check for framing, noise and parity errors ***************/

    temp2 = pUSARTHandle->pUSARTx->CR3 & (1U << USART_CR3_EIE);

    if(temp2)
    {
        temp1 = pUSARTHandle->pUSARTx->SR;

        if(temp1 & (1U << USART_SR_FE))
        {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_FE);
        }

        if(temp1 & (1U << USART_SR_NE))
        {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_NE);
        }

        if(temp1 & (1U << USART_SR_ORE))
        {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
        }
    }
}


/*********************************************************************
 * @fn      		  - USART_ApplicationEventCallback
 *
 * @brief             - Weak default callback
 *
 * @return            - None
 *
 * @Note              - The application should override this. Declared weak so
 *                      the project still links if it does not.
 *********************************************************************/
__attribute__((weak)) void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,
                                                          uint8_t AppEvent)
{
    (void)pUSARTHandle;
    (void)AppEvent;

    /* Intentionally empty - override in the application */
}
