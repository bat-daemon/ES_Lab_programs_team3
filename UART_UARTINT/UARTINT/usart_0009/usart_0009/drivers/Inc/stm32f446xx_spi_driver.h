/*
 * stm32f446xx_spi_driver.h
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 */

#ifndef INC_STM32F446XX_SPI_DRIVER_H_       //this contains driver Specific data
#define INC_STM32F446XX_SPI_DRIVER_H_       //this contains driver Specific data

#include "stm32f446xx.h"                    //this contains MCU Specific data


/*
 * This is a Configuration structure for a SPI peripheral
 */

typedef struct
{
uint8_t SPI_DeviceMode;      //<Possible values from @SPI_DEVICE_MODE
uint8_t SPI_BusConfig;       //<Possible values from @SPI_BUS_CONFIG
uint8_t SPI_SclkSpeed;       //<Possible values from @SPI_SCLK_SPEED
uint8_t SPI_DFF;             //<Possible values from @SPI_DFF
uint8_t SPI_CPOL;            //<Possible values from @SPI_CPOL
uint8_t SPI_CPHA;            //<Possible values from @SPI_CPHA
uint8_t SPI_SSM;             //<Possible values from @SPI_SSM
}SPI_Config_t;

/*
* This is a Handle structure for a SPI peripheral
*/

typedef struct
{

SPI_RegDef_t *pSPIx;        // *!< This holds the base address of the SPI peripheral used >*/
SPI_Config_t  SPI_Config;   // *< This holds SPI peripheral configuration settings >*/

/* The members below are used only by the interrupt based APIs.
 * The ISR moves one frame per interrupt, so it needs somewhere to
 * remember the buffer, how much is left and whether it is busy. */
uint8_t  *pTxBuffer;        // *< Application Tx buffer address >*/
uint8_t  *pRxBuffer;        // *< Application Rx buffer address >*/
uint32_t  TxLen;            // *< Number of bytes still to transmit >*/
uint32_t  RxLen;            // *< Number of bytes still to receive >*/
uint8_t   TxState;          // *< Possible values from @SPI_STATE >*/
uint8_t   RxState;          // *< Possible values from @SPI_STATE >*/

}SPI_Handle_t;


/********************************************************************************
 *                      Configuration option macros
 ********************************************************************************/

/*
 * @SPI_DEVICE_MODE - CR1 reg. bit 2 (MSTR)
 */
#define SPI_DEVICE_MODE_SLAVE       0U   // The peripheral follows an external clock
#define SPI_DEVICE_MODE_MASTER      1U   // The peripheral generates SCLK

/*
 * @SPI_BUS_CONFIG
 * Full duplex uses both MOSI and MISO. Half duplex shares one line and
 * changes direction. Simplex RXONLY keeps the clock running but never
 * drives MOSI - useful for a receive-only sensor.
 */
#define SPI_BUS_CONFIG_FD               1U   // Full duplex, the normal case
#define SPI_BUS_CONFIG_HD               2U   // Half duplex, one bidirectional line
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY   3U   // Receive only, MOSI unused

/*
 * @SPI_SCLK_SPEED - CR1 reg. bits [5:3] (BR)
 * SCLK = PCLKx / divisor. There is NO divide by 1, so the fastest possible
 * SCLK is half the bus clock. SPI1 and SPI4 are on APB2 (up to 90 MHz),
 * SPI2 and SPI3 on APB1 (up to 45 MHz).
 */
#define SPI_SCLK_SPEED_DIV2         0U   // fastest
#define SPI_SCLK_SPEED_DIV4         1U
#define SPI_SCLK_SPEED_DIV8         2U
#define SPI_SCLK_SPEED_DIV16        3U
#define SPI_SCLK_SPEED_DIV32        4U
#define SPI_SCLK_SPEED_DIV64        5U
#define SPI_SCLK_SPEED_DIV128       6U
#define SPI_SCLK_SPEED_DIV256       7U   // slowest

/*
 * @SPI_DFF - CR1 reg. bit 11
 * Decides how many bits make one frame, and therefore how many clock
 * pulses the master generates per transfer.
 */
#define SPI_DFF_8BITS               0U   // 8-bit frames
#define SPI_DFF_16BITS              1U   // 16-bit frames

/*
 * @SPI_CPOL - CR1 reg. bit 1
 * The IDLE level of the clock line between transfers.
 */
#define SPI_CPOL_LOW                0U   // Clock rests low, first edge is rising
#define SPI_CPOL_HIGH               1U   // Clock rests high, first edge is falling

/*
 * @SPI_CPHA - CR1 reg. bit 0
 * Which clock edge the receiver uses to sample the data line.
 * CPOL and CPHA together give the four SPI modes 0 to 3. The mode is a
 * property of the SLAVE device - read its datasheet, do not guess.
 */
#define SPI_CPHA_LOW                0U   // Sample on the FIRST edge
#define SPI_CPHA_HIGH               1U   // Sample on the SECOND edge

/*
 * @SPI_SSM - CR1 reg. bit 9
 * Software slave management. With SSM enabled the NSS pin is ignored and
 * the SSI bit supplies the internal slave select level instead - which is
 * what you want when driving chip select from an ordinary GPIO.
 */
#define SPI_SSM_DI                  0U   // Hardware NSS management
#define SPI_SSM_EN                  1U   // Software NSS management, use a GPIO


/********************************************************************************
 *                    Status flags and driver states
 ********************************************************************************/

/*
 * SPI status register (SR) flag masks, for SPI_GetFlagStatus()
 */
#define SPI_FLAG_RXNE               (1U << SPI_SR_RXNE)    // Receive buffer not empty
#define SPI_FLAG_TXE                (1U << SPI_SR_TXE)     // Transmit buffer empty
#define SPI_FLAG_UDR                (1U << SPI_SR_UDR)     // Underrun, slave mode
#define SPI_FLAG_CRCERR             (1U << SPI_SR_CRCERR)  // CRC error
#define SPI_FLAG_MODF               (1U << SPI_SR_MODF)    // Mode fault
#define SPI_FLAG_OVR                (1U << SPI_SR_OVR)     // Overrun
#define SPI_FLAG_BSY                (1U << SPI_SR_BSY)     // Busy, shift register active
#define SPI_FLAG_FRE                (1U << SPI_SR_FRE)     // Frame format error

/*
 * @SPI_STATE - application communication states
 */
#define SPI_READY                   0U
#define SPI_BUSY_IN_RX              1U
#define SPI_BUSY_IN_TX              2U

/*
 * Application events reported through SPI_ApplicationEventCallback()
 */
#define SPI_EVENT_TX_CMPLT          0U   // Transmission finished
#define SPI_EVENT_RX_CMPLT          1U   // Reception finished
#define SPI_EVENT_OVR_ERR           2U   // Overrun error
#define SPI_EVENT_CRC_ERR           3U   // CRC error


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Peripheral Clock Setup */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi); // for EnorDi = define macro in stm32f446xx.h

/* SPI Initialization and De-initialization */
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);

/* Blocking (polling based) Data Send and Receive */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len);

/* Non-blocking (interrupt based) Data Send and Receive */
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len);

/* SPI Interrupt Configuration and Handling */
void SPI_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void SPI_IRQHandling(SPI_Handle_t *pSPIHandle);

/* Other Peripheral Control APIs */
void    SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi);  // the SPE bit
void    SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi);          // internal slave select
void    SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi);         // hardware NSS output
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName);
void    SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void    SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void    SPI_CloseReception(SPI_Handle_t *pSPIHandle);

/* Application Callback - implemented by the application, weak default in the .c */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEvent);


#endif /* INC_STM32F446XX_SPI_DRIVER_H_ */
