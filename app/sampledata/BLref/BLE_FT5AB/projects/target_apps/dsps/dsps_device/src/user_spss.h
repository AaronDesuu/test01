/**
 ****************************************************************************************
 *
 * @file user_spss.h
 *
 * @brief SPS Application header file
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DSPS_APPLICATION      DSPS Application
 * @defgroup DSPS_DEVICE_SERVICE   DSPS Device Service
 * @addtogroup DSPS_DEVICE_SERVICE
 * @ingroup DSPS_APPLICATION
 * @brief DSPS Application Device Service.
 * @{
 ****************************************************************************************
 */

#ifndef USER_SPSS_H_
#define USER_SPSS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_SPS_SERVER)

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

#include "sps_server.h"
#include "user_remote_config.h"

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
enum config_struct_elem_id_t
{
    ELEM_ID_ADV_INTERVAL_MIN = 0x0100,
    ELEM_ID_ADV_INTERVAL_MAX,
    ELEM_ID_ADV_CHANNEL_MAP,
    ELEM_ID_ADV_MODE,
    ELEM_ID_ADV_FILTER_POLICY,
    ELEM_ID_ADV_PEER_ADDR,
    ELEM_ID_ADV_PEER_ADDR_TYPE,
    ELEM_ID_GAP_ROLE = 0x0200,
    ELEM_ID_GAP_MTU,
    ELEM_ID_GAP_ADDR_TYPE,
    ELEM_ID_GAP_ADDRESS,
    ELEM_ID_GAP_REGEN_ADDR_DUR,
    ELEM_ID_GAP_DEVICE_IRK,
    ELEM_ID_GAP_ATTR_CONFIG,
    ELEM_ID_GAP_SERVICE_START_HANDLE,
    ELEM_ID_GATT_SERVICE_START_HANDLE,
    ELEM_ID_GAP_MAX_MPS,
    ELEM_ID_GAP_MAX_TX_OCTETS,
    ELEM_ID_GAP_MAX_TX_TIME,
    ELEM_ID_CONN_INTERVAL_MIN = 0x0300,
    ELEM_ID_CONN_INTERVAL_MAX,
    ELEM_ID_CONN_LATENCY,
    ELEM_ID_CONN_TIME_OUT,
    ELEM_ID_CONN_CE_LEN_MIN,
    ELEM_ID_CONN_CE_LEN_MAX,
    ELEM_ID_USER_ADV_DATA = 0x0400,
    ELEM_ID_USER_ADV_DATA_LEN,
    ELEM_ID_USER_ADV_SCAN_RESP_DATA,
    ELEM_ID_USER_ADV_SCAN_RESP_DATA_LEN,
    ELEM_ID_USER_DEVICE_NAME,
    ELEM_ID_USER_DEVICE_NAME_LEN,
    ELEM_ID_PERIPH_UART_BAUDRATE = 0x0500,
    ELEM_ID_CENTRAL_CODE  = 0x0600,
    ELEM_ID_CENTRAL_ADDR_SRC,
    ELEM_ID_CENTRAL_SCAN_INTERVAL,
    ELEM_ID_CENTRAL_SCAN_WINDOW,
    ELEM_ID_CENTRAL_PEER_ADDR_0,
    ELEM_ID_CENTRAL_PEER_ADDR_0_TYPE,
    ELEM_ID_GPIO_UART1_RX = 0x700,
    ELEM_ID_GPIO_UART1_TX,
    ELEM_ID_GPIO_UART1_CTS,
    ELEM_ID_GPIO_UART1_RTS,
    ELEM_ID_GPIO_EXT_WAKEUP,
    ELEM_ID_GPIO_EXT_WAKEUP_POLARITY,
    ELEM_ID_GPIO_EXT_WAKEUP_DEBOUNCE,
    ELEM_ID_GPIO_ACTIVE_STATUS_PIN,
    ELEM_ID_GPIO_WAKEUP_EXT_HOST,
    ELEM_ID_GPIO_POR_PIN,
    ELEM_ID_GPIO_POR_PIN_POLARITY,
    ELEM_ID_GPIO_POR_PIN_TIMEOUT,
    ELEM_ID_GPIO_UART2_RX,
    ELEM_ID_GPIO_UART2_TX,
};

// CONNECTION INTERVAL: From 6*1.25 (7.5 ms) to 3200*1.25(4000 ms)
#define CON_INT_MIN     6
#define CON_INT_MAX     3200

// MTU: 23 bytes to 512 bytes (maximum attribute size)
#define MTU_MIN         23
#define MTU_MAX         512

// ADVERTISING INTERVAL: From 6*0.625 (20 ms) to 3200*1.25(4000 ms)
#define ADV_INT_MIN     32
#define ADV_INT_MAX     16384

// Slave Latency: From 0 to 0x1F3 (499) connection events
#define SL_MAX     499

//Connection supervision timeout. From 10 to 3200 slots of 10ms. Time = N * 10 ms. Time Range: 100 ms to 32 s
#define SUP_TO_MIN     	10
#define SUP_TO_MAX     3200

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
 /**
 ****************************************************************************************
 * @brief Initialize task variables
 * @return void
 ****************************************************************************************
 */ 
void user_spss_init(void);

 /**
 ****************************************************************************************
 * @brief       user_sps_create_db creates a database for the sps server service
 * @return      none
 ****************************************************************************************
 */
void user_spss_create_db(void);

/**
 ****************************************************************************************
 * @brief     Function used to enable server role
 * @param[in] conidx	Connection handle
 * @return    none
 ****************************************************************************************
 */
void user_spss_enable(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief	  Send data over Bluetooth
 * @param[in] data		Pointer to the data
 * @param[in] length	length of the data
 * @return	  none
 ****************************************************************************************
 */
void user_send_ble_data(const uint8_t *data, uint16_t length);

/**
 ****************************************************************************************
 * @brief     Sends dummy data over Bluetooth
 * @param[in] length   length of dummy data
 * @return    none
 ****************************************************************************************
 */
void user_send_dummy_ble_data(uint16_t length);

/**
 ****************************************************************************************
 * @brief       Send flow control over Bluetooth
 * @param[in]   flow_ctrl flow control (FLOW_ON or FLOW_OFF)
 * @return      none
 ****************************************************************************************
 */
void user_send_ble_flow_ctrl(uint8_t flow_ctrl);

#endif //BLE_SPS_SERVER

#endif // USER_SPSS_H_

/// @} DSPS_DEVICE_SERVICE
