/**
 ****************************************************************************************
 *
 * @file dma_uart_sps.h
 *
 * @brief SPS project include file for dma uart adaptation funtions
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup USER_PLATFORM User Platform
 * @defgroup UART UART
 * @addtogroup UART
 * @ingroup USER_PLATFORM
 * @brief UART handling
 * @{
 ****************************************************************************************
 */

#ifndef _DMA_UART_SPS_H_
#define _DMA_UART_SPS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include <stdlib.h>
#include <stdint.h>
#if BLE_SPS_CLIENT
#include "sps_client_task.h"
#include "sps_client.h"
#elif BLE_SPS_SERVER
#include "sps_server_task.h"
#include "sps_server.h"
#endif
#include "co_list.h"
#include "user_periph_setup.h"


/*
 * DEFINES
 ****************************************************************************************
 */
/* These RX TX LVL's are relative to UART */
#if defined (__DA14531__)
#define RX_FLOW_ON_LVL (4)
#define RX_FLOW_OFF_LVL ((MSG_HEAP_SZ/ELEMENT_SIZE_RX)/2) //(12)
#if defined (CFG_BLE_FLOW_CONTROL)
#define RX_FLOW_OFF_HEAP_LIMIT 57 //60
#define RX_FLOW_ON_HEAP_LIMIT 57  //50
#endif
#else
#define RX_FLOW_ON_LVL (4)
#define RX_FLOW_OFF_LVL (12)
#endif

#define DMA_UART_RX_TO_ROUNDS 1 // 20
/******************************************/
/* Expected size should be MTU-3 */
#define DEFAULT_TX_SIZE 244
#define EXPECTED_SIZE (dma_uart.tx_size)


#if BLE_SPS_SERVER
#define UART_RX_STRUCT sps_server_data_tx_req
#define UART_TX_STRUCT sps_val_write_ind
#define DATA_TX_REQ SPS_SERVER_DATA_TX_REQ
#define DEST_TASK_ID TASK_ID_SPS_SERVER
#else
#define UART_RX_STRUCT sps_client_data_tx_req
#define UART_TX_STRUCT sps_client_data_rx_ind
#define DATA_TX_REQ SPS_CLIENT_DATA_TX_REQ
#define DEST_TASK_ID TASK_ID_SPS_CLIENT
#endif

#define ELEMENT_SIZE_RX (sizeof(struct UART_RX_STRUCT)+EXPECTED_SIZE + sizeof(struct ke_msg)+16)
#define ELEMENT_SIZE_TX (sizeof(struct UART_TX_STRUCT)+EXPECTED_SIZE +sizeof(struct ke_msg)+16 )

#define PON_STATE_IDLE          0
#define PON_STATE_INIT          1
#define PON_STATE_WAIT_RTS  2

/*
 * STRUCTURES
 ****************************************************************************************
 */
struct dma_uart_tag{
    bool lists_initialized;

    /* Holds buffers received by the UART and stored by the DMA */
    struct co_list rx_list_ready;
    struct ke_msg *p_rx_ready_active;
    struct ke_msg *p_rx_active;

    /* Holds buffers waiting to be transmitted by the UART */
    struct co_list tx_list_ready;
    struct ke_msg *p_tx_active; 

    uint32_t nof_rx_callbacks;

    uint32_t uart_flow_off;
    uint32_t ble_flow_off;
    uint32_t ble_forced_flow_off;

    uint32_t rx_bytes;
    uint32_t tx_bytes;

    uint32_t power_on_state;

    uint32_t rx_to_cnt;
    uint32_t rx_cnt_prev;

    uint32_t rx_zero_data_cnt;

    uint32_t mtu;
    uint32_t tx_size;
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief  Asserts (pulls low) UART RTS
 *
 * @return void
 ****************************************************************************************
 */
void dma_uart_assert_rts(void);

/**
 ****************************************************************************************
 * @brief  De-asserts (pulls hi) UART RTS
 *
 * @return void
 ****************************************************************************************
 */
void dma_uart_deassert_rts(void);

/**
 ****************************************************************************************
 * @brief  Initiliazies the UART and the DMA channels on connection
 * 
 * @param[in] baudr a UART_BAUDRATE_xxx as in uart.h  
 * @param[in] mode not used
 *
 * @return void
 ****************************************************************************************
 */ 
void dma_uart_sps_init(UART_BAUDRATE baudr, uint8_t mode);

/**
 ****************************************************************************************
 * @brief Asynchronsously checks if there are buffers ready to be placed in uart tx queue.
 *        Used in BLE-> UART path
 * @return void
 ****************************************************************************************
 */ 
void dma_uart_tx_async(void);

/**
 ****************************************************************************************
 * @brief Handles closing the Rx DMA due to timeout.
 *
 * @return void 
 ****************************************************************************************
 */
void dma_uart_timeout(void);

/**
 ****************************************************************************************
 * @brief  Checks is DMA Rx channel has data
 * @param[out] rx_length  The length of the data the channel has received
 *
 * @return true if there is not data, false if data exists
 ****************************************************************************************
 */
bool dma_uart_rx_check_empty(uint32_t *rx_length);

/**
 ****************************************************************************************
 * @brief  Checks if any DMA Uart Channel is active
 *
 * @return true if any DMA channel is active, false if both channels inactive 
 ****************************************************************************************
 */ 
bool dma_uart_check_active(void);

/**
 ****************************************************************************************
 * @brief Initializes DMA Uart in Active (non sleep) mode
 *
 * @return void
 ****************************************************************************************
 */ 
void dma_uart_on_connect(void);

/**
 ****************************************************************************************
 * @brief Allocates a buffer and triggers reading from DMA
 *
 * @return void
 ****************************************************************************************
 */
void dma_uart_rx_activate(void);

/**
 ****************************************************************************************
 * @brief  Reactivates the DMA Uart Rx Channel when wake up
 *
 * @return void
 ****************************************************************************************
 */ 
void dma_uart_rx_reactivate(void);

/**
 ****************************************************************************************
 * @brief Deactivates the DMA Uart Rx Channel (Uart to BLE)
 *
 * @return void 
 ****************************************************************************************
 */
void dma_uart_rx_disable(void);

/**
 ****************************************************************************************
 * @brief  Checks if the UART Tx can be reactivated by comparing tx_list_ready.cnt
 *               with TX_FLOW_ON_LVL
 *
 * @return true if the Rx of data from the BLE peer can be continued, false otherwise
 ****************************************************************************************
 */
bool dma_uart_test_tx_queue_for_flow_on(void);

/**
 ****************************************************************************************
 * @brief  Calculates the new TX buffer size
 * @param[in] mtu  The negotiated MTU
 *
 * @return void
 ****************************************************************************************
 */ 
void dma_uart_set_tx_size(uint32_t mtu);

#endif // _DMA_UART_SPS_H_
/// @} UART
