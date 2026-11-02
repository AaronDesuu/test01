/**
 ****************************************************************************************
 *
 * @file remote_config_task.h
 *
 * @brief Remote COnfiguration Service Task implementation header file
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup REMOTE_CONFIG_GATT_SERVICE
 * @{
 ****************************************************************************************
 */
 
#ifndef REMOTE_CONFIG_TASK_H_
#define REMOTE_CONFIG_TASK_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_REMOTE_CONFIG)
#include "ke_task.h"
#include "remote_config.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum number of Sample128 task instances
#define REMOTE_CONFIG_IDX_MAX      (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the REMOTE_CONFIG task
enum remote_config_state
{
    /// Disabled State
    REMOTE_CONFIG_DISABLED,
    /// Idle state
    REMOTE_CONFIG_IDLE,
    /// Connected state
    REMOTE_CONFIG_CONNECTED,
    /// Number of defined states.
    REMOTE_CONFIG_STATE_MAX
};

/// Messages for Remote configuration
enum remote_config_msg_id
{
    /// Start remote_config. Remote connection
    REMOTE_CONFIG_ENABLE_REQ = KE_FIRST_MSG(TASK_ID_REMOTE_CONFIG),

    /// Att Value change indication
    REMOTE_DISCOVER_VAL_IND,

    /// Discover elms response indication
    REMOTE_DISCOVER_RESPONSE_CMD,
    
    /// Discover configuration Change Indication
    REMOTE_DISCOVER_CFG_IND,

    /// Write value request indication
    REMOTE_WRITE_REQUEST_IND,

    /// Read value response indication
    REMOTE_WRITE_RESPONSE_CMD,
    
    /// Write configuration Change Indication
    REMOTE_WRITE_CFG_IND,

    /// Read value request indication
    REMOTE_READ_REQUEST_IND,
    
    /// Read value response indication
    REMOTE_READ_RESPONSE_CMD,

    /// Read configuration Change Indication
    REMOTE_READ_CFG_IND,

    /// Error Indication
    REMOTE_CONFIG_ERROR_IND,            
};

/// Features Flag Masks
enum remote_config_features
{
    /// Characteristic indications disabled
    REMOTE_CONFIG_IND_DIS,
    /// Characteristic notifications enabled
    REMOTE_CONFIG_NTF_EN,
    /// Characteristic indications enabled
    REMOTE_CONFIG_IND_EN,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref REMOTE_CONFIG_ENABLE_REQ message
struct remote_config_enable_req
{
    /// Connection index
    uint8_t  conidx;

    /// Notification Configuration
    uint8_t  ntf_cfg;

    /// Configuration setup version
    uint8_t version[16];

    /// Configuration setup version length
    uint8_t version_length;     
};

/// Parameters of the GATTC_WRITE_REQ_IND message
struct remote_config_val_ind
{
    /// Connection handle
    uint16_t conhdl;
    
    /// Value
    uint8_t val[64];
    
    /// Connection handle
    uint8_t size;
    
    /// Characteristic id
    uint8_t param_id;
    
};

/// Parameters of the @ref REMOTE_DISCOVER_VAL_IND message
struct remote_discover_response_cmd
{
    /// Connection handle
    uint16_t conhdl;

    /// Number of elements
    uint8_t num_of_elms;

    /// More elements flag
    uint8_t more_elms;

    /// size
    uint8_t size;
    
    /// status
    uint8_t status;

    /// Value
    uint16_t val[__ARRAY_EMPTY];
};

/// Parameters of the @ref REMOTE_DISCOVER_RESPONSE_CMD message
struct remote_discover_request_ind
{
    /// Connection handle
    uint16_t conhdl;

    /// Value
    uint8_t max_elms;

    /// status
    uint8_t status;
};

/// Parameters of the @ref REMOTE_WRITE_REQUEST_IND message
struct remote_write_request_ind
{
    /// Connection handle
    uint16_t conhdl;

    /// Number of elements
    uint8_t num_of_elms;

    /// More elements flag
    uint8_t more_elms;

    /// Value
    uint8_t data[132];
};

/// Parameters of the @ref REMOTE_WRITE_RESPONSE_CMD message
struct remote_write_response_cmd
{
    /// Connection handle
    uint16_t conhdl;

    /// status
    uint8_t status;

    /// Elemnt id
    uint16_t elm_id;
};

/// Parameters of the @ref REMOTE_READ_REQUEST_IND message
struct remote_read_request_ind
{
    /// Connection handle
    uint16_t conhdl;
    
    /// Characteristic id
    uint16_t param_id;
    
};

/// Parameters of the @ref REMOTE_READ_RESPONSE_CMD message
struct remote_read_response_cmd
{
    /// Connection handle
    uint16_t conhdl;
    
    /// Value
    uint8_t val[64];
    
    /// Connection handle
    uint8_t size;
    
    /// Characteristic id
    uint16_t param_id;
    
    /// status 
    uint8_t status;
    
};

/// Parameters of the @ref REMOTE_DISCOVER_CFG_IND message
struct remote_discover_cfg_ind
{
    /// Connection handle
    uint16_t conhdl;
    /// Value
    uint16_t val;
};

struct remote_write_cfg_ind
{
    /// Connection handle
    uint16_t conhdl;
    /// Value
    uint16_t val;
};

struct remote_read_cfg_ind
{
    /// Connection handle
    uint16_t conhdl;
    /// Value
    uint16_t val;  
};

/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler remote_config_default_handler;

#endif //BLE_REMOTE_CONFIG

/// @} 
#endif // REMOTE_CONFIG_TASK_H_
