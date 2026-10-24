/*
 * stm32f446xx_adc_driver.c
 *
 *  Created on: Aug 28, 2026
 *      Author: raghudathesh.gp
 */

#include "stm32f446xx_adc_driver.h"


/*********************************************************************
 * @fn      		  - ADC_PeriClockControl
 *
 * @brief             - Enables or disables peripheral clock for the ADC
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - All three ADCs sit on APB2, so the enable bits are in
 *                      RCC_APB2ENR at positions 8, 9 and 10
 *********************************************************************/
void ADC_PeriClockControl(ADC_RegDef_t *pADCx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pADCx == ADC1)
        {
            ADC1_PCLK_EN();
        }
        else if(pADCx == ADC2)
        {
            ADC2_PCLK_EN();
        }
        else if(pADCx == ADC3)
        {
            ADC3_PCLK_EN();
        }
    }
    else
    {
        if(pADCx == ADC1)
        {
            ADC1_PCLK_DI();
        }
        else if(pADCx == ADC2)
        {
            ADC2_PCLK_DI();
        }
        else if(pADCx == ADC3)
        {
            ADC3_PCLK_DI();
        }
    }
}


/*********************************************************************
 * @fn      		  - ADC_Init
 *
 * @brief             - Initialises an ADC peripheral from its handle
 *
 * @param[in]         - Pointer to ADC handle
 *
 * @return            - None
 *
 * @Note              - Does NOT set ADON. Call ADC_PeripheralControl
 *                      afterwards, in the same way USART_Init leaves UE alone.
 *********************************************************************/
void ADC_Init(ADC_Handle_t *pADCHandle)
{
    uint32_t tempreg = 0;

    /* Enable the clock for the given ADC peripheral */
    ADC_PeriClockControl(pADCHandle->pADCx, ENABLE);


    /******************** Common register : clock prescaler ********************/

    /*
     * 1. ADCCLK = PCLK2 / prescaler. This lives in the SHARED common block,
     *    so it applies to all three ADCs at once.
     */
    ADC->CCR &= ~(0x3U << ADC_CCR_ADCPRE);                   // clear the field first

    ADC->CCR |= (pADCHandle->ADC_Config.ADC_ClkPreScaler
                 << ADC_CCR_ADCPRE);                          // then write the new divider


    /******************** Configuration of CR1 ********************/

    tempreg = 0;

    /*
     * 2. Resolution
     */
    tempreg |= (pADCHandle->ADC_Config.ADC_Resolution << ADC_CR1_RES);

    /*
     * 3. Scan mode - convert the whole sequence, or only the first entry
     */
    if(pADCHandle->ADC_Config.ADC_ScanMode == ADC_SCAN_ENABLE)
    {
        tempreg |= (1U << ADC_CR1_SCAN);
    }

    pADCHandle->pADCx->CR1 = tempreg;


    /******************** Configuration of CR2 ********************/

    tempreg = 0;

    /*
     * 4. Data alignment
     */
    tempreg |= (pADCHandle->ADC_Config.ADC_DataAlignment << ADC_CR2_ALIGN);

    /*
     * 5. Continuous or single conversion
     */
    if(pADCHandle->ADC_Config.ADC_ConvMode == ADC_CONV_CONTINUOUS)
    {
        tempreg |= (1U << ADC_CR2_CONT);
    }

    pADCHandle->pADCx->CR2 = tempreg;


    /******************** Sequence length ********************/

    /*
     * 6. SQR1 L field holds the number of conversions MINUS ONE.
     *    A single conversion therefore means writing zero.
     */
    ADC_SetSequenceLength(pADCHandle->pADCx,
                          pADCHandle->ADC_Config.ADC_NumOfConversion);

    /* Driver starts idle */
    pADCHandle->RxBusyState = ADC_READY;
}


/*********************************************************************
 * @fn      		  - ADC_DeInit
 *
 * @brief             - Resets the ADC peripheral registers
 *
 * @param[in]         - None
 *
 * @return            - None
 *
 * @Note              - Takes no argument on purpose. APB2RSTR bit 8 is a
 *                      single ADCRST bit that resets ALL THREE ADCs together;
 *                      the hardware offers no way to reset just one.
 *********************************************************************/
void ADC_DeInit(void)
{
    ADC_REG_RESET();
}


/*********************************************************************
 * @fn      		  - ADC_ConfigChannel
 *
 * @brief             - Sets the sampling time for one channel and places
 *                      that channel first in the regular sequence
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - Channel number, from @ADC_CHANNELS
 * @param[in]         - Sampling time, from @ADC_SAMPLE_TIME
 *
 * @return            - None
 *
 * @Note              - Each channel owns a 3-bit field. SMPR2 covers channels
 *                      0 to 9, SMPR1 covers channels 10 to 18.
 *********************************************************************/
void ADC_ConfigChannel(ADC_RegDef_t *pADCx, uint8_t Channel, uint8_t SamplingTime)
{
    uint8_t shift;

    if(Channel <= 9U)
    {
        /* Channels 0 to 9 live in SMPR2, field starts at 3 x channel */
        shift = (uint8_t)(Channel * ADC_SMPR_FIELD_WIDTH);

        pADCx->SMPR2 &= ~(0x7U << shift);                     // clear the old field
        pADCx->SMPR2 |=  ((uint32_t)SamplingTime << shift);   // write the new one
    }
    else
    {
        /* Channels 10 to 18 live in SMPR1, counting restarts from channel 10 */
        shift = (uint8_t)((Channel - 10U) * ADC_SMPR_FIELD_WIDTH);

        pADCx->SMPR1 &= ~(0x7U << shift);                     // clear the old field
        pADCx->SMPR1 |=  ((uint32_t)SamplingTime << shift);   // write the new one
    }

    /* Put this channel in rank 1, which is SQR3 bits [4:0] */
    ADC_SetSequenceRank(pADCx, Channel, 1U);
}


/*********************************************************************
 * @fn      		  - ADC_SetSequenceLength
 *
 * @brief             - Writes how many conversions the regular sequence holds
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - Number of conversions, 1 to 16
 *
 * @return            - None
 *
 * @Note              - The hardware field stores the count MINUS ONE
 *********************************************************************/
void ADC_SetSequenceLength(ADC_RegDef_t *pADCx, uint8_t Length)
{
    if((Length == 0U) || (Length > 16U))
    {
        Length = 1U;                                          // guard against an illegal value
    }

    pADCx->SQR1 &= ~(0xFU << ADC_SQR1_L);                     // clear the L field

    pADCx->SQR1 |= ((uint32_t)(Length - 1U) << ADC_SQR1_L);   // store count minus one
}


/*********************************************************************
 * @fn      		  - ADC_SetSequenceRank
 *
 * @brief             - Places a channel at a given position in the sequence
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - Channel number, from @ADC_CHANNELS
 * @param[in]         - Rank, 1 to 16, where 1 converts first
 *
 * @return            - None
 *
 * @Note              - Ranks 1 to 6 live in SQR3, 7 to 12 in SQR2 and
 *                      13 to 16 in SQR1. The FIRST conversions are in SQR3,
 *                      so the register numbering runs backwards from the
 *                      order in which conversions actually happen.
 *********************************************************************/
void ADC_SetSequenceRank(ADC_RegDef_t *pADCx, uint8_t Channel, uint8_t Rank)
{
    uint8_t shift;

    if((Rank == 0U) || (Rank > 16U))
    {
        return;                                               // illegal rank, do nothing
    }

    if(Rank <= 6U)
    {
        /* SQR3 holds ranks 1 to 6 */
        shift = (uint8_t)((Rank - 1U) * ADC_SQR_FIELD_WIDTH);

        pADCx->SQR3 &= ~(0x1FU << shift);
        pADCx->SQR3 |=  ((uint32_t)Channel << shift);
    }
    else if(Rank <= 12U)
    {
        /* SQR2 holds ranks 7 to 12 */
        shift = (uint8_t)((Rank - 7U) * ADC_SQR_FIELD_WIDTH);

        pADCx->SQR2 &= ~(0x1FU << shift);
        pADCx->SQR2 |=  ((uint32_t)Channel << shift);
    }
    else
    {
        /* SQR1 holds ranks 13 to 16, below the L field */
        shift = (uint8_t)((Rank - 13U) * ADC_SQR_FIELD_WIDTH);

        pADCx->SQR1 &= ~(0x1FU << shift);
        pADCx->SQR1 |=  ((uint32_t)Channel << shift);
    }
}


/*********************************************************************
 * @fn      		  - ADC_TempSensorVrefintControl
 *
 * @brief             - Enables or disables the internal temperature sensor
 *                      and the internal reference voltage channels
 *
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Both share one enable bit in the common CCR. These
 *                      channels are on ADC1 only and need a long sampling
 *                      time, because the internal source impedance is high.
 *********************************************************************/
void ADC_TempSensorVrefintControl(uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        ADC->CCR |= (1U << ADC_CCR_TSVREFE);
    }
    else
    {
        ADC->CCR &= ~(1U << ADC_CCR_TSVREFE);
    }
}


/*********************************************************************
 * @fn      		  - ADC_VbatControl
 *
 * @brief             - Enables or disables the battery voltage channel
 *
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - VBAT shares channel 18 with the temperature sensor on
 *                      this device, so enabling both at once is meaningless -
 *                      the VBAT path takes priority.
 *********************************************************************/
void ADC_VbatControl(uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        ADC->CCR |= (1U << ADC_CCR_VBATE);
    }
    else
    {
        ADC->CCR &= ~(1U << ADC_CCR_VBATE);
    }
}


/*********************************************************************
 * @fn      		  - ADC_PeripheralControl
 *
 * @brief             - Sets or clears the ADON bit, powering the ADC on or off
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - ADON does not take effect instantly. The analog block
 *                      needs a stabilisation time, so a short delay is
 *                      inserted here. Starting a conversion too soon gives
 *                      garbage on the first few reads.
 *********************************************************************/
void ADC_PeripheralControl(ADC_RegDef_t *pADCx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pADCx->CR2 |= (1U << ADC_CR2_ADON);

        /* Let the analog block settle before anything is converted */
        for(volatile uint32_t i = 0; i < 10000U; i++);
    }
    else
    {
        pADCx->CR2 &= ~(1U << ADC_CR2_ADON);
    }
}


/*********************************************************************
 * @fn      		  - ADC_StartConversion
 *
 * @brief             - Triggers a regular conversion in software
 *
 * @param[in]         - Base address of the ADC peripheral
 *
 * @return            - None
 *
 * @Note              - SWSTART is bit 30 of CR2. In continuous mode this only
 *                      needs to be called once; the hardware restarts itself.
 *********************************************************************/
void ADC_StartConversion(ADC_RegDef_t *pADCx)
{
    pADCx->CR2 |= (1U << ADC_CR2_SWSTART);
}


/*********************************************************************
 * @fn      		  - ADC_GetConversionValue
 *
 * @brief             - Reads the regular data register
 *
 * @param[in]         - Base address of the ADC peripheral
 *
 * @return            - The 12-bit result, right aligned
 *
 * @Note              - Reading DR clears the EOC flag automatically. That is
 *                      why there is no separate ADC_ClearFlag call for EOC in
 *                      the normal read path.
 *********************************************************************/
uint16_t ADC_GetConversionValue(ADC_RegDef_t *pADCx)
{
    return (uint16_t)(pADCx->DR & 0xFFFFU);
}


/*********************************************************************
 * @fn      		  - ADC_GetFlagStatus
 *
 * @brief             - Reads one flag from the status register
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - Flag mask, from the ADC_FLAG_xxx macros
 *
 * @return            - FLAG_SET or FLAG_RESET
 *********************************************************************/
uint8_t ADC_GetFlagStatus(ADC_RegDef_t *pADCx, uint32_t FlagName)
{
    if(pADCx->SR & FlagName)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}


/*********************************************************************
 * @fn      		  - ADC_ClearFlag
 *
 * @brief             - Clears a status flag
 *
 * @param[in]         - Base address of the ADC peripheral
 * @param[in]         - Flag mask, from the ADC_FLAG_xxx macros
 *
 * @return            - None
 *
 * @Note              - ADC status flags are cleared by writing ZERO to them,
 *                      unlike EXTI_PR where you write a one.
 *********************************************************************/
void ADC_ClearFlag(ADC_RegDef_t *pADCx, uint32_t FlagName)
{
    pADCx->SR &= ~(FlagName);
}


/*********************************************************************
 * @fn      		  - ADC_ReadChannel
 *
 * @brief             - Blocking single-shot read of one channel
 *
 * @param[in]         - Pointer to ADC handle
 * @param[in]         - Channel number, from @ADC_CHANNELS
 *
 * @return            - The raw conversion result
 *
 * @Note              - This is a blocking call: it spins until EOC is set.
 *                      The GPIO pin must already be in GPIO_MODE_ANALOG with
 *                      GPIO_NO_PUPD, and ADC_PeripheralControl must have been
 *                      called to set ADON.
 *********************************************************************/
uint16_t ADC_ReadChannel(ADC_Handle_t *pADCHandle, uint8_t Channel)
{
    /* Select the channel and give it the configured sampling time */
    ADC_ConfigChannel(pADCHandle->pADCx,
                      Channel,
                      pADCHandle->ADC_Config.ADC_SamplingTime);

    /* Begin the conversion */
    ADC_StartConversion(pADCHandle->pADCx);

    /* Spin until the hardware says the result is ready */
    while(ADC_GetFlagStatus(pADCHandle->pADCx, ADC_FLAG_EOC) == FLAG_RESET);

    /* Reading DR returns the result and clears EOC in one step */
    return ADC_GetConversionValue(pADCHandle->pADCx);
}


/*********************************************************************
 * @fn      		  - ADC_ReadChannelIT
 *
 * @brief             - Starts an interrupt driven collection of Len results
 *
 * @param[in]         - Pointer to ADC handle
 * @param[in]         - Address of the result buffer, uint16_t not uint8_t
 * @param[in]         - How many conversions to collect
 *
 * @return            - Previous state (ADC_READY if the transfer started)
 *
 * @Note              - Returns immediately. Completion is reported through
 *                      ADC_ApplicationEventCallback with ADC_EVENT_CONV_CMPLT.
 *                      The channel must already have been selected with
 *                      ADC_ConfigChannel.
 *********************************************************************/
uint8_t ADC_ReadChannelIT(ADC_Handle_t *pADCHandle, uint16_t *pRxBuffer, uint32_t Len)
{
    uint8_t state = pADCHandle->RxBusyState;

    if(state != ADC_BUSY_IN_CONVERSION)
    {
        pADCHandle->pRxBuffer   = pRxBuffer;
        pADCHandle->RxLen       = Len;
        pADCHandle->RxBusyState = ADC_BUSY_IN_CONVERSION;

        /* Ask for an interrupt each time a conversion finishes */
        pADCHandle->pADCx->CR1 |= (1U << ADC_CR1_EOCIE);

        /* Also report lost results rather than failing silently */
        pADCHandle->pADCx->CR1 |= (1U << ADC_CR1_OVRIE);

        /* Kick off the first conversion */
        ADC_StartConversion(pADCHandle->pADCx);
    }

    return state;
}


/*********************************************************************
 * @fn      		  - ADC_IRQConfig
 *
 * @brief             - Enables/disables the IRQ and sets its priority
 *
 * @param[in]         - IRQ number, use IRQ_NO_ADC
 * @param[in]         - IRQ priority, 0 is the most urgent
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 *
 * @Note              - Identical in structure to GPIO_IRQConfig and
 *                      USART_IRQConfig; only the IRQ number differs.
 *                      ADC1, ADC2 and ADC3 all share IRQ number 18.
 *********************************************************************/
void ADC_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi)
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
 * @fn      		  - ADC_IRQHandling
 *
 * @brief             - Single entry point for every ADC interrupt source
 *
 * @param[in]         - Pointer to ADC handle
 *
 * @return            - None
 *
 * @Note              - ADC1, ADC2 and ADC3 share one vector, so if you use
 *                      more than one ADC the application handler must call
 *                      this function once per handle.
 *********************************************************************/
void ADC_IRQHandling(ADC_Handle_t *pADCHandle)
{
    uint32_t temp1, temp2;

    /*************** Check for EOC (a result is ready) ***************/

    temp1 = pADCHandle->pADCx->SR  & (1U << ADC_SR_EOC);
    temp2 = pADCHandle->pADCx->CR1 & (1U << ADC_CR1_EOCIE);

    if(temp1 && temp2)
    {
        if(pADCHandle->RxBusyState == ADC_BUSY_IN_CONVERSION)
        {
            if(pADCHandle->RxLen > 0U)
            {
                /* Reading DR stores the result and clears EOC in one step */
                *pADCHandle->pRxBuffer = ADC_GetConversionValue(pADCHandle->pADCx);

                pADCHandle->pRxBuffer++;
                pADCHandle->RxLen--;
            }

            if(pADCHandle->RxLen == 0U)
            {
                /* Nothing left to collect - stop asking for interrupts */
                pADCHandle->pADCx->CR1 &= ~(1U << ADC_CR1_EOCIE);
                pADCHandle->pADCx->CR1 &= ~(1U << ADC_CR1_OVRIE);

                pADCHandle->RxBusyState = ADC_READY;
                pADCHandle->pRxBuffer   = NULL;

                ADC_ApplicationEventCallback(pADCHandle, ADC_EVENT_CONV_CMPLT);
            }
            else
            {
                /* In single conversion mode the hardware does not restart
                 * by itself, so trigger the next one here */
                if(!(pADCHandle->pADCx->CR2 & (1U << ADC_CR2_CONT)))
                {
                    ADC_StartConversion(pADCHandle->pADCx);
                }
            }
        }
        else
        {
            /* Unexpected conversion - clear the flag so we do not re-enter */
            (void)ADC_GetConversionValue(pADCHandle->pADCx);
        }
    }


    /*************** Check for the analog watchdog ***************/

    temp1 = pADCHandle->pADCx->SR  & (1U << ADC_SR_AWD);
    temp2 = pADCHandle->pADCx->CR1 & (1U << ADC_CR1_AWDIE);

    if(temp1 && temp2)
    {
        ADC_ClearFlag(pADCHandle->pADCx, ADC_FLAG_AWD);

        ADC_ApplicationEventCallback(pADCHandle, ADC_EVENT_AWD);
    }


    /*************** Check for overrun ***************/

    temp1 = pADCHandle->pADCx->SR  & (1U << ADC_SR_OVR);
    temp2 = pADCHandle->pADCx->CR1 & (1U << ADC_CR1_OVRIE);

    if(temp1 && temp2)
    {
        /* A result was lost because DR was not read in time */
        ADC_ClearFlag(pADCHandle->pADCx, ADC_FLAG_OVR);

        ADC_ApplicationEventCallback(pADCHandle, ADC_ERR_OVR);
    }
}


/*********************************************************************
 * @fn      		  - ADC_CountsToMillivolts
 *
 * @brief             - Converts a raw conversion result into millivolts
 *
 * @param[in]         - Pointer to ADC handle, used to read the resolution
 * @param[in]         - Raw count from ADC_ReadChannel
 *
 * @return            - Voltage in millivolts
 *
 * @Note              - Uses integer maths throughout. At 12-bit resolution
 *                      over 3.3 V, one count is about 0.806 mV.
 *                      This is not a register operation, but students who
 *                      only ever see raw counts never connect the peripheral
 *                      to the physical world.
 *********************************************************************/
uint32_t ADC_CountsToMillivolts(ADC_Handle_t *pADCHandle, uint16_t Counts)
{
    uint32_t fullscale;

    /* Full scale depends on how many bits the conversion produced */
    switch(pADCHandle->ADC_Config.ADC_Resolution)
    {
        case ADC_RES_10BITS: fullscale = 1023U; break;
        case ADC_RES_8BITS:  fullscale = 255U;  break;
        case ADC_RES_6BITS:  fullscale = 63U;   break;
        default:             fullscale = 4095U; break;   // 12-bit
    }

    /* Multiply before dividing so the integer division keeps its precision */
    return (((uint32_t)Counts * ADC_VREF_MILLIVOLTS) / fullscale);
}


/*********************************************************************
 * @fn      		  - ADC_ApplicationEventCallback
 *
 * @brief             - Weak default callback
 *
 * @param[in]         - Pointer to ADC handle
 * @param[in]         - Event code, from the ADC_EVENT_xxx macros
 *
 * @return            - None
 *
 * @Note              - The application should override this. Declared weak so
 *                      the project still links if it does not.
 *********************************************************************/
__attribute__((weak)) void ADC_ApplicationEventCallback(ADC_Handle_t *pADCHandle,
                                                        uint8_t AppEvent)
{
    (void)pADCHandle;
    (void)AppEvent;

    /* Intentionally empty - override in the application */
}
