/**
 ****************************************************************************************
 *
 * @file user_remote_config.h
 *
 * @brief Remote Configuration Application Implementation header
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
 

#ifndef USER_REMOTE_CONFIG_H_
#define USER_REMOTE_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "gpio.h"
#include "user_config_common.h"

#if BLE_REMOTE_CONFIG
#include "remote_config.h"
#include "remote_config_task.h"

extern uint16_t user_remote_config_conhdl;
/*
 * DEFINES
 ****************************************************************************************
 */
 
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef uint8_t (*user_rem_config_function_t)(user_configuration_struct_tag*);
#if 0
/// Remote Configuration Parameters
struct user_remote_config_param 
{
    uint8_t  *p_data;
    uint8_t  size;
};
#endif
/// Remote Config error codes
enum remote_error_code
{
    /// No Error
    NO_ERROR = 0x0,
    /// Large element
    LARGE_ELEMENT,
    /// Invalid number of elements
    INVALID_ELMS_NUM,
    /// Unknown element id
    UNKNOWN_ELM_ID,
    /// Invalid element length
    INVALID_ELM_LEN,
    /// Invalid element data
    INVALID_ELM_DATA,
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable remote_config profile.
 * @param[in] conhdl	Connection handler
 * @return void.
 ****************************************************************************************
 */
void user_remote_config_enable(uint8_t conhdl);

 /**
 ****************************************************************************************
 * @brief Create Sample128 profile database
 * @return void.
 ****************************************************************************************
 */
void user_remote_config_create_db(void);

 /**
 ****************************************************************************************
 * @brief Initialization of remote_config profile.
 * @param[in]   *params_struct  Pointer to the parameter struct
 * @param[in]   params_num      Number of parameters
 * @param[in]   *version        Pointer to the version string        
 * @param[in]   version_length  Length of the version string
 * @param[in]   callback        Callback after the init has finished
 * @return void.
 ****************************************************************************************
 */
void user_remote_config_init(user_config_elem_t *params_struct, uint8_t params_num, char * version, uint8_t version_length, user_rem_config_function_t callback);

#endif //BLE_REMOTE_CONFIG

/// @}
#endif // USER_REMOTE_CONFIG_H_
