/**
 ****************************************************************************************
 *
 * @file user_sps_device_config.h
 *
 * @brief DSPS Device project source code include file.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup USER_CONFIGURATION    User Configuration
 * @defgroup DSPS_DEVICE_CONFIG    DSPS Device Configuration
 * @addtogroup DSPS_DEVICE_CONFIG
 * @ingroup USER_CONFIGURATION
 * @brief DSPS Device Configuration Structures.
 * @{
 ****************************************************************************************
 */

#ifndef _USER_SPS_DEVICE_CONFIG_H_
#define _USER_SPS_DEVICE_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
/****************************************************************************
Add here supported profiles' application header files.
i.e.
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif
*****************************************************************************/

#include "user_config_sw_ver.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define SPSS_CONF_STRUCT_VER    "SPSS CONF v0"

enum user_sps_config_params_idx
{
    USER_SPS_PARAM_ADV_INTERVAL_MIN = 0,
    USER_SPS_PARAM_ADV_INTERVAL_MAX,
    USER_SPS_PARAM_ADV_CHANNEL_MAP,
    USER_SPS_PARAM_GAP_MTU,
    USER_SPS_PARAM_CONN_INTERVAL_MIN,
    USER_SPS_PARAM_CONN_INTERVAL_MAX,
    USER_SPS_PARAM_CONN_LATENCY,
    USER_SPS_PARAM_CONN_TIME_OUT,
    USER_SPS_PARAM_USER_ADV_DATA,
    USER_SPS_PARAM_USER_ADV_DATA_LEN,
    USER_SPS_PARAM_ADV_SCAN_RESP_DATA,
    USER_SPS_PARAM_ADV_SCAN_RESP_DATA_LEN,
    USER_SPS_PARAM_USER_DEVICE_NAME,
    USER_SPS_PARAM_DEVIC_NAME_LEN,
    USER_SPS_PARAM_PERIPH_UART_BAUDRATE,
    USER_SPS_GPIO_UART1_RX,
    USER_SPS_GPIO_UART1_TX,
    USER_SPS_GPIO_UART1_CTS,
    USER_SPS_GPIO_UART1_RTS,
    #if (EXTERNAL_WAKEUP)
    USER_SPS_GPIO_EXT_WAKEUP,
    USER_SPS_GPIO_EXT_WAKEUP_POLARITY,
    USER_SPS_GPIO_EXT_WAKEUP_DEBOUNCE,
    #endif
    USER_SPS_GPIO_ACTIVE_STATUS_PIN,
    #if WKUP_EXT_PROCESSOR
    USER_SPS_GPIO_WAKEUP_EXT_HOST,
    #endif
    USER_SPS_GPIO_POR_PIN,
    USER_SPS_GPIO_POR_PIN_POLARITY,
    USER_SPS_GPIO_POR_PIN_TIMEOUT,
    USER_CONF_ELEMENTS_NUM
};

extern char * spss_conf_struct_version;
extern uint8_t spss_conf_struct_version_len;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
#if BLE_REMOTE_CONFIG
/**
 ****************************************************************************************
 * @brief Takes actions according to the changed Configuration parameters and stores them to external memory
 * @param[in] config_data    Pointer to the latest Configuration parameters
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_apply_config(user_configuration_struct_tag* config_data);
#endif

/**
 ****************************************************************************************
 * @brief Set the UART parameters according to the selected baudrate
 * @return void
 ****************************************************************************************
 */
void user_sps_apply_uart_baudrate(void);

/**
 ****************************************************************************************
 * @brief Checks if the new Minimum Connection Interval value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_ci_min_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Maximum Connection Interval value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_ci_max_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Connection Latency value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_sl_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Connection Timeout value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_sup_to_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new MTU value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_mtu_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Minimum Advertising Interval value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_adv_int_min_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Maximum Advertising Interval value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_adv_int_max_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new Advertising Channel Map value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_adv_channel_map_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new UART Baudrate value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_uart_baudrate_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new assigned GPIO value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_gpio_config_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new GPIO Polarity value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_polarity_cb(uint8_t* param);

/**
 ****************************************************************************************
 * @brief Checks if the new POR Timeout value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_por_pin_timeout_cb(uint8_t* param);

#if (EXTERNAL_WAKEUP)
/**
 ****************************************************************************************
 * @brief Checks if the new External Wakeup Debounce value is valid
 * @param[in] param    Pointer to the new value
 * @return void
 ****************************************************************************************
 */
uint8_t user_sps_config_verify_ext_wakeup_debounce_cb(uint8_t* param);
#endif

/// @} DSPS_DEVICE_CONFIG

#endif //_USER_SPS_DEVICE_H_
