/*
 * stm32f446xx_can_driver.h
 *
 *  Created on: Sep 01, 2026
 *      Author: raghudathesh.gp
 *
 *  bxCAN - Basic Extended CAN controller.
 *
 *  Unlike SPI, I2C and USART, CAN is a MULTI-MASTER broadcast bus. There
 *  is no address and no master: every node transmits when the bus is idle,
 *  and collisions are resolved by the identifier itself - the message with
 *  the numerically lower ID wins and the loser simply retries. A message
 *  is not sent TO anybody; it is broadcast, and each node decides through
 *  its filters whether it is interested.
 *
 *  NOTE on hardware: the STM32 provides only the CAN protocol controller.
 *  A CAN TRANSCEIVER chip (MCP2551, TJA1050, SN65HVD230 or similar) is
 *  required between the MCU pins and the twisted pair, and the bus needs
 *  120 ohm termination at BOTH ends. Without a transceiver, nothing works
 *  except loopback mode.
 */

#ifndef INC_STM32F446XX_CAN_DRIVER_H_       //this contains driver Specific data
#define INC_STM32F446XX_CAN_DRIVER_H_       //this contains driver Specific data

#include "stm32f446xx.h"                    //this contains MCU Specific data


/*
 * This is a Configuration structure for a CAN peripheral
 */

typedef struct
{
uint32_t CAN_Prescaler;      //<1 to 1024. Bit rate = PCLK1 / Prescaler / (1 + TS1 + TS2)
uint8_t  CAN_Mode;           //<Possible values from @CAN_MODE
uint8_t  CAN_SJW;            //<Possible values from @CAN_SJW
uint8_t  CAN_TS1;            //<Possible values from @CAN_TS1
uint8_t  CAN_TS2;            //<Possible values from @CAN_TS2
uint8_t  CAN_TTCM;           //<Time triggered communication : ENABLE or DISABLE
uint8_t  CAN_ABOM;           //<Automatic bus-off management : ENABLE or DISABLE
uint8_t  CAN_AWUM;           //<Automatic wakeup : ENABLE or DISABLE
uint8_t  CAN_NART;           //<No automatic retransmission : ENABLE or DISABLE
uint8_t  CAN_RFLM;           //<Receive FIFO locked mode : ENABLE or DISABLE
uint8_t  CAN_TXFP;           //<Transmit FIFO priority : ENABLE or DISABLE
}CAN_Config_t;

/*
* This is a Handle structure for a CAN peripheral
*/

typedef struct
{

CAN_RegDef_t *pCANx;        // *!< This holds the base address of the CAN peripheral used >*/
CAN_Config_t  CAN_Config;   // *< This holds CAN peripheral configuration settings >*/

}CAN_Handle_t;


/*
* This is a message structure used for both transmission and reception
*/

typedef struct
{
uint32_t StdId;             // *< Standard identifier, 11 bits, 0 to 0x7FF >*/
uint32_t ExtId;             // *< Extended identifier, 29 bits, 0 to 0x1FFFFFFF >*/
uint8_t  IDE;               // *< Possible values from @CAN_IDENTIFIER_TYPE >*/
uint8_t  RTR;               // *< Possible values from @CAN_FRAME_TYPE >*/
uint8_t  DLC;               // *< Data length code, 0 to 8 bytes >*/
uint8_t  Data[8];           // *< Payload. A CAN frame carries AT MOST 8 bytes >*/
uint8_t  FMI;               // *< Filter match index, set by hardware on receive >*/

}CAN_Message_t;


/********************************************************************************
 *                      Configuration option macros
 ********************************************************************************/

/*
 * @CAN_MODE - BTR reg. bits 30 and 31
 * Loopback is invaluable for teaching: the controller receives its own
 * transmissions internally, so a single board can demonstrate a complete
 * send and receive cycle with NO transceiver and NO second node.
 */
#define CAN_MODE_NORMAL             0U   // Normal operation, needs a transceiver
#define CAN_MODE_LOOPBACK           1U   // Transmissions loop back internally
#define CAN_MODE_SILENT             2U   // Listen only, never drives the bus
#define CAN_MODE_SILENT_LOOPBACK    3U   // Both, for self test while offline

/*
 * @CAN_SJW - BTR reg. bits [25:24]
 * Resynchronisation jump width, in time quanta. How far the controller may
 * shift its sample point to stay locked to other nodes.
 */
#define CAN_SJW_1TQ                 0U
#define CAN_SJW_2TQ                 1U
#define CAN_SJW_3TQ                 2U
#define CAN_SJW_4TQ                 3U

/*
 * @CAN_TS1 - BTR reg. bits [19:16]
 * Time segment 1, in time quanta. The register stores (value - 1).
 */
#define CAN_TS1_1TQ                 0U
#define CAN_TS1_2TQ                 1U
#define CAN_TS1_3TQ                 2U
#define CAN_TS1_4TQ                 3U
#define CAN_TS1_5TQ                 4U
#define CAN_TS1_6TQ                 5U
#define CAN_TS1_7TQ                 6U
#define CAN_TS1_8TQ                 7U
#define CAN_TS1_9TQ                 8U
#define CAN_TS1_10TQ                9U
#define CAN_TS1_11TQ                10U
#define CAN_TS1_12TQ                11U
#define CAN_TS1_13TQ                12U
#define CAN_TS1_14TQ                13U
#define CAN_TS1_15TQ                14U
#define CAN_TS1_16TQ                15U

/*
 * @CAN_TS2 - BTR reg. bits [22:20]
 * Time segment 2, in time quanta. The register stores (value - 1).
 */
#define CAN_TS2_1TQ                 0U
#define CAN_TS2_2TQ                 1U
#define CAN_TS2_3TQ                 2U
#define CAN_TS2_4TQ                 3U
#define CAN_TS2_5TQ                 4U
#define CAN_TS2_6TQ                 5U
#define CAN_TS2_7TQ                 6U
#define CAN_TS2_8TQ                 7U

/*
 * @CAN_IDENTIFIER_TYPE - TIR reg. bit 2
 */
#define CAN_ID_STD                  0U   // 11-bit standard identifier
#define CAN_ID_EXT                  1U   // 29-bit extended identifier

/*
 * @CAN_FRAME_TYPE - TIR reg. bit 1
 * A remote frame carries no data; it asks another node to transmit the
 * message with that identifier.
 */
#define CAN_RTR_DATA                0U   // Data frame, carries a payload
#define CAN_RTR_REMOTE              1U   // Remote frame, requests data

/*
 * @CAN_FILTER_MODE - FM1R
 */
#define CAN_FILTER_MODE_IDMASK      0U   // Mask mode : accept a RANGE of IDs
#define CAN_FILTER_MODE_IDLIST      1U   // List mode : accept EXACT IDs only

/*
 * @CAN_FILTER_SCALE - FS1R
 */
#define CAN_FILTER_SCALE_16BIT      0U   // Two 16-bit filters per bank
#define CAN_FILTER_SCALE_32BIT      1U   // One 32-bit filter per bank

/*
 * @CAN_FIFO
 */
#define CAN_FIFO_0                  0U
#define CAN_FIFO_1                  1U

/*
 * @CAN_TX_MAILBOX
 */
#define CAN_TX_MAILBOX_0            0U
#define CAN_TX_MAILBOX_1            1U
#define CAN_TX_MAILBOX_2            2U
#define CAN_TX_NO_MAILBOX           0xFFU  // Returned when all three are busy


/********************************************************************************
 *                    Status flags and driver states
 ********************************************************************************/

/*
 * Return codes
 */
#define CAN_OK                      0U
#define CAN_ERROR                   1U
#define CAN_TIMEOUT                 2U

/*
 * Application events reported through CAN_ApplicationEventCallback()
 */
#define CAN_EVENT_TX_CMPLT          0U   // A mailbox finished transmitting
#define CAN_EVENT_RX_FIFO0          1U   // A message is waiting in FIFO 0
#define CAN_EVENT_RX_FIFO1          2U   // A message is waiting in FIFO 1
#define CAN_ERROR_BOF               3U   // Bus off - too many errors, node disconnected
#define CAN_ERROR_EPV               4U   // Error passive
#define CAN_ERROR_EWG               5U   // Error warning threshold reached


/********************************************************************************
 *                         APIs supported by this driver
 *              For more information about the APIs check the function definitions
 ********************************************************************************/

/* Peripheral Clock Setup */
void CAN_PeriClockControl(CAN_RegDef_t *pCANx, uint8_t EnorDi); // for EnorDi = define macro in stm32f446xx.h

/* CAN Initialization and De-initialization */
uint8_t CAN_Init(CAN_Handle_t *pCANHandle);
void    CAN_DeInit(CAN_RegDef_t *pCANx);

/* Filter configuration - a node receives NOTHING until at least one filter is set */
void CAN_ConfigFilter(uint8_t FilterBank, uint8_t FilterMode, uint8_t FilterScale,
                      uint32_t FilterId, uint32_t FilterMask, uint8_t FifoAssign);
void CAN_AcceptAllFilter(uint8_t FilterBank, uint8_t FifoAssign);

/* Operating mode control */
uint8_t CAN_Start(CAN_RegDef_t *pCANx);    // leave initialisation, join the bus
uint8_t CAN_Stop(CAN_RegDef_t *pCANx);     // re-enter initialisation mode

/* Transmission */
uint8_t CAN_AddTxMessage(CAN_RegDef_t *pCANx, CAN_Message_t *pMessage, uint8_t *pMailbox);
uint8_t CAN_IsTxMailboxFree(CAN_RegDef_t *pCANx);

/* Reception */
uint8_t CAN_GetRxFifoFillLevel(CAN_RegDef_t *pCANx, uint8_t Fifo);
uint8_t CAN_GetRxMessage(CAN_RegDef_t *pCANx, uint8_t Fifo, CAN_Message_t *pMessage);

/* CAN Interrupt Configuration and Handling */
void CAN_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void CAN_InterruptEnable(CAN_RegDef_t *pCANx, uint32_t InterruptBit);
void CAN_InterruptDisable(CAN_RegDef_t *pCANx, uint32_t InterruptBit);
void CAN_TX_IRQHandling(CAN_Handle_t *pCANHandle);    // for the CANx_TX vector
void CAN_RX0_IRQHandling(CAN_Handle_t *pCANHandle);   // for the CANx_RX0 vector
void CAN_RX1_IRQHandling(CAN_Handle_t *pCANHandle);   // for the CANx_RX1 vector
void CAN_SCE_IRQHandling(CAN_Handle_t *pCANHandle);   // for the CANx_SCE vector

/* Application Callback - implemented by the application, weak default in the .c */
void CAN_ApplicationEventCallback(CAN_Handle_t *pCANHandle, uint8_t AppEvent);


#endif /* INC_STM32F446XX_CAN_DRIVER_H_ */
