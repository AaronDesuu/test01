/**
 ****************************************************************************************
 *
 * @file user_config_common.h
 *
 * @brief Common application configuration data API.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 ****************************************************************************************
 */

 /**
 ****************************************************************************************
 * @addtogroup CONFIG_STORAGE
 * @{
 ****************************************************************************************
 */

#ifndef USER_CONFIG_COMMON_H_
#define USER_CONFIG_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include <stdint.h>
#include <stdbool.h>
#include "app.h" 
#include "co_bt.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// SPS Configuration Element type
typedef struct
{
    /// Element Id
    uint16_t        elem_id;
    /// Max size of element
    uint8_t         max_size;
    /// Current size of element
    uint8_t         cur_size;
    /// Pointer to element's data
    void *          data;
    /// Element's validation callback
    uint8_t         (*validation_cb)(uint8_t* param);
    /// Change flag
    bool            changed;
} user_config_elem_t;

/// SPS Configuration Struct type
typedef struct
{
    /// SPS Configuration Struct version
    char version[20];
    /// SPS Configuration Struct version length
    uint8_t version_length;
    /// Number of elements in SPS Configuration Struct
    uint8_t params_num;
    /// Pointer to Configuration Elements table
    user_config_elem_t   *params;
}user_configuration_struct_tag;

/// SPS Product Header type
typedef struct
{
    /// Product Header signature
    uint8_t signature[2];
    /// Product Header version
    uint8_t version[2];
    /// Start address of first image header
    uint32_t offset1;
    /// Start address of second image header
    uint32_t offset2;
    /// Start address of first Configuration Struct header
    uint32_t config_offset1;
    /// Start address of second Configuration Struct header
    uint32_t config_offset2;
}config_product_header_t;

extern char * spss_conf_struct_version;
extern uint8_t current_config_struct_index;
extern uint8_t current_config_struct_id;

#endif //USER_CONFIG_COMMON_H_

/// @} CONFIG_STORAGE

