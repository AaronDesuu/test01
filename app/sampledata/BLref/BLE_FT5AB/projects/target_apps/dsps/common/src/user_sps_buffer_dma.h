/**
 ****************************************************************************************
 *
 * @file user_sps_buffer_dma.h
 *
 * @brief SPS project include file for handling DMA buffers.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_SERVICE_COMMON
 * @{
 ****************************************************************************************
 */

#ifndef _USER_SPS_BUFFER_DMA_H_
#define _USER_SPS_BUFFER_DMA_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 #include <stdio.h>
 
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */ 
/**
 ****************************************************************************************
 * @brief  Initialize DMA queues
 * @return void
 ****************************************************************************************
 */ 
void user_init_queues(void);

/**
 ****************************************************************************************
 * @brief Places a buffer from BLE to DMA Uart TX queue
 * @param[in] msg a pointer to sps_client_data_rx_ind parameter
 * @return void
 ****************************************************************************************
 */
void user_ble_to_dma_uart(void *msg);

/**
 ****************************************************************************************
 * @brief  If there is confirmation message send another one or resend the same one
 * @param[in] success  true if the message has been confirmed, false if error occured
 * @return void
 ****************************************************************************************
 */   
void user_dma_uart_to_ble_confirm(bool success);

/**
 ****************************************************************************************
 * @brief Asynchronsously checks if there are buffers ready to be send over BLE
 *        Used in UART->BLE path
 * @return true if a buffer has successfully been pushed to BLE, false otherwise
 ****************************************************************************************
 */  
bool user_dma_uart_to_ble(void);


#endif  // _USER_SPS_BUFFER_DMA_H_

/// @} DSPS_SERVICE_COMMON
