 /**
 ****************************************************************************************
 *
 * @file sps_client.h
 *
 * @brief Serial Port Service Client Header File
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup SPS_CLIENT    SPS Client Service
 * @ingroup SPS_SERVICES
 * @brief SPS Client profile database structure and initialization.
 * @{
 ****************************************************************************************
 */

#ifndef SPS_CLIENT_H_
#define SPS_CLIENT_H_


/// Serial Port Service Client Role
#define BLE_SPS_CLIENT          1
#if !defined (BLE_CLIENT_PRF)
    #define BLE_CLIENT_PRF      1
#endif 

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SPS_CLIENT)

#include "ke_task.h"
#include "prf.h"
#include "prf_types.h"

/*
 * Defines
 ****************************************************************************************
 */
#define SPSC_IDX_MAX 1

#define SPS_SERVICE_UUID    {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_SERVER_TX_UUID  {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_SERVER_RX_UUID  {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_FLOW_CTRL_UUID  {0xb9, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

#define SPS_CLIENT_CLEANUP_FNCT        (NULL)

/// SPS Service Characteristics
enum
{
    SPSC_SRV_TX_DATA_CHAR = 0,
    SPSC_SRV_RX_DATA_CHAR,
    SPSC_FLOW_CTRL_CHAR,

    SPSC_CHAR_MAX
};

/// SPS Service Characteristic Descriptors
enum
{
    /// SPS client config
    SPSC_SRV_TX_DATA_CLI_CFG,
    SPSC_FLOW_CTRL_CLI_CFG,
    
    SPSC_DESC_MAX,
};

/// Flow Control States type
typedef enum spsc_flow_ctrl_states
{
    /// Flow ON
    FLOW_ON     = (uint8_t)0x01,
    /// Flow OFF
    FLOW_OFF    = (uint8_t)0x02,
} spsc_flow_ctrl_state_t;

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Service information
struct spsc_sps_content
{
    /// Service info
    struct prf_svc svc;

    /// Characteristic info:
    struct prf_char_inf chars[SPSC_CHAR_MAX];

    /// Characteristic cfg
    struct prf_char_desc_inf descs[SPSC_DESC_MAX];
};

/// SPS Host environment variable
struct spsc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Service Start Handle
    uint16_t shdl;
    /// To store the DB max number of attributes
    uint8_t max_nb_att;
    /// On-going operation
    struct ke_msg *operation;

    /// Cursor on connection used to notify peer devices
    uint8_t cursor;
    /// CCC handle index, used during notification/indication busy state
    uint8_t ccc_idx;

    /// List of values set by application
    struct co_list values;
    /// CUSTS1 task state
    ke_state_t state[SPSC_IDX_MAX];
    
    /// Profile Connection Info

    /// Last requested UUID(to keep track of the two services and char)
    uint16_t last_uuid_req;
    /// Last service for which something was discovered
    uint8_t last_svc_req[ATT_UUID_128_LEN];

    /// Last char. code requested to read.
    uint8_t last_char_code;

    /// counter used to check service uniqueness
    uint8_t nb_svc;

    /// SPS Device Service information recovered from discovery
    struct spsc_sps_content sps;
    
    /// Tx flow control state
    bool tx_flow_en;
    /// Rx flow control state
    bool rx_flow_en;
    
    /// Updated mtu
    uint16_t mtu;
    /// Updated Tx size
    uint16_t tx_size;
    /// Updated Tx wait level
    uint16_t tx_wait_level;

    /// Pending TX
    struct ke_msg *pending_tx_msg;
    /// Write no rsp completion remaining
    uint8_t pending_wr_no_rsp_cmp;
    /// Pending notification of data tx
    bool pending_tx_ntf_cmp;
};

struct spsc_db_cfg
{
    ///Database configuration
    uint16_t features;
};

struct sps_counter 
{
    uint32_t spsc_ble_rx_data_bytes;
    uint32_t spsc_ble_tx_data_bytes;
};  


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

extern struct spsc_env_tag **spsc_envs;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief     Profile enable confirmation
 * @param[in] spsc_env  enviroment
 * @param[in] conidx    Connection index
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void sps_client_enable_cfm_send(struct spsc_env_tag *spsc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 * @param[in] spsc_env enviroment 
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void sps_client_confirm_data_tx(struct spsc_env_tag *spsc_env, uint8_t status);

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 * @param[in] spsc_env  enviroment 
 * @param[in] flow_ctrl 
 * @return    void
 ****************************************************************************************
 */
void sps_client_indicate_tx_flow_ctrl(struct spsc_env_tag *spsc_env, uint8_t flow_ctrl);

/**
 ****************************************************************************************
 * @brief  Find next empty characteristic description
 * @param[in] spsc_env   enviroment 
 * @param[in] desc_def  service characteristic description information table
 * @return    position of next characteristic description
 ****************************************************************************************
 */
uint8_t spsc_get_next_desc_char_code(struct spsc_env_tag *spsc_env,
                                     const struct prf_char_desc_def *desc_def);

/**
 ****************************************************************************************
 * @brief Retrieve SPS profile interface
 * @return SPS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* spsc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Receives data and forwards it to application
 * @param[in] spsc_env	enviroment
 * @param[in] msg		pointer to received message
 * @return    void
 ****************************************************************************************
 */
void sps_client_data_receive(struct spsc_env_tag *spsc_env, void *msg );

/**
 ****************************************************************************************
 * @brief Writes data to be transmitted in BLE
 * @param[in] msg       void pointer to SPS_CLIENT_DATA_TX_REQ message
 * @param[in] prf_env   spsc profile enviroment
 * @param[in] conidx    connection index
 * @param[in] handle    SPSC_SRV_RX_DATA_CHAR handle
 * @param[in] value     data pointer to be written
 * @param[in] length    length of data to be written
 * @param[in] operation GATTC_WRITE_NO_RESPONSE
 * @return    void
 ****************************************************************************************
 */
void sps_client_write_data_tx(void *msg, prf_env_t *prf_env, uint8_t conidx,
                                                         uint16_t handle, uint8_t* value, uint16_t length, uint8_t operation);

#endif //BLE_SPS_CLIENT

#endif // SPS_CLIENT_H_

/// @} SPS_CLIENT
