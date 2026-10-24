/*
 * stm32f446xx_usart_driver.h
 *
 *  Created on: Aug 26, 2026
 *      Author: raghudathesh.gp
 */

#ifndef INC_STM32F446XX_USART_DRIVER_H_     //this contains driver Specific data
#define INC_STM32F446XX_USART_DRIVER_H_     //this contains driver Specific data

#include "stm32f446xx.h"                    //this contains MCU Specific data


/*
 * This is a Configuration structure for a USART peripheral
 */

typedef struct
{
uint8_t  USART_Mode;            //<Possible values from @USART_MODE
uint32_t USART_Baud;            //<Possible values from @USART_BAUD
uint8_t  USART_NoOfStopBits;    //<Possible values from @USART_STOPBITS
uint8_t  USART_WordLength;      //<Possible values from @USART_WORDLEN
uint8_t  USART_ParityControl;   //<Possible values from @USART_PARITY
uint8_t  USART_HWFlowControl;   //<Possible values from @USART_HWFLOWCTRL
}USART_Config_t;

/*
* This is a Handle structure for a USART peripheral
*/

typedef struct
{

USART_RegDef_t *pUSARTx;    // *!< This holds the base address of the USART peripheral used >*/
USART_Config_t  USART_Config;   // *< This holds USART peripheral configuration settings >*/

/* The members below are used only by the interrupt based APIs.
 * The ISR moves one byte per interrupt, so it needs somewhere to
 * remember the buffer, how much is left and whether it is busy. */
uint8_t  *pTxBuffer;        // *< Application Tx buffer address >*/
uint8_t  *pRxBuffer;        // *< Application Rx buffer address >*/
uint32_t  TxLen;            // *< Number of bytes still to transmit >*/
uint32_t  RxLen;            // *< Number of bytes still to receive >*/
uint8_t   TxBusyState;      // *< Possible values from @USART_STATE >*/
uint8_t   RxBusyState;      // *< Possible values from @USART_STATE >*/

}USART_Handle_t;


/********************************************************************************
 *                      Configuration option macros
 ********************************************************************************/

/*
 * @USART_MODE
 */
#define USART_MODE_ONLY_TX          0U   // Transmit only  - CR1 TE set -Pg No 817
#define USART_MODE_ONLY_RX          1U   // Receive only   - CR1 RE set
#define USART_MODE_TXRX             2U   // Full duplex    - CR1 TE and RE set

/*
 * @USART_BAUD
 * Standard baud rates. Any other value may also be passed directly. Pg. No. 790 - 791
 */
#define USART_STD_BAUD_1200         1200U  //Baud rate register (USART_BRR)
#define USART_STD_BAUD_2400         2400U
#define USART_STD_BAUD_9600         9600U
#define USART_STD_BAUD_19200        19200U
#define USART_STD_BAUD_38400        38400U
#define USART_STD_BAUD_57600        57600U
#define USART_STD_BAUD_115200       115200U
#define USART_STD_BAUD_230400       230400U
#define USART_STD_BAUD_460800       460800U
#define USART_STD_BAUD_921600       921600U
#define USART_STD_BAUD_2M           2000000U
#define USART_STD_BAUD_3M           3000000U

/*
 * @USART_PARITY - CR1 Reg. Pg No 817 bit 9 and 10
 */
#define USART_PARITY_DISABLE        0U   // No parity bit in the frame
#define USART_PARITY_EN_EVEN        1U   // Parity enabled, even
#define USART_PARITY_EN_ODD         2U   // Parity enabled, odd

/*
 * @USART_WORDLEN
 * NOTE: the parity bit occupies the LAST data bit position. For 8 data bits
 *       PLUS parity you must select 9-bit word length.  CR1 Reg. Pg. No. 817 bit 12
 */
#define USART_WORDLEN_8BITS         0U   // CR1 M bit = 0
#define USART_WORDLEN_9BITS         1U   // CR1 M bit = 1

/*
 * @USART_STOPBITS - CR2 reg. Pg. No. 820 bit 12-13
 */
#define USART_STOPBITS_1            0U   // CR2 STOP = 00
#define USART_STOPBITS_0_5          1U   // CR2 STOP = 01
#define USART_STOPBITS_2            2U   // CR2 STOP = 10
#define USART_STOPBITS_1_5          3U   // CR2 STOP = 11

/*
 * @USART_HWFLOWCTRL - CR3 reg. Pg. No. 821 bit 8-9
 */
#define USART_HW_FLOW_CTRL_NONE     0U   // No hardware flow control
#define USART_HW_FLOW_CTRL_CTS      1U   // CTS only
#define USART_HW_FLOW_CTRL_RTS      2U   // RTS only
#define USART_HW_FLOW_CTRL_CTS_RTS  3U   // Both CTS and RTS


/********************************************************************************
 *                    Status flags and driver states
 ********************************************************************************/

/*
 * USART status register (SR) flag masks, for USART_GetFlagStatus()
 */
#define USART_FLAG_PE               (1U << USART_SR_PE)     // Parity error
#define USART_FLAG_FE               (1U << USART_SR_FE)     // Framing error
#define USART_FLAG_NE               (1U << USART_SR_NE)     // Noise detected
#define USART_FLAG_ORE              (1U << USART_SR_ORE)    // Overrun error
#define USART_FLAG_IDLE             (1U << USART_SR_IDLE)   // Idle line detected
#define USART_FLAG_RXNE             (1U << USART_SR_RXNE)   // Read data reg not empty
#define USART_FLAG_TC               (1U << USART_SR_TC)     // Transmission complete
#define USART_FLAG_TXE              (1U << USART_SR_TXE)    // Transmit data reg empty
#define USART_FLAG_CTS              (1U << USART_SR_CTS)    // CTS flag

/*
 * @USART_STATE - application communication states
 */
#define USART_READY                 0U
#define USART_BUSY_IN_RX            1U
#define USART_BUSY_IN_TX            2U

/*
 * Application events reported through USART_ApplicationEventCallback()
 */
#define USART_EVENT_TX_CMPLT        0U   // Transmission finished
#define USART_EVENT_RX_CMPLT        1U   // Reception finished
#define USART_EVENT_IDLE            2U   // Idle line detected
#define USART_EVENT_CTS             3U   // CTS line changed
#define USART_EVENT_PE              4U   // Parity error
#define USART_ERR_FE                5U   // Framing error
#define USART_ERR_NE                6U   // Noise error
#define USART_ERR_ORE               7U   // Overrun error


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Peripheral Clock Setup */
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi); // for EnorDi = define macro in stm32f446xx.h

/* USART Initialization and De-initialization */
void USART_Init(USART_Handle_t *pUSARTHandle);
void USART_DeInit(USART_RegDef_t *pUSARTx);

/* Baud rate configuration - writes the BRR register */
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate);

/* Blocking (polling based) Data Send and Receive */
void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len);
void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len);

/* Non-blocking (interrupt based) Data Send and Receive */
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len);

/* USART Interrupt Configuration and Handling */
void USART_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void USART_IRQHandling(USART_Handle_t *pUSARTHandle);

/* Other Peripheral Control APIs */
void    USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi);
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName);
void    USART_ClearFlag(USART_RegDef_t *pUSARTx, uint32_t FlagName);

/* Application Callback - implemented by the application, weak default in the .c */
void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t AppEvent);


#endif /* INC_STM32F446XX_USART_DRIVER_H_ */
