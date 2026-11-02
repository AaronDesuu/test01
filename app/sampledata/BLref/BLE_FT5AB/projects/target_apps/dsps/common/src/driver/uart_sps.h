/**
 ****************************************************************************************
 *
 * @file uart_spp.h
 *
 * @brief UART driver for SPP
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
 
 
#ifndef _UART_SPS_H_
#define _UART_SPS_H_

/**
 ****************************************************************************************
 * @defgroup UART UART
 * @ingroup DRIVERS
 *
 * @brief UART driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>       // integer definition
#include <stdbool.h>      // boolean definition
#include "uart.h"
#include "user_periph_setup.h"
/*
 * DEFINES
 *****************************************************************************************
 */
// /// Flow control bytes
#define UART_XON_BYTE               (uint8_t)0x11
#define UART_XOFF_BYTE              (uint8_t)0x13


/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
 
 enum
{
    /// status ok
    UART_STATUS_OK,
    /// status not ok
    UART_STATUS_ERROR
};
enum
{
    /// error detection disabled
    UART_ERROR_DETECT_DISABLED = 0,
    /// error detection enabled
    UART_ERROR_DETECT_ENABLED  = 1
};
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the UART to default values.
 *****************************************************************************************
 */
void uart_sps_init(uint8_t baudr, uint8_t dlf_value,  uint8_t mode );

/**
 ****************************************************************************************
 * @brief Enable UART flow (hw).
 *****************************************************************************************
 */
void uart_sps_flow_on(void);

/**
 ****************************************************************************************
 * @brief Disable UART flow (hw).
 *****************************************************************************************
 */
bool uart_sps_flow_off(bool force);

/**
 ****************************************************************************************
 * @brief Returns size of TX buffer contents.
 *****************************************************************************************
 */
uint32_t uart_sps_get_tx_buffer_size(void);
/**
 ****************************************************************************************
 * @brief Finish current UART transfers
 *****************************************************************************************
 */
void uart_sps_finish_transfers(void);

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * As soon as the end of the data transfer or a buffer overflow is detected,
 * the hci_uart_rx_done function is executed.
 *
 * @param[in,out]  bufptr Pointer to the RX buffer
 * @param[in]      size   Size of the expected reception
 *****************************************************************************************
 */
void uart_sps_read(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t, uint32_t));

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * As soon as the end of the data transfer is detected, the hci_uart_tx_done function is
 * executed.
 *
 * @param[in]  bufptr Pointer to the TX buffer
 * @param[in]  size   Size of the transmission
 *****************************************************************************************
 */
void uart_sps_write(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));

void uart_sps_register_flow_ctrl_cb(void (*callback) (bool));

/**
 ****************************************************************************************
 * @brief Check if RX FIFO is empty.
 *****************************************************************************************
 */
bool uart_sps_is_rx_fifo_empty(void);

/**
 ****************************************************************************************
 * @brief Check rx or tx is ongoing.
 *****************************************************************************************
 */
bool uart_sps_fifo_check(void);

/// @} UART
#endif /* _UART_SPS_H_ */
