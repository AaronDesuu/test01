/**
 ****************************************************************************************
 *
 * @file user_config.h
 *
 * @brief User configuration file.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_DEVICE_CONFIG
 * @ingroup USER_CONFIGURATION
 * @{
 ****************************************************************************************
 */

#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_user_config.h"
#include "arch_api.h"
#include "app_default_handlers.h"
#include "app_adv_data.h"
#include "co_bt.h"

#define DEFAULT_ADVERTISE_INTERVAL      100
#define DEFAULT_ADDRESS_TYPE            APP_CFG_ADDR_PUB
#define DEFAULT_AUTH_REQUIREMENTS       GAP_AUTH_REQ_NO_MITM_BOND

/*
 * LOCAL VARIABLES
 ****************************************************************************************
 */

/*************************************************************************
 * Privacy Capabilities and address configuration of local device:
 * - APP_CFG_ADDR_PUB               No Privacy, Public BDA
 * - APP_CFG_ADDR_STATIC            No Privacy, Random Static BDA
 * - APP_CFG_HOST_PRIV_RPA          Host Privacy, RPA, Public Identity
 * - APP_CFG_HOST_PRIV_NRPA         Host Privacy, NRPA (non-connectable ONLY)
 * - APP_CFG_CNTL_PRIV_RPA_PUB      Controller Privacy, RPA or PUB, Public Identity
 * - APP_CFG_CNTL_PRIV_RPA_RAND     Controller Privacy, RPA, Public Identity
 *
 * Select only one option for privacy / addressing configuration.
 **************************************************************************
 */
#define USER_CFG_ADDRESS_MODE       APP_CFG_ADDR_PUB
//#define USER_CFG_ADDRESS_MODE       APP_CFG_ADDR_STATIC

/*************************************************************************
 * Controller Privacy Mode:
 * - APP_CFG_CNTL_PRIV_MODE_NETWORK Controler Privacy Network mode (default)
 * - APP_CFG_CNTL_PRIV_MODE_DEVICE  Controler Privacy Device mode
 *
 * Select only one option for controller privacy mode configuration.
 **************************************************************************
 */
#define USER_CFG_CNTL_PRIV_MODE     APP_CFG_CNTL_PRIV_MODE_NETWORK

/******************************************
 * Default sleep mode. Possible values are:
 *
 * - ARCH_SLEEP_OFF
 * - ARCH_EXT_SLEEP_ON
 * - ARCH_EXT_SLEEP_OTP_COPY_ON
 *
 ******************************************
 */
const static sleep_state_t app_default_sleep_mode = ARCH_SLEEP_OFF;

/*
 ****************************************************************************************
 *
 * Advertising or scan response data for the following cases:
 *
 * - ADV_IND: Connectable undirected advertising event.
 *    - The maximum length of the user defined advertising data shall be 28 bytes.
 *    - The Flags data type are written by the related ROM function, hence the user shall
 *      not include them in the advertising data. The related ROM function adds 3 bytes in 
 *      the start of the advertising data that are to be transmitted over the air.
 *    - The maximum length of the user defined response data shall be 31 bytes.
 *
 * - ADV_NONCONN_IND: Non-connectable undirected advertising event.
 *    - The maximum length of the user defined advertising data shall be 31 bytes.
 *    - The Flags data type may be omitted, hence the user can use all the 31 bytes for 
 *      data.
 *    - The scan response data shall be empty.
 *
 * - ADV_SCAN_IND: Scannable undirected advertising event.
 *    - The maximum length of the user defined advertising data shall be 31 bytes.
 *    - The Flags data type may be omitted, hence the user can use all the 31 bytes for 
 *      data.
 *    - The maximum length of the user defined response data shall be 31 bytes.
 ****************************************************************************************
 */
/// Advertising data
#define USER_ADVERTISE_DATA_COMPLETE_128BIT                 "\x11\x07\x66\x9A\x0C\x20\x00\x08\x96\x9E\xE2\x11\x9E\xB1\xE0\xF2\x73\xB9"
#if BLE_SUOTA_RECEIVER
#define USER_ADVERTISE_DATA_COMPLETE_16BIT                  "\x03"\
                                                            ADV_TYPE_COMPLETE_LIST_16BIT_SERVICE_IDS\
                                                            ADV_UUID_SUOTAR_SERVICE
#else
#define USER_ADVERTISE_DATA_COMPLETE_16BIT                  ""
#endif

#define USER_ADVERTISE_DATA                                 USER_ADVERTISE_DATA_COMPLETE_128BIT\
                                                            USER_ADVERTISE_DATA_COMPLETE_16BIT

/// Advertising data length - maximum 28 bytes, 3 bytes are reserved to set
#define USER_ADVERTISE_DATA_LEN               (sizeof(USER_ADVERTISE_DATA)-1)

/// Scan response data
#define USER_ADVERTISE_SCAN_RESPONSE_DATA     ""

/// Scan response data length- maximum 31 bytes
#define USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN (sizeof(USER_ADVERTISE_SCAN_RESPONSE_DATA)-1)

extern char user_advertise_data[28];
extern uint8_t user_advertise_data_len;
extern char user_adv_scan_resp_data[31];
extern uint8_t user_adv_scan_resp_data_len;
extern uint8_t uart_baudrate_param;

/*
 ****************************************************************************************
 *
 * Device name.
 *
 * - If there is space left in the advertising or scan response data the device name is
 *   copied there. The device name can be anytime read by a connected peer, if the
 *   application supports it.
 * - The Bluetooth device name can be up to 248 bytes.
 *
 ****************************************************************************************
 */

/// Device name. Either "SPS_531" or "SPS_585".
#if defined (__DA14531__) 
#define USER_DEVICE_NAME        "FT5AB"
#else
#define USER_DEVICE_NAME        "SPS_585"
#endif

/// Device name length
#define USER_DEVICE_NAME_LEN    (sizeof(USER_DEVICE_NAME)-1)

/// Default Connection Interval
#define DEFAULT_CONNECTION_INTERVAL     100
/// Default MTU
#define DEFAULT_MTU                     800

extern struct advertise_configuration user_adv_conf;
extern struct gapm_configuration user_gapm_conf;
extern struct connection_param_configuration user_connection_param_conf;
extern struct security_configuration user_security_conf;

/*
 ****************************************************************************************
 *
 * Central configuration (not used by current example)
 *
 ****************************************************************************************
 */
static const struct central_configuration user_central_conf = {
    /// GAPM requested operation:
    /// - GAPM_CONNECTION_DIRECT: Direct connection operation
    /// - GAPM_CONNECTION_AUTO: Automatic connection operation
    /// - GAPM_CONNECTION_SELECTIVE: Selective connection operation
    /// - GAPM_CONNECTION_NAME_REQUEST: Name Request operation (requires to start a direct
    ///   connection)
    .code = GAPM_CONNECTION_DIRECT,

    /// Own BD address source of the device:
    ///  - GAPM_STATIC_ADDR: Public or Private Static Address according to device address configuration
    ///  - GAPM_GEN_RSLV_ADDR: Generated resolvable private random address
    ///  - GAPM_GEN_NON_RSLV_ADDR: Generated non-resolvable private random address
    .addr_src = APP_CFG_ADDR_SRC(USER_CFG_ADDRESS_MODE),

    /// Scan interval
    .scan_interval = 0x180,

    /// Scan window size
    .scan_window = 0x160,

     /// Minimum of connection interval
    .con_intv_min = DEFAULT_CONNECTION_INTERVAL, //100,

    /// Maximum of connection interval
    .con_intv_max = DEFAULT_CONNECTION_INTERVAL, //100,

    /// Connection latency
    .con_latency = 0,

    /// Link supervision timeout
    .superv_to = 0x1F4,

     /// Minimum CE length
    .ce_len_min = 0,

    /// Maximum CE length
    .ce_len_max = 0x5,

    /**************************************************************************************
     * Peer device information (maximum number of peers = 8)
     **************************************************************************************
     */

    /// BD Address of device
    .peer_addr_0 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_0_type = 0,

    /// BD Address of device
    .peer_addr_1 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_1_type = 0,

    /// BD Address of device
    .peer_addr_2 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_2_type = 0,

    /// BD Address of device
    .peer_addr_3 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_3_type = 0,

    /// BD Address of device
    .peer_addr_4 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_4_type = 0,

    /// BD Address of device
    .peer_addr_5 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_5_type = 0,

    /// BD Address of device
    .peer_addr_6 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_6_type = 0,

    /// BD Address of device
    .peer_addr_7 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},

    /// Address type of the device 0=public/1=private random
    .peer_addr_7_type = 0,
};

/*
 ****************************************************************************************
 *
 * Default handlers configuration (applies only for @app_default_handlers.c)
 *
 ****************************************************************************************
 */
static const struct default_handlers_configuration  user_default_hnd_conf = {
    //Configure the advertise operation used by the default handlers
    //Possible values:
    //  - DEF_ADV_FOREVER
    //  - DEF_ADV_WITH_TIMEOUT
    .adv_scenario = DEF_ADV_FOREVER,

    //Configure the advertise period in case of DEF_ADV_WITH_TIMEOUT.
    //It is measured in timer units (3 min). Use MS_TO_TIMERUNITS macro to convert
    //from milliseconds (ms) to timer units.
    .advertise_period = MS_TO_TIMERUNITS(100),

    //Configure the security start operation of the default handlers
    //if the security is enabled (CFG_APP_SECURITY)
    .security_request_scenario = DEF_SEC_REQ_NEVER
};
#endif // _USER_CONFIG_H_

/// @} DSPS_DEVICE_CONFIG
