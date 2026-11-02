/**
 ****************************************************************************************
 *
 * @file user_sps_scheduler.h
 *
 * @brief SPS Project application header file.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef APP_SPS_SCHEDULER_H_
#define APP_SPS_SCHEDULER_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application entry point.
 *
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 #include <stdint.h>
 #include <stdbool.h>
 #include "user_buffer.h"
 #include "uart_sps.h"
 #include "user_periph_setup.h"
 /*
 * DEFINES
 ****************************************************************************************
 */
//application defines 
#if 0 //defaults
#define TX_CALLBACK_SIZE        (uint8_t)   16  //16 bytes messages
#define RX_CALLBACK_SIZE        (uint8_t)   8

#define TX_BUFFER_ITEM_COUNT    (int)       1800
#define TX_BUFFER_HWM           (int)       850     //47%
#define TX_BUFFER_LWM           (int)       650     //36%

#define RX_BUFFER_ITEM_COUNT    (int)       500
#define RX_BUFFER_HWM           (int)       350     //70%
#define RX_BUFFER_LWM           (int)       150     //30%

#define TX_SIZE                     (128)
#define TX_WAIT_LEVEL               (74)    //3 packets (20 + 2*27)
#define TX_WAIT_ROUNDS              (10)
#define TX_START_FRAME_DATA_SIZE    (20)
#define TX_CONTINUE_FRAME_DATA_SIZE (27)

#define UART_STATUS_INIT (UART_STATUS_ERROR +1)

#else
/******************************************************************/
#define TX_CALLBACK_SIZE        (uint8_t)   16  //16 bytes messages
#define RX_CALLBACK_SIZE        (uint8_t)   8

#if UART_SW_FLOW_ENABLED
/* TX TO UART */
#if (CFG_UART_SPS_BAUDRATE<UART_BAUDRATE_115K2)
#warning "Baudrates higher than 115k2 have not been tested with SW flow control"
#endif
#define TX_BUFFER_ITEM_COUNT    (int)       1800
#define TX_BUFFER_HWM           (int)        850
#define TX_BUFFER_LWM           (int)        650
/* RX FROM UART */
#define RX_BUFFER_ITEM_COUNT    (int)       1200
#define RX_BUFFER_HWM           (int)        800
#define RX_BUFFER_LWM           (int)        300 

#define TX_SIZE                     (480)  
#define TX_WAIT_LEVEL               (200)    
#define TX_WAIT_ROUNDS              (1)

//#define TX_BUFFER_ITEM_COUNT    (int)       32
//#define TX_BUFFER_HWM           (int)        16
//#define TX_BUFFER_LWM           (int)        12
///* RX FROM UART */
//#define RX_BUFFER_ITEM_COUNT    (int)       32
//#define RX_BUFFER_HWM           (int)        16
//#define RX_BUFFER_LWM           (int)        12 

//#define TX_SIZE                     (32)  
//#define TX_WAIT_LEVEL               (10)    
//#define TX_WAIT_ROUNDS              (1)

#else //UART_HW_FLOW_ENABLED
#if (CFG_UART_SPS_BAUDRATE>=8) //115k2->8
/* Baudrates lesser or equal than 115.2k*/
/* TX TO UART */
#define TX_BUFFER_ITEM_COUNT    (int)       1800
#define TX_BUFFER_HWM           (int)       1000
#define TX_BUFFER_LWM           (int)        660
/* RX FROM UART */
#define RX_BUFFER_ITEM_COUNT    (int)       1800
#define RX_BUFFER_HWM           (int)        600
#define RX_BUFFER_LWM           (int)        300

//#define TX_SIZE                     (240)
//#define TX_WAIT_LEVEL               (100)
#define TX_WAIT_ROUNDS              (10)
#else
#if !defined (__DA14531__)  
/* Baudrates larger than 115.2k*/
/* TX TO UART */
#define TX_BUFFER_ITEM_COUNT    (int)       6600
#define TX_BUFFER_HWM           (int)       1000
#define TX_BUFFER_LWM           (int)        600
/* RX FROM UART */
#define RX_BUFFER_ITEM_COUNT    (int)       3600
#define RX_BUFFER_HWM           (int)       2200
#define RX_BUFFER_LWM           (int)       1000

#define TX_SIZE                     (244)
#define TX_WAIT_LEVEL               (200)
#define TX_WAIT_ROUNDS              (5)
#else
/* TX TO UART */
#define TX_BUFFER_ITEM_COUNT    (int)       1800
#define TX_BUFFER_HWM           (int)       1000
#define TX_BUFFER_LWM           (int)        660
/* RX FROM UART */
#define RX_BUFFER_ITEM_COUNT    (int)       1800
#define RX_BUFFER_HWM           (int)        600
#define RX_BUFFER_LWM           (int)        300

//#define TX_SIZE                     (240)
//#define TX_SIZE                     (20)
//#define TX_WAIT_LEVEL               (100)
#define TX_WAIT_ROUNDS              (10)
#endif
#endif      
#endif //UART_SW_FLOW_ENABLED



        
#define UART_STATUS_INIT (UART_STATUS_ERROR +1)

#endif
/*
 * STRUCTURES
 ****************************************************************************************
 */

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize buffers
 ****************************************************************************************
 */
void user_scheduler_init(void);

/**
 ****************************************************************************************
 * @brief push ble data to uart transmit buffer
 ****************************************************************************************
 */
void user_ble_push(uint8_t* wrdata, uint16_t write_amount);

/**
 ****************************************************************************************
 * @brief Generate messages and provide to stream queue.
 ****************************************************************************************
*/
void user_ble_pull (bool init, bool success);

/**
 ****************************************************************************************
 * Sleep Functions
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Restores the flow control when device wakes up
 ****************************************************************************************
*/
void user_scheduler_reinit(void);

/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 ****************************************************************************************
 */
void user_sps_sleep_check(void);

/**
 ****************************************************************************************
 * @brief Check peripheral flow control state before sleep and restores it
 ****************************************************************************************
 */
void user_sps_sleep_restore(void);

/// @} APP

#endif //APP_SPS_SCHEDULER_H_
