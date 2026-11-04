/*
 * stm32f446xx_i2c_driver.h
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 */

#ifndef INC_STM32F446XX_I2C_DRIVER_H_       //this contains driver Specific data
#define INC_STM32F446XX_I2C_DRIVER_H_       //this contains driver Specific data

#include "stm32f446xx.h"                    //this contains MCU Specific data


/*
 * This is a Configuration structure for an I2C peripheral
 */

typedef struct
{
uint32_t I2C_SCLSpeed;       //<Possible values from @I2C_SCL_SPEED
uint8_t  I2C_DeviceAddress;  //<7-bit own address, only used in slave mode
uint8_t  I2C_ACKControl;     //<Possible values from @I2C_ACK_CONTROL
uint8_t  I2C_FMDutyCycle;    //<Possible values from @I2C_FM_DUTY
}I2C_Config_t;

/*
* This is a Handle structure for an I2C peripheral
*/

typedef struct
{

I2C_RegDef_t *pI2Cx;        // *!< This holds the base address of the I2C peripheral used >*/
I2C_Config_t  I2C_Config;   // *< This holds I2C peripheral configuration settings >*/

/* The members below are used only by the interrupt based APIs.
 * An I2C transfer is a state machine, not a simple byte loop, so the
 * handle must remember where it is in that sequence between interrupts. */
uint8_t  *pTxBuffer;        // *< Application Tx buffer address >*/
uint8_t  *pRxBuffer;        // *< Application Rx buffer address >*/
uint32_t  TxLen;            // *< Number of bytes still to transmit >*/
uint32_t  RxLen;            // *< Number of bytes still to receive >*/
uint8_t   TxRxState;        // *< Possible values from @I2C_STATE >*/
uint8_t   DevAddr;          // *< Slave address of the current transfer >*/
uint32_t  RxSize;           // *< Total size of the current reception >*/
uint8_t   Sr;               // *< Repeated start : I2C_ENABLE_SR or I2C_DISABLE_SR >*/

}I2C_Handle_t;


/********************************************************************************
 *                      Configuration option macros
 ********************************************************************************/

/*
 * @I2C_SCL_SPEED
 * Standard mode tops out at 100 kHz. Fast mode reaches 400 kHz but needs
 * the FS bit in CCR and a different duty cycle.
 */
#define I2C_SCL_SPEED_SM        100000U   // Standard mode, 100 kHz
#define I2C_SCL_SPEED_FM2K      200000U   // Fast mode, 200 kHz
#define I2C_SCL_SPEED_FM4K      400000U   // Fast mode, 400 kHz

/*
 * @I2C_ACK_CONTROL - CR1 reg. bit 10
 * NOTE: the ACK bit cannot be set until PE is set. The driver therefore
 *       applies this setting inside I2C_PeripheralControl, not I2C_Init.
 */
#define I2C_ACK_DISABLE         0U   // Do not acknowledge received bytes
#define I2C_ACK_ENABLE          1U   // Acknowledge, the normal case

/*
 * @I2C_FM_DUTY - CCR reg. bit 14
 * Only relevant in fast mode. Changes the low-to-high ratio of SCL.
 */
#define I2C_FM_DUTY_2           0U   // Tlow / Thigh = 2
#define I2C_FM_DUTY_16_9        1U   // Tlow / Thigh = 16 / 9

/*
 * @I2C_REPEATED_START
 * A repeated start keeps the bus without generating a STOP, so no other
 * master can interrupt between the write and the read. This is what
 * register-read sequences on most sensors require.
 */
#define I2C_DISABLE_SR          0U   // Generate a STOP at the end
#define I2C_ENABLE_SR           1U   // Hold the bus for a repeated start


/********************************************************************************
 *                    Status flags and driver states
 ********************************************************************************/

/*
 * I2C status register 1 (SR1) flag masks, for I2C_GetFlagStatus()
 */
#define I2C_FLAG_SB             (1U << I2C_SR1_SB)        // Start bit generated
#define I2C_FLAG_ADDR           (1U << I2C_SR1_ADDR)      // Address sent or matched
#define I2C_FLAG_BTF            (1U << I2C_SR1_BTF)       // Byte transfer finished
#define I2C_FLAG_ADD10          (1U << I2C_SR1_ADD10)     // 10-bit header sent
#define I2C_FLAG_STOPF          (1U << I2C_SR1_STOPF)     // Stop detected, slave mode
#define I2C_FLAG_RXNE           (1U << I2C_SR1_RXNE)      // Data register not empty
#define I2C_FLAG_TXE            (1U << I2C_SR1_TXE)       // Data register empty
#define I2C_FLAG_BERR           (1U << I2C_SR1_BERR)      // Bus error
#define I2C_FLAG_ARLO           (1U << I2C_SR1_ARLO)      // Arbitration lost
#define I2C_FLAG_AF             (1U << I2C_SR1_AF)        // Acknowledge failure, NACK
#define I2C_FLAG_OVR            (1U << I2C_SR1_OVR)       // Overrun or underrun
#define I2C_FLAG_TIMEOUT        (1U << I2C_SR1_TIMEOUT)   // Timeout error

/*
 * @I2C_STATE - application communication states
 */
#define I2C_READY               0U
#define I2C_BUSY_IN_RX          1U
#define I2C_BUSY_IN_TX          2U

/*
 * Application events reported through I2C_ApplicationEventCallback()
 */
#define I2C_EV_TX_CMPLT         0U   // Master transmission finished
#define I2C_EV_RX_CMPLT         1U   // Master reception finished
#define I2C_EV_STOP             2U   // Stop condition detected, slave mode
#define I2C_EV_DATA_REQ         3U   // Slave mode: the master wants a byte
#define I2C_EV_DATA_RCV         4U   // Slave mode: the master sent a byte
#define I2C_ERROR_BERR          5U   // Bus error
#define I2C_ERROR_ARLO          6U   // Arbitration lost
#define I2C_ERROR_AF            7U   // Acknowledge failure, no device responded
#define I2C_ERROR_OVR           8U   // Overrun or underrun
#define I2C_ERROR_TIMEOUT       9U   // Timeout


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Peripheral Clock Setup */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi); // for EnorDi = define macro in stm32f446xx.h

/* I2C Initialization and De-initialization */
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

/* Blocking (polling based) Master Send and Receive */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len,
                        uint8_t SlaveAddr, uint8_t Sr);
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len,
                           uint8_t SlaveAddr, uint8_t Sr);

/* Non-blocking (interrupt based) Master Send and Receive */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len,
                             uint8_t SlaveAddr, uint8_t Sr);
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len,
                                uint8_t SlaveAddr, uint8_t Sr);

/* Slave mode Send and Receive - single byte, called from the callback */
void    I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data);
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx);

/* I2C Interrupt Configuration and Handling */
void I2C_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle);   // for the I2Cx_EV vector
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle);   // for the I2Cx_ER vector

/* Other Peripheral Control APIs */
void    I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);  // the PE bit
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName);
void    I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);
void    I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx);
void    I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);
void    I2C_CloseSendData(I2C_Handle_t *pI2CHandle);
void    I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle);

/* Application Callback - implemented by the application, weak default in the .c */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEvent);


#endif /* INC_STM32F446XX_I2C_DRIVER_H_ */
