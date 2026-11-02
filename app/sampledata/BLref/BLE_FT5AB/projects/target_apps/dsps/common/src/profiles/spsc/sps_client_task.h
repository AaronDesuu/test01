/**
 ****************************************************************************************
 *
 * @file sps_client_task.h
 *
 * @brief Header file - SPS_CLIENTTASK.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SPS_CLIENT
 * @{
 ****************************************************************************************
 */

#ifndef SPS_CLIENT_TASK_H_
#define SPS_CLIENT_TASK_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "ke_task.h"
#include "sps_client.h"

#if (BLE_SPS_CLIENT)
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Serial Port Service client task instances
#define SPS_CLIENT_IDX_MAX    (BLE_CONNECTION_MAX)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the SPS_CLIENT task
enum
{
    /// IDLE state
    SPS_CLIENT_IDLE,
    
    ///Connected state
    SPS_CLIENT_CONNECTED,
    
    ///Discovering
    SPS_CLIENT_DISCOVERING,
    
    /// Number of defined states.
    SPS_CLIENT_STATE_MAX
};

///SPS Host API messages
enum
{
    ///SPS client role enable request from application.
    SPS_CLIENT_ENABLE_REQ = KE_FIRST_MSG(TASK_ID_SPS_CLIENT),
    
    /// SPS Host role enable confirmation to application.
    SPS_CLIENT_ENABLE_CFM,
 
    /// Request to transmit data
    SPS_CLIENT_DATA_TX_REQ,
    
    /// Confirm that data has been sent
    SPS_CLIENT_DATA_TX_CFM,
    
    /// Send data to app
    SPS_CLIENT_DATA_RX_IND,
    
    /// Send flow control status
    SPS_CLIENT_RX_FLOW_CTRL_REQ,
    
    /// Indicate flow control state
    SPS_CLIENT_TX_FLOW_CTRL_IND,
};

/*
 * API Messages Structures
 ****************************************************************************************
 */

///Parameters of the @ref SPS_CLIENT_ENABLE_REQ message
struct sps_client_enable_req
{
    /// Connection handle
    uint8_t conidx;
    
    /// Connection type
    uint8_t con_type;
    
};

///Parameters of the @ref SPS_CLIENT_ENABLE_CFM message
struct sps_client_enable_cfm
{
   
    /// Status
    uint8_t status;
    
    /// SPS Device details to keep in APP
    struct spsc_sps_content sps;
};

///Parameters of the @ref SPS_CLIENT_DATA_TX_REQ message
///WARNING, DO NOT ALTER THIS STRUCT, IT SHOULD BE COMPATIBLE WITH gattc_write_cmd struct
struct sps_client_data_tx_req
{
    uint8_t operation;
    /// Perform automatic execution
    /// (if false, an ATT Prepare Write will be used this shall be use for reliable write)
    bool auto_execute;
    /// operation sequence number
    uint16_t seq_num;
    /// Attribute handle
    uint16_t handle;
    /// Write offset
    uint16_t offset;
    /// Write length
    uint16_t length;
    /// Internal write cursor shall be initialized to 0
    uint16_t cursor;
    /// Value to write
    uint8_t data[__ARRAY_EMPTY];
};
///Parameters of the @ref SPS_CLIENT_DATA_TX_CFM message
struct sps_client_data_tx_cfm
{
    ///Status
    uint8_t status;
};

///Parameters of the @ref SPS_CLIENT_DATA_RX_IND message
///WARNING, DO NOT ALTER THIS STRUCT, IT SHOULD BE COMPATIBLE WITH gattc_event_ind struct
struct sps_client_data_rx_ind
{
    /// Event Type
    uint8_t type;
    /// Data length
    uint16_t length;
    /// Attribute handle
    uint16_t handle;
    /// Event Value
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref SPS_CLIENT_RX_FLOW_CTRL_REQ message
struct sps_client_rx_flow_ctrl_req
{
    // flow control state
    uint8_t flow_control_state;
};

///Parameters of the @ref SPS_CLIENT_TX_FLOW_CTRL_IND message
struct sps_client_tx_flow_ctrl_ind
{
    // flow control state
    uint8_t flow_control_state;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler sps_client_default_handler;

/*
 * Functions
 ****************************************************************************************
 */

#endif //BLE_SPS_CLIENT

#endif // SPS_CLIENT_TASK_H_

/// @} SPS_CLIENT
