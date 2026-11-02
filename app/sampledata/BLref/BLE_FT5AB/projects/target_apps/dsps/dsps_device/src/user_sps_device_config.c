/**
 ****************************************************************************************
 *
 * @file user_sps_device_config.c
 *
 * @brief SPS Configuration parameters.
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
 * @{ 
 ****************************************************************************************
 */
  
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_APP_PRESENT)

#include "user_sps_device_config.h"
#include "user_sps_device_dma.h"
#include "user_periph_setup.h"

#ifdef CFG_CONFIG_STORAGE
#include "user_config_storage.h"
#endif

/*
 ****************************************************************************************
 *
 * Advertising configuration
 *
 ****************************************************************************************
 */
struct advertise_configuration user_adv_conf = {
    /**
     * Own BD address source of the device:
     * - GAPM_STATIC_ADDR: Public or Private Static Address according to device address configuration
     * - GAPM_GEN_RSLV_ADDR: Generated resolvable private random address
     * - GAPM_GEN_NON_RSLV_ADDR: Generated non-resolvable private random address
     */
    .addr_src = APP_CFG_ADDR_SRC(USER_CFG_ADDRESS_MODE),

    /// Minimum interval for advertising
    .intv_min = MS_TO_BLESLOTS(DEFAULT_ADVERTISE_INTERVAL),                    // 687.5ms

    /// Maximum interval for advertising
    .intv_max = MS_TO_BLESLOTS(DEFAULT_ADVERTISE_INTERVAL),                    // 687.5ms

    /**
     *  Advertising channels map:
     * - ADV_CHNL_37_EN:   Advertising channel map for channel 37.
     * - ADV_CHNL_38_EN:   Advertising channel map for channel 38.
     * - ADV_CHNL_39_EN:   Advertising channel map for channel 39.
     * - ADV_ALL_CHNLS_EN: Advertising channel map for channel 37, 38 and 39.
     */
    .channel_map = ADV_ALL_CHNLS_EN,

    /*************************
     * Advertising information
     *************************
     */

    /// Host information advertising data (GAPM_ADV_NON_CONN and GAPM_ADV_UNDIRECT)
    /// Advertising mode :
    /// - GAP_NON_DISCOVERABLE: Non discoverable mode
    /// - GAP_GEN_DISCOVERABLE: General discoverable mode
    /// - GAP_LIM_DISCOVERABLE: Limited discoverable mode
    /// - GAP_BROADCASTER_MODE: Broadcaster mode
    .mode = GAP_GEN_DISCOVERABLE,

    /// Host information advertising data (GAPM_ADV_NON_CONN and GAPM_ADV_UNDIRECT)
    /// - ADV_ALLOW_SCAN_ANY_CON_ANY: Allow both scan and connection requests from anyone
    /// - ADV_ALLOW_SCAN_WLST_CON_ANY: Allow both scan req from White List devices only and
    ///   connection req from anyone
    /// - ADV_ALLOW_SCAN_ANY_CON_WLST: Allow both scan req from anyone and connection req
    ///   from White List devices only
    /// - ADV_ALLOW_SCAN_WLST_CON_WLST: Allow scan and connection requests from White List
    ///   devices only
    .adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY,

    /// Direct address information (GAPM_ADV_DIRECT/GAPM_ADV_DIRECT_LDC)
    /// (used only if reconnection address isn't set or privacy disabled)
    /// BD Address of device
    .peer_addr = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6},

    /// Direct address information (GAPM_ADV_DIRECT/GAPM_ADV_DIRECT_LDC)
    /// (used only if reconnection address isn't set or privacy disabled)
    /// Address type of the device 0=public/1=private random
    .peer_addr_type = 0,
};
struct gapm_configuration user_gapm_conf = {
    /// Device Role: Central, Peripheral, Observer, Broadcaster or All roles. (@see enum gap_role)
    .role = GAP_ROLE_PERIPHERAL,

    /// Maximal MTU
    .max_mtu = DEFAULT_MTU, //247,

  /// Device Address Type
    .addr_type = APP_CFG_ADDR_TYPE(USER_CFG_ADDRESS_MODE),

    /***********************
     * Privacy configuration
     ***********************
     */

    /// Private static address
    // NOTE: The address shall comply with the following requirements:
    // - the two most significant bits of the address shall be equal to 1,
    // - all the remaining bits of the address shall NOT be equal to 1,
    // - all the remaining bits of the address shall NOT be equal to 0.
    // In case the {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} null address is used, a
    // random static address will be automatically generated.
    .addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    /// Duration before regenerate device address when privacy is enabled.
    .renew_dur = 15000,

    /// Device IRK used for resolvable random BD address generation (LSB first)
    .irk = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    /****************************
     * ATT database configuration
     ****************************
     */

    /// Attribute database configuration (@see enum gapm_att_cfg_flag)
    ///    7     6    5     4     3    2    1    0
    /// +-----+-----+----+-----+-----+----+----+----+
    /// | DBG | RFU | SC | PCP | APP_PERM |NAME_PERM|
    /// +-----+-----+----+-----+-----+----+----+----+
    /// - Bit [0-1]: Device Name write permission requirements for peer device (@see gapm_write_att_perm)
    /// - Bit [2-3]: Device Appearance write permission requirements for peer device (@see gapm_write_att_perm)
    /// - Bit [4]  : Slave Preferred Connection Parameters present
    /// - Bit [5]  : Service change feature present in GATT attribute database.
    /// - Bit [6]  : Reserved
    /// - Bit [7]  : Enable Debug Mode
    .att_cfg = GAPM_MASK_ATT_SVC_CHG_EN,

    /// GAP service start handle
    .gap_start_hdl = 0,

    /// GATT service start handle
    .gatt_start_hdl = 0,

    /**************************************************
     * Data packet length extension configuration (4.2)
     **************************************************
     */

    /// Maximal MPS
    .max_mps = 0,

    /// Maximal Tx octets & Tx time
#ifdef USE_DLE
    .max_txoctets = 251,
    .max_txtime = 2120,
#else
    .max_txoctets = 27,
    .max_txtime = 328
#endif
};

/*
 ****************************************************************************************
 *
 * Parameter update configuration
 *
 ****************************************************************************************
 */
/// Parameter update configuration
struct connection_param_configuration user_connection_param_conf = {
    /// Connection interval minimum measured in ble double slots (1.25ms)
    /// use the macro MS_TO_DOUBLESLOTS to convert from milliseconds (ms) to double slots
#ifdef USE_DLE
     .intv_min = MS_TO_DOUBLESLOTS(25),
#else
       .intv_min = MS_TO_DOUBLESLOTS(12.5),
#endif

    /// Connection interval maximum measured in ble double slots (1.25ms)
    /// use the macro MS_TO_DOUBLESLOTS to convert from milliseconds (ms) to double slots
#ifdef USE_DLE
     .intv_max = MS_TO_DOUBLESLOTS(25),
#else
       .intv_max = MS_TO_DOUBLESLOTS(12.5),
#endif

    /// Latency measured in connection events
    .latency = 0,

    /// Supervision timeout measured in timer units (10 ms)
    /// use the macro MS_TO_TIMERUNITS to convert from milliseconds (ms) to timer units
    .time_out = MS_TO_TIMERUNITS(10000),

    /// Minimum Connection Event Duration measured in ble double slots (1.25ms)
    /// use the macro MS_TO_DOUBLESLOTS to convert from milliseconds (ms) to double slots
    .ce_len_min = MS_TO_DOUBLESLOTS(40),

    /// Maximum Connection Event Duration measured in ble double slots (1.25ms)
    /// use the macro MS_TO_DOUBLESLOTS to convert from milliseconds (ms) to double slots
    .ce_len_max = MS_TO_DOUBLESLOTS(40),
};

#if (BLE_APP_SEC)
/*
 ****************************************************************************************
 *
 * Security related configuration
 *
 ****************************************************************************************
 */
struct security_configuration user_security_conf = {
    /**************************************************************************************
     * IO capabilities (@see gap_io_cap)
     *
     * - GAP_IO_CAP_DISPLAY_ONLY          Display Only
     * - GAP_IO_CAP_DISPLAY_YES_NO        Display Yes No
     * - GAP_IO_CAP_KB_ONLY               Keyboard Only
     * - GAP_IO_CAP_NO_INPUT_NO_OUTPUT    No Input No Output
     * - GAP_IO_CAP_KB_DISPLAY            Keyboard Display
     *
     **************************************************************************************
     */
    .iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,

    /**************************************************************************************
     * OOB information (@see gap_oob)
     *
     * - GAP_OOB_AUTH_DATA_NOT_PRESENT    OOB Data not present
     * - GAP_OOB_AUTH_DATA_PRESENT        OOB data present
     *
     **************************************************************************************
     */
    .oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT,

    /**************************************************************************************
     * Authentication (@see gap_auth)
     *
     * - GAP_AUTH_REQ_NO_MITM_NO_BOND     No MITM No Bonding
     * - GAP_AUTH_REQ_NO_MITM_BOND        No MITM Bonding
     * - GAP_AUTH_REQ_MITM_NO_BOND        MITM No Bonding
     * - GAP_AUTH_REQ_MITM_BOND           MITM and Bonding
     *
     **************************************************************************************
     */
    .auth           = DEFAULT_AUTH_REQUIREMENTS, //GAP_AUTH_REQ_NO_MITM_BOND,

    /**************************************************************************************
     * Device security requirements (minimum security level). (@see gap_sec_req)
     *
     * - GAP_NO_SEC                       No security (no authentication and encryption)
     * - GAP_SEC1_NOAUTH_PAIR_ENC         Unauthenticated pairing with encryption
     * - GAP_SEC1_AUTH_PAIR_ENC           Authenticated pairing with encryption
     * - GAP_SEC2_NOAUTH_DATA_SGN         Unauthenticated pairing with data signing
     * - GAP_SEC2_AUTH_DATA_SGN           Authentication pairing with data signing
     * - GAP_SEC_UNDEFINED                Unrecognized security
     *
     **************************************************************************************
     */
    .sec_req        = GAP_NO_SEC,

     /// Encryption key size (7 to 16) - LTK Key Size
    .key_size       = KEY_LEN,

    /**************************************************************************************
     * Initiator key distribution (@see gap_kdist)
     *
     * - GAP_KDIST_NONE                   No Keys to distribute
     * - GAP_KDIST_ENCKEY                 LTK (Encryption key) in distribution
     * - GAP_KDIST_IDKEY                  IRK (ID key)in distribution
     * - GAP_KDIST_SIGNKEY                CSRK (Signature key) in distribution
     * - Any combination of the above
     *
     **************************************************************************************
     */
    .ikey_dist      = GAP_KDIST_SIGNKEY,

    /**************************************************************************************
     * Responder key distribution (@see gap_kdist)
     *
     * - GAP_KDIST_NONE                   No Keys to distribute
     * - GAP_KDIST_ENCKEY                 LTK (Encryption key) in distribution
     * - GAP_KDIST_IDKEY                  IRK (ID key)in distribution
     * - GAP_KDIST_SIGNKEY                CSRK (Signature key) in distribution
     * - Any combination of the above
     *
     **************************************************************************************
     */
    .rkey_dist      = GAP_KDIST_ENCKEY,
};
#endif

char user_advertise_data[28] = USER_ADVERTISE_DATA;
uint8_t user_advertise_data_len = USER_ADVERTISE_DATA_LEN;
char user_adv_scan_resp_data[31] = USER_ADVERTISE_SCAN_RESPONSE_DATA;
uint8_t user_adv_scan_resp_data_len = USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN;
uint8_t uart_baudrate_param = BAUDRATE_CONFIG;

char * spss_conf_struct_version = SPSS_CONF_STRUCT_VER;
uint8_t spss_conf_struct_version_len = sizeof(SPSS_CONF_STRUCT_VER)-1;

gpio_func_t gpio_uart1_rx = {GPIO_UART1_RX_PIN, GPIO_UART1_RX_PORT};
gpio_func_t gpio_uart1_tx = {GPIO_UART1_TX_PIN, GPIO_UART1_TX_PORT};
gpio_func_t gpio_uart1_cts = {GPIO_UART1_CTS_PIN, GPIO_UART1_CTS_PORT};
gpio_func_t gpio_uart1_rts = {GPIO_UART1_RTS_PIN, GPIO_UART1_RTS_PORT};
#if (EXTERNAL_WAKEUP)
gpio_func_t gpio_ext_wakeup = {EXTERNAL_WAKEUP_GPIO_PIN, EXTERNAL_WAKEUP_GPIO_PORT};
uint8_t gpio_ext_wakeup_polarity = 0;
uint16_t gpio_ext_wakeup_debounce = 0;
#endif

gpio_func_t gpio_reset_status = {GPIO_ACTIVE_STATUS_PIN, GPIO_ACTIVE_STATUS_PORT};

#if WKUP_EXT_PROCESSOR
gpio_func_t gpio_wakeup_ext_host = {WKUP_EXT_PROC_GPIO_PIN, WKUP_EXT_PROC_GPIO_PORT};
#endif
gpio_func_t gpio_por_pin = {GPIO_POR_PIN, GPIO_POR_PORT};
uint8_t gpio_por_pin_polarity = GPIO_POR_PIN_POLARITY_HIGH;
uint8_t gpio_por_pin_timeout = 0;
#if BLE_REMOTE_CONFIG
bool mtu_reset = false;
#endif

/// SPS Server Configuration Elements table
user_config_elem_t    spss_configuration_table[USER_CONF_ELEMENTS_NUM] = {
                                                {ELEM_ID_ADV_INTERVAL_MIN, 2, 2, &user_adv_conf.intv_min, user_sps_config_verify_adv_int_min_cb, false},
                                                {ELEM_ID_ADV_INTERVAL_MAX, 2, 2, &user_adv_conf.intv_max, user_sps_config_verify_adv_int_max_cb, false},
                                                {ELEM_ID_ADV_CHANNEL_MAP, 1, 1, &user_adv_conf.channel_map, user_sps_config_verify_adv_channel_map_cb, false},
                                                {ELEM_ID_GAP_MTU, 2, 2, &user_gapm_conf.max_mtu, user_sps_config_verify_mtu_cb, false},
                                                {ELEM_ID_CONN_INTERVAL_MIN, 2, 2, &user_connection_param_conf.intv_min, user_sps_config_verify_ci_min_cb, false},
                                                {ELEM_ID_CONN_INTERVAL_MAX, 2, 2, &user_connection_param_conf.intv_max, user_sps_config_verify_ci_max_cb, false},
                                                {ELEM_ID_CONN_LATENCY, 2, 2, &user_connection_param_conf.latency, user_sps_config_verify_sl_cb, false},
                                                {ELEM_ID_CONN_TIME_OUT, 2, 2, &user_connection_param_conf.time_out, user_sps_config_verify_sup_to_cb, false},
                                                {ELEM_ID_USER_ADV_DATA, 28, USER_ADVERTISE_DATA_LEN, user_advertise_data, NULL, false},
                                                {ELEM_ID_USER_ADV_DATA_LEN, 1, 1, &user_advertise_data_len, NULL, false},
                                                {ELEM_ID_USER_ADV_SCAN_RESP_DATA, 31, USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN, NULL, NULL, false},
                                                {ELEM_ID_USER_ADV_SCAN_RESP_DATA_LEN, 1, 1, &user_adv_scan_resp_data_len, NULL, false},
                                                {ELEM_ID_USER_DEVICE_NAME, 32, USER_DEVICE_NAME_LEN, device_info.dev_name.name, NULL, false},
                                                {ELEM_ID_USER_DEVICE_NAME_LEN, 1, 1, &device_info.dev_name.length, NULL, false},
                                                {ELEM_ID_PERIPH_UART_BAUDRATE, 1, 1, &uart_baudrate_param, user_sps_config_verify_uart_baudrate_cb, false},
                                                {ELEM_ID_GPIO_UART1_RX, 2, 2, &gpio_uart1_rx, user_sps_config_verify_gpio_config_cb, false},
                                                {ELEM_ID_GPIO_UART1_TX, 2, 2, &gpio_uart1_tx, user_sps_config_verify_gpio_config_cb, false},
                                                {ELEM_ID_GPIO_UART1_CTS, 2, 2, &gpio_uart1_cts, user_sps_config_verify_gpio_config_cb, false},
                                                {ELEM_ID_GPIO_UART1_RTS, 2, 2, &gpio_uart1_rts, user_sps_config_verify_gpio_config_cb, false},
#if (EXTERNAL_WAKEUP)
                                                {ELEM_ID_GPIO_EXT_WAKEUP, 2, 2, &gpio_ext_wakeup, user_sps_config_verify_gpio_config_cb, false},
                                                {ELEM_ID_GPIO_EXT_WAKEUP_POLARITY, 1, 1, &gpio_ext_wakeup_polarity, user_sps_config_verify_polarity_cb, false},
                                                {ELEM_ID_GPIO_EXT_WAKEUP_DEBOUNCE, 1, 1, &gpio_ext_wakeup_debounce, user_sps_config_verify_ext_wakeup_debounce_cb, false},
#endif
                                                {ELEM_ID_GPIO_ACTIVE_STATUS_PIN, 2, 2, &gpio_reset_status, user_sps_config_verify_gpio_config_cb, false},
#if WKUP_EXT_PROCESSOR
                                                {ELEM_ID_GPIO_WAKEUP_EXT_HOST, 2, 2, &gpio_wakeup_ext_host, user_sps_config_verify_gpio_config_cb, false},
#endif
                                                {ELEM_ID_GPIO_POR_PIN, 2, 2, &gpio_por_pin, user_sps_config_verify_gpio_config_cb, false},
                                                {ELEM_ID_GPIO_POR_PIN_POLARITY, 1, 1, &gpio_por_pin_polarity, user_sps_config_verify_polarity_cb, false},
                                                {ELEM_ID_GPIO_POR_PIN_TIMEOUT, 1, 1, &gpio_por_pin_timeout, user_sps_config_verify_por_pin_timeout_cb, false},
};

#if BLE_REMOTE_CONFIG
/**
 ****************************************************************************************
 * @brief Checks if a GPIO related Configuration parameter has changed
 * @param[in] config_data    Pointer to the latest Configuration parameters table
 * @return True if there is a change, else False
 ****************************************************************************************
 */
static uint8_t user_sps_check_gpio_conf_change(user_config_elem_t* params)
{
    for (int i = USER_SPS_GPIO_UART1_RX; i <= USER_SPS_GPIO_POR_PIN_TIMEOUT; i++)
    {
        if (spss_configuration_table[i].changed)
        {
            return 1;
        }
    }
    return 0;
}

uint8_t user_sps_apply_config(user_configuration_struct_tag* config_data)
{
	uint16_t i;
    uint8_t status = 0;
    uint8_t reset = 0;
#ifdef CFG_CONFIG_STORAGE
    status = user_config_storage_save_configuration(config_data);
#endif
    if (status == 0)
    {
        for (int i = USER_SPS_PARAM_CONN_INTERVAL_MIN; i <= USER_SPS_PARAM_CONN_TIME_OUT; i++)
        {
            if (spss_configuration_table[i].changed)
            {
                app_easy_gap_param_update_start(app_env[0].conidx);
                break;
            }
        }

        if( spss_configuration_table[USER_SPS_PARAM_GAP_MTU].changed)
        {
            mtu_reset = true;
        }

        if (user_sps_check_gpio_conf_change(config_data->params))
            reset = 1;

        if (spss_configuration_table[USER_SPS_PARAM_PERIPH_UART_BAUDRATE].changed)
        {
            reset = 1;
        }

        if (reset)
        {
            platform_reset(RESET_NO_ERROR);
        }

        for (i=0; i < USER_CONF_ELEMENTS_NUM; i++)
        {
            spss_configuration_table[i].changed = false;
        }
    }
#ifdef CFG_CONFIG_STORAGE
    else
    {
        user_conf_storage_init((user_config_elem_t *)spss_configuration_table, sizeof(spss_configuration_table)/sizeof(user_config_elem_t),
        spss_conf_struct_version, &spss_conf_struct_version_len);
    }
#endif
    return status;
}
#endif

void user_sps_apply_uart_baudrate(void)
{
    cfg_uart_sps_baudrate = uart_baudrate_t[uart_baudrate_param];
    cfg_uart_sps_baud = uart_baud_t[uart_baudrate_param];
    // 8 bits + start bit + stop bit, uart_wait_byte_time in usec
    uart_wait_byte_time = (1000000/cfg_uart_sps_baud) * 10;
    // In order to calculate a wait for loop equal to 1 byte duration.
    // Duration of 1 iteration in a for loop is 0.25 usec
    uart_wait_byte_counter = (uart_wait_byte_time * 4) + 30;
}

/**
 ****************************************************************************************
 * @brief Calculates the interval between two successive connection events
 * @param[in] latency    The connection latency parameter
 * @param[in] intv_max   The maximum connection interval parameter
 * @return void
 ****************************************************************************************
 */
static uint32_t calc_total_interval(uint16_t latency, uint16_t intv_max)
{
    return (uint32_t)((((1 + latency) * intv_max * 2 * 5)/4) + 1);
}

uint8_t user_sps_config_verify_ci_min_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);

    if ((val < CON_INT_MIN) || (val > CON_INT_MAX) || (val > user_connection_param_conf.intv_max))
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_ci_max_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);
    uint32_t interval = calc_total_interval(user_connection_param_conf.latency, val);

    if ((val < CON_INT_MIN) || (val > CON_INT_MAX) || (interval >= (user_connection_param_conf.time_out * 10)) || (val < user_connection_param_conf.intv_min))
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_sl_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);
    uint32_t interval = calc_total_interval(val, user_connection_param_conf.intv_max);

    if ((val > SL_MAX) || ( interval >= (user_connection_param_conf.time_out * 10)))
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_sup_to_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);
    uint32_t interval = calc_total_interval(user_connection_param_conf.latency, user_connection_param_conf.intv_max);

    if ((val < SUP_TO_MIN || val > SUP_TO_MAX) || ( interval >= (val * 10)))
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_mtu_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);

    if (val < MTU_MIN || val > MTU_MAX)
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_adv_int_min_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);

    if (val < ADV_INT_MIN || val > ADV_INT_MAX || val > user_adv_conf.intv_max)
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_adv_int_max_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);

    if (val < ADV_INT_MIN || val > ADV_INT_MAX || val < user_adv_conf.intv_min)
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_adv_channel_map_cb(uint8_t* param)
{
    if (param[0] > ADV_ALL_CHNLS_EN)
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_uart_baudrate_cb(uint8_t* param)
{
    if (param[0] < 1 || param[0] > 8)
        return 1;
    else return 0;
}

uint8_t user_sps_config_verify_gpio_config_cb(uint8_t* param)
{
    if (param[1] > 1 || param[0] > 11)
        return 1;
    if (param[0] == 0 || param[0] == 1 || param[0] == 3 || param[0] == 4)
        return 1;
    return 0;
}

uint8_t user_sps_config_verify_polarity_cb(uint8_t* param)
{
    if (param[0] > 1)
        return 1;
    else return 0;
}


uint8_t user_sps_config_verify_por_pin_timeout_cb(uint8_t* param)
{
    if (param[0] > 0x7F)
        return 1;
    else return 0;
}

#if (EXTERNAL_WAKEUP)
uint8_t user_sps_config_verify_ext_wakeup_debounce_cb(uint8_t* param)
{
    uint16_t val;
    val = (uint16_t)param[0] | (((uint16_t)param[1])<<8);

    if (val > 0x3F)
        return 1;
    else return 0;
}
#endif

#endif //BLE_APP_PRESENT

/// @} DSPS_DEVICE_CONFIG
