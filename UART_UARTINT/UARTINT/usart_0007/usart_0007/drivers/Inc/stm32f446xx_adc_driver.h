/*
 * stm32f446xx_adc_driver.h
 *
 *  Created on: Aug 28, 2026
 *      Author: raghudathesh.gp
 */

#ifndef INC_STM32F446XX_ADC_DRIVER_H_       //this contains driver Specific data
#define INC_STM32F446XX_ADC_DRIVER_H_       //this contains driver Specific data

#include "stm32f446xx.h"                    //this contains MCU Specific data


/*
 * This is a Configuration structure for an ADC peripheral
 */

typedef struct
{
uint8_t ADC_Resolution;      //<Possible values from @ADC_RESOLUTION
uint8_t ADC_DataAlignment;   //<Possible values from @ADC_ALIGN
uint8_t ADC_ConvMode;        //<Possible values from @ADC_CONV_MODE
uint8_t ADC_ScanMode;        //<Possible values from @ADC_SCAN_MODE
uint8_t ADC_NumOfConversion; //<1 to 16. Written to SQR1 L field as (value - 1)
uint8_t ADC_SamplingTime;    //<Possible values from @ADC_SAMPLE_TIME
uint8_t ADC_ClkPreScaler;    //<Possible values from @ADC_CLK_PRESCALER
}ADC_Config_t;

/*
* This is a Handle structure for an ADC peripheral
*/

typedef struct
{

ADC_RegDef_t *pADCx;        // *!< This holds the base address of the ADC peripheral used >*/
ADC_Config_t  ADC_Config;   // *< This holds ADC peripheral configuration settings >*/

/* The members below are used only by the interrupt based APIs.
 * The ISR collects one result per interrupt, so it needs somewhere to
 * remember the buffer, how much is left and whether it is busy.
 * NOTE the buffer is uint16_t, not uint8_t - a 12-bit result does not
 * fit in a byte. */
uint16_t *pRxBuffer;        // *< Application result buffer address >*/
uint32_t  RxLen;            // *< Number of conversions still to collect >*/
uint8_t   RxBusyState;      // *< Possible values from @ADC_STATE >*/

}ADC_Handle_t;


/********************************************************************************
 *                      Configuration option macros
 ********************************************************************************/

/*
 * @ADC_RESOLUTION - CR1 reg. bits [25:24]
 * Lower resolution shortens the conversion phase, so it is faster.
 */
#define ADC_RES_12BITS          0U   // 12-bit, conversion phase is 12 cycles
#define ADC_RES_10BITS          1U   // 10-bit, conversion phase is 10 cycles
#define ADC_RES_8BITS           2U   // 8-bit,  conversion phase is 8 cycles
#define ADC_RES_6BITS           3U   // 6-bit,  conversion phase is 6 cycles

/*
 * @ADC_ALIGN - CR2 reg. bit 11
 */
#define ADC_ALIGN_RIGHT         0U   // Result in the low bits of DR - use this
#define ADC_ALIGN_LEFT          1U   // Result shifted up, rarely wanted

/*
 * @ADC_CONV_MODE - CR2 reg. bit 1
 */
#define ADC_CONV_SINGLE         0U   // One conversion per trigger, then stop
#define ADC_CONV_CONTINUOUS     1U   // Restart automatically, forever

/*
 * @ADC_SCAN_MODE - CR1 reg. bit 8
 * NOTE: with scan enabled every result still lands in the same DR, so
 *       without DMA each conversion overwrites the previous one.
 */
#define ADC_SCAN_DISABLE        0U   // Convert only the first sequence entry
#define ADC_SCAN_ENABLE         1U   // Walk the whole sequence

/*
 * @ADC_SAMPLE_TIME - SMPR1 and SMPR2, 3 bits per channel
 * The hold capacitor must fully charge through the source resistance.
 * A potentiometer needs 3 cycles; a high impedance divider needs many more,
 * or every reading comes out low.
 */
#define ADC_SAMPLE_3CYCLES      0U
#define ADC_SAMPLE_15CYCLES     1U
#define ADC_SAMPLE_28CYCLES     2U
#define ADC_SAMPLE_56CYCLES     3U
#define ADC_SAMPLE_84CYCLES     4U
#define ADC_SAMPLE_112CYCLES    5U
#define ADC_SAMPLE_144CYCLES    6U
#define ADC_SAMPLE_480CYCLES    7U

/*
 * @ADC_CLK_PRESCALER - common CCR reg. bits [17:16]
 * ADCCLK = PCLK2 / prescaler. There is no divide by 1.
 * The ADC clock must not exceed 36 MHz on the STM32F446.
 */
#define ADC_CLK_DIV2            0U   // PCLK2 / 2
#define ADC_CLK_DIV4            1U   // PCLK2 / 4
#define ADC_CLK_DIV6            2U   // PCLK2 / 6
#define ADC_CLK_DIV8            3U   // PCLK2 / 8


/********************************************************************************
 *                        Channel number macros
 ********************************************************************************/

/*
 * @ADC_CHANNELS
 * External channels 0 to 15, plus the internally connected ones.
 * Arduino header mapping on the NUCLEO-F446RE:
 *      A0 = PA0 = IN0      A3 = PB0 = IN8
 *      A1 = PA1 = IN1      A4 = PC1 = IN11
 *      A2 = PA4 = IN4      A5 = PC0 = IN10
 */
#define ADC_CHANNEL_0           0U
#define ADC_CHANNEL_1           1U
#define ADC_CHANNEL_2           2U
#define ADC_CHANNEL_3           3U
#define ADC_CHANNEL_4           4U
#define ADC_CHANNEL_5           5U
#define ADC_CHANNEL_6           6U
#define ADC_CHANNEL_7           7U
#define ADC_CHANNEL_8           8U
#define ADC_CHANNEL_9           9U
#define ADC_CHANNEL_10          10U
#define ADC_CHANNEL_11          11U
#define ADC_CHANNEL_12          12U
#define ADC_CHANNEL_13          13U
#define ADC_CHANNEL_14          14U
#define ADC_CHANNEL_15          15U

/*
 * Internally connected channels - ADC1 ONLY, and not brought out to any pin.
 * NOTE: on the STM32F446 the temperature sensor is channel 18, SHARED with
 *       VBAT. On the STM32F407 it is channel 16, so copied F407 example code
 *       reads a channel that is not connected here and returns nonsense.
 */
#define ADC_CHANNEL_VREFINT     17U  // Internal reference voltage
#define ADC_CHANNEL_TEMPSENSOR  18U  // Temperature sensor, shares IN18 with VBAT
#define ADC_CHANNEL_VBAT        18U  // Battery voltage, shares IN18 with the sensor


/********************************************************************************
 *                    Status flags and driver states
 ********************************************************************************/

/*
 * ADC status register (SR) flag masks, for ADC_GetFlagStatus()
 */
#define ADC_FLAG_AWD            (1U << ADC_SR_AWD)     // Analog watchdog
#define ADC_FLAG_EOC            (1U << ADC_SR_EOC)     // End of conversion
#define ADC_FLAG_JEOC           (1U << ADC_SR_JEOC)    // Injected end of conversion
#define ADC_FLAG_JSTRT          (1U << ADC_SR_JSTRT)   // Injected start
#define ADC_FLAG_STRT           (1U << ADC_SR_STRT)    // Regular start
#define ADC_FLAG_OVR            (1U << ADC_SR_OVR)     // Overrun

/*
 * @ADC_STATE - application conversion states
 */
#define ADC_READY               0U
#define ADC_BUSY_IN_CONVERSION  1U

/*
 * Application events reported through ADC_ApplicationEventCallback()
 */
#define ADC_EVENT_CONV_CMPLT    0U   // Every requested conversion has been collected
#define ADC_EVENT_AWD           1U   // Analog watchdog threshold crossed
#define ADC_ERR_OVR             2U   // Overrun, a result was lost

/*
 * Reference voltage in millivolts, used by ADC_CountsToMillivolts().
 * On the NUCLEO-F446RE, VREF+ is tied to VDD.
 */
#define ADC_VREF_MILLIVOLTS     3300U


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Peripheral Clock Setup */
void ADC_PeriClockControl(ADC_RegDef_t *pADCx, uint8_t EnorDi); // for EnorDi = define macro in stm32f446xx.h

/* ADC Initialization and De-initialization */
void ADC_Init(ADC_Handle_t *pADCHandle);
void ADC_DeInit(void);   // no argument: ADCRST resets all three ADCs together

/* Channel and sequence configuration */
void ADC_ConfigChannel(ADC_RegDef_t *pADCx, uint8_t Channel, uint8_t SamplingTime);
void ADC_SetSequenceLength(ADC_RegDef_t *pADCx, uint8_t Length);
void ADC_SetSequenceRank(ADC_RegDef_t *pADCx, uint8_t Channel, uint8_t Rank);

/* Internal channel control - temperature sensor, VREFINT and VBAT */
void ADC_TempSensorVrefintControl(uint8_t EnorDi);
void ADC_VbatControl(uint8_t EnorDi);

/* Conversion control */
void     ADC_PeripheralControl(ADC_RegDef_t *pADCx, uint8_t EnorDi); // the ADON bit
void     ADC_StartConversion(ADC_RegDef_t *pADCx);                   // the SWSTART bit
uint16_t ADC_GetConversionValue(ADC_RegDef_t *pADCx);                // reads DR

/* Blocking (polling based) read */
uint16_t ADC_ReadChannel(ADC_Handle_t *pADCHandle, uint8_t Channel);

/* Non-blocking (interrupt based) read */
uint8_t ADC_ReadChannelIT(ADC_Handle_t *pADCHandle, uint16_t *pRxBuffer, uint32_t Len);

/* ADC Interrupt Configuration and Handling */
void ADC_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void ADC_IRQHandling(ADC_Handle_t *pADCHandle);

/* Other Peripheral Control APIs */
uint8_t ADC_GetFlagStatus(ADC_RegDef_t *pADCx, uint32_t FlagName);
void    ADC_ClearFlag(ADC_RegDef_t *pADCx, uint32_t FlagName);

/* Helper - converts a raw count into millivolts */
uint32_t ADC_CountsToMillivolts(ADC_Handle_t *pADCHandle, uint16_t Counts);

/* Application Callback - implemented by the application, weak default in the .c */
void ADC_ApplicationEventCallback(ADC_Handle_t *pADCHandle, uint8_t AppEvent);


#endif /* INC_STM32F446XX_ADC_DRIVER_H_ */
