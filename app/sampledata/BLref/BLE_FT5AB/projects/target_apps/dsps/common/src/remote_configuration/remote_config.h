/**
 ****************************************************************************************
 *
 * @file remote_config.h
 *
 * @brief Remote Configuration Service header
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @defgroup REMOTE_CONFIG_GATT_SERVICE Remote Configuration Service
 * @ingroup GATT_SERVICES
 * @brief   Remote Config Module GATT Service
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @addtogroup REMOTE_CONFIG_GATT_SERVICE
 * @{
 ****************************************************************************************
 */

#ifndef REMOTE_CONFIG_H_
#define REMOTE_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#define BLE_REMOTE_CONFIG           1
#if !defined (BLE_SERVER_PRF)
    #define BLE_SERVER_PRF          1
#endif
 
#if (BLE_REMOTE_CONFIG)

#include "ke_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "rwprf_config.h"
#include "gattc.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// Remote Configuration Service Characteristics UUIDs and Descriptions
#define REMOTE_CONFIG_SERVICE_UUID  {0xa0, 0xc6, 0x53, 0x27, 0x76, 0xea, 0xd1, 0x80, 0xc1, 0x42, 0x3d, 0x0a, 0x9b, 0xa3, 0x47, 0xa2}
#define REM_CONF_CHAR_LEN      250

//Version
#define REM_VERSION_CHAR_UUID   {0xa1, 0xc6, 0x53, 0x27, 0x76, 0xea, 0xd1, 0x80, 0xc1, 0x42, 0x3d, 0x0a, 0x9b, 0xa3, 0x47, 0xa2}
#define REM_VERSION_MAX_SIZE    66
#define REM_VERSION_DESC        "Remote Version"

//Discover Elements
#define REM_DISCOVER_ELM_CHAR_UUID    {0xa2, 0xc6, 0x53, 0x27, 0x76, 0xea, 0xd1, 0x80, 0xc1, 0x42, 0x3d, 0x0a, 0x9b, 0xa3, 0x47, 0xa2}
#define REM_DISCOVER_ELM_MAX_SIZE           130
#define REM_DISCOVER_ELM_WRITE_MAX_LEN      1
#define REM_DISCOVER_ELM_WRITE_MAX_VAL      64
#define REM_DISCOVER_ELM_DESC    "Discover elements cmd"

//Write Elements
#define REM_WRITE_ELM_CHAR_UUID  {0xa3, 0xc6, 0x53, 0x27, 0x76, 0xea, 0xd1, 0x80, 0xc1, 0x42, 0x3d, 0x0a, 0x9b, 0xa3, 0x47, 0xa2}
#define REM_WRITE_ELM_MAX_SIZE              133
#define REM_WRITE_ELM_MIN_LEN               3
#define REM_WRITE_ELM_DESC       "Write elements Command"

//Read Elements
#define REM_READ_ELM_CHAR_UUID  {0xa4, 0xc6, 0x53, 0x27, 0x76, 0xea, 0xd1, 0x80, 0xc1, 0x42, 0x3d, 0x0a, 0x9b, 0xa3, 0x47, 0xa2}
#define REM_READ_ELM_MAX_SIZE               132
#define REM_READ_ELM_MIN_LEN                2
#define REM_READ_ELM_DESC       "Read elements Command"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Handles offsets
enum
{
    REMOTE_CONFIG_IDX_SVC,          //0
    
    /// Version
    REMOTE_VERSION_IDX_CHAR,        //1
    REMOTE_VERSION_IDX_VAL,         //2
    REMOTE_VERSION_IDX_CFG,         //3
    REMOTE_VERSION_IDX_DESC,        //4
    
    /// Discover elm
    REMOTE_DISCOVER_ELM_IDX_CHAR,	//5
    REMOTE_DISCOVER_ELM_IDX_VAL,	//6
    REMOTE_DISCOVER_ELM_IDX_CFG,	//7
    REMOTE_DISCOVER_ELM_IDX_DESC,	//8

    /// Write elm
    REMOTE_WRITE_ELM_IDX_CHAR,      //9
    REMOTE_WRITE_ELM_IDX_VAL,       //10
    REMOTE_WRITE_ELM_IDX_CFG,       //11
    REMOTE_WRITE_ELM_IDX_DESC,      //12

    /// Read elm
    REMOTE_READ_ELM_IDX_CHAR,       //13
    REMOTE_READ_ELM_IDX_VAL,        //14
    REMOTE_READ_ELM_IDX_CFG,        //15
    REMOTE_READ_ELM_IDX_DESC,       //16
    
    REMOTE_CONFIG_IDX_NB            //17
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Remote configuration environment variable
struct remote_config_env_tag
{       
    /// profile environment
    prf_env_t prf_env;
    
    /// connection index
    uint8_t  conidx;
    
    /// On-going operation
    struct ke_msg * operation; 

    /// Remote Config svc Start Handle
    uint16_t remote_config_shdl;
    
    /// To store the DB max number of attributes
    uint8_t max_nb_att;
    
    /// List of values set by application
    struct co_list values;
       
    /// Notification configuration of Discover characteristic
    uint8_t discover_ntf_cfg;

    /// Notification configuration of Write characteristic
    uint8_t write_ntf_cfg;

    /// Notification configuration of Read characteristic
    uint8_t read_ntf_cfg;

    /// Connection status
    bool connection_active;
};

/// Remote configuration database configuration struct
struct remote_config_db_cfg
{
    ///Max number of attributes
    uint8_t max_nb_att;
    ///Attribute's table
    uint8_t *att_tbl;
    ///Configuration flag
    uint8_t *cfg_flag;
    ///Database configuration
    uint16_t features;
};

/*
 *  REMOTE_CONFIG PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */
extern struct remote_config_env_tag remote_config_env;

static const struct att_uuid_128      remote_config_service = {REMOTE_CONFIG_SERVICE_UUID};

static const struct att_char128_desc  rem_version_char  = {ATT_CHAR_PROP_RD, {0,0}, REM_VERSION_CHAR_UUID};  
static const struct att_uuid_128      rem_version_val = {REM_VERSION_CHAR_UUID};

static const struct att_char128_desc  rem_discover_elm_char  = {ATT_CHAR_PROP_WR|ATT_CHAR_PROP_NTF, {0,0}, REM_DISCOVER_ELM_CHAR_UUID};
static const struct att_uuid_128      rem_discover_elm_val = {REM_DISCOVER_ELM_CHAR_UUID};

static const struct att_char128_desc  rem_write_elm_char  = {ATT_CHAR_PROP_WR|ATT_CHAR_PROP_NTF, {0,0}, REM_WRITE_ELM_CHAR_UUID};
static const struct att_uuid_128      rem_write_elm_val= {REM_WRITE_ELM_CHAR_UUID};
                                      
static const struct att_char128_desc  rem_read_elm_char  = {ATT_CHAR_PROP_WR|ATT_CHAR_PROP_IND, {0,0}, REM_READ_ELM_CHAR_UUID};
static const struct att_uuid_128      rem_read_elm_val = {REM_READ_ELM_CHAR_UUID};


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve REMOTE_CONFIG service profile interface
 * @return REMOTE_CONFIG service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* remote_config_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Service and attribute index from attribute handle
 * @param[in] handle    attribute handle 
 * @param[in] att_idx   attribute index 
 * @return Success if attribute and service index found, else Application error 
 ****************************************************************************************
 */
uint8_t remote_config_get_att_idx(uint16_t handle, uint8_t *att_idx); 

#endif //BLE_REMOTE_CONFIG

#endif // REMOTE_CONFIG_H_

/// @} REMOTE_CONFIG_GATT_SERVICE
