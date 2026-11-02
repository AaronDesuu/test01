/**
 ****************************************************************************************
 *
 * @file user_config_storage.h
 *
 * @brief Application configuration data storage API header file
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @defgroup CONFIG_STORAGE Configuration Storage
 * @brief Configuration can be saved in Flash. This module provides read/write functions.
 * @addtogroup CONFIG_STORAGE
 * @{
 ****************************************************************************************
 */

#ifndef USER_CONFIG_STORAGE_H_
#define USER_CONFIG_STORAGE_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include <stdint.h>
#include <stdbool.h>
#include "app.h" 
#include "co_bt.h"
#include "spi.h"
#include "spi_flash.h"
#include "user_config_common.h"
/*
 * DEFINES
 ****************************************************************************************
 */

// Defines for image manipulation
/// PRODUCT_HEADER_SIGNATURE1
#ifndef PRODUCT_HEADER_SIGNATURE1
#define PRODUCT_HEADER_SIGNATURE1   0x70
#endif

/// PRODUCT_HEADER_SIGNATURE2
#ifndef PRODUCT_HEADER_SIGNATURE2
#define PRODUCT_HEADER_SIGNATURE2   0x52
#endif

/// CONFIGURATION_HEADER_SIGNATURE1
#define CONFIGURATION_HEADER_SIGNATURE1     0x70
/// CONFIGURATION_HEADER_SIGNATURE2
#define CONFIGURATION_HEADER_SIGNATURE2     0x54

/// STATUS_VALID_CONFIG
#define STATUS_VALID_CONFIG                 0xAA
/// STATUS_INVALID_CONFIG
#define STATUS_INVALID_CONFIG               0xFF

/// User config storage error codes
enum user_config_storage_error_codes
{
    /// Storage success
    USER_CONFIG_STORAGE_SUCCESS = 0,
    /// Configuration structure has crc error
    USER_CONFIG_STORAGE_CRC_ERR,
    /// Invalid product header
    USER_CONFIG_STORAGE_PRODUCT_HEADER_FAILURE,
    /// Failed to read Configuration header
    USER_CONFIG_STORAGE_CFG_HEADER_FAILURE,
    /// Failed to read Configuration data
    USER_CONFIG_STORAGE_READ_FAILURE,
    /// Failed to write Configuration structure
    USER_CONFIG_STORAGE_WRITE_FAILURE,
    /// Invalid data
    USER_CONFIG_STORAGE_INVALID_DATA,
    /// Failed to erase data
    USER_CONFIG_STORAGE_ERASE_FAILURE, 
};

/// SPS Configuration Struct Header type
typedef struct
{
    /// Configuration Struct signature
    uint8_t     signature[2];
    /// Configuration Struct valid flag
    uint8_t     valid;
    /// Configuration Struct Id
    uint8_t     struct_id;
    /// Configuration Struct CRC
    uint32_t    crc;
    /// Configuration Struct version
    uint8_t     version[16];
    /// Configuration Struct data size
    uint16_t    data_size;
    /// Number of elements in Configuration Struct
    uint8_t     num_of_items;
    /// Configuration Struct encryption flag
    uint8_t     encr;
    /// Configuration Struct reserved field
    uint8_t     reserved[36];
}user_config_struct_header_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initializes the Configuration parameters
 * @param[in]   conf_table                  Address of the Configuration parameters table
 * @param[in]   conf_table_num_of_elmts     Number of parameters contained in the Configuration table
 * @param[in]   version                     Address of the Configuration's version
 * @param[in]   version_length              Length of the Configuration's version
 * @return Configuration storage error code
 ****************************************************************************************
 */
enum user_config_storage_error_codes user_conf_storage_init(user_config_elem_t *conf_table, uint8_t conf_table_num_of_elmts, char *version, uint8_t *version_length);

/**
 ****************************************************************************************
 * @brief Reads the Configuration Structure stored in external memory
 * @param[in]   conf_data_pos               Address of the Configuration Structure in external memory
 * @param[in]   conf_table                  Address of the Configuration parameters table
 * @param[in]   conf_table_num_of_elmts     Number of parameters contained in the Configuration table
 * @return Configuration storage error code
 ****************************************************************************************
 */
enum user_config_storage_error_codes user_config_read_configuration_struct(uint32_t conf_data_pos,
                                                                           user_config_elem_t *conf_table,
                                                                           uint8_t conf_table_num_of_elmts);

/**
 ****************************************************************************************
 * @brief Saves the Configuration parameters to the extrenal memory
 * @param[in]   config_data     Address of the application structure that contains the Configuration parameters
 * @return Configuration storage error code
 ****************************************************************************************
 */
enum user_config_storage_error_codes user_config_storage_save_configuration(user_configuration_struct_tag* config_data);

/**
 ****************************************************************************************
 * @brief Writes Configuration data to external memory
 * @param[in]   config_data     Address of the application structure that contains the Configuration parameters
 * @param[in]   config_offset   Offset in the external memory
 * @param[in]   size            Size of data
 * @return negative error code if failed, else 0
 ****************************************************************************************
 */
enum user_config_storage_error_codes user_config_write_configuration_struct(user_configuration_struct_tag* config_data, uint32_t config_offset, uint32_t size);

#endif 

/// @} CONFIG_STORAGE
