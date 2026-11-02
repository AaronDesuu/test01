/**
 ****************************************************************************************
 *
 * @file user_sps_schedule_dma.h
 *
 * @brief SPS project include file for scheduling DMA  UART operations.
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

#ifndef _USER_SPS_SCHEDULE_DMA_H_
#define _USER_SPS_SCHEDULE_DMA_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdlib.h>
#include "arch_api.h"

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 * @return void
 ****************************************************************************************
 */ 
void user_dma_sps_sleep_check(void);

/**
 ****************************************************************************************
 * @brief Checks the UART DMA Rx timeout
 * @return void
 ****************************************************************************************
 */
void user_check_dma_uart_rx_to(void);

/**
 ****************************************************************************************
 * @brief Checks if the receive list queue has exceeded the limit RX_FLOW_OFF_LVL in order
 *        to deassert RTS
 * @return void
 ****************************************************************************************
 */
bool user_check_set_flow_off(void);

/**
 ****************************************************************************************
 * @brief Checks if the receive list queue is bellow limit RX_FLOW_ON_LVL in order
 *        to assert RTS
 * @return void
 ****************************************************************************************
 */
bool user_check_set_flow_on(void);

/**
 ****************************************************************************************
 * @brief  Initializes the DMA Uart on init and keeps polling RX/TX DMA operations
 * @return arch_main_loop_callback_ret_t mode GOTO_SLEEP or KEEP_POWERED
 ****************************************************************************************
 */
arch_main_loop_callback_ret_t user_on_ble_powered(void);

#endif // _USER_SPS_SCHEDULE_DMA_H_

/// @} DSPS_SERVICE_COMMON
