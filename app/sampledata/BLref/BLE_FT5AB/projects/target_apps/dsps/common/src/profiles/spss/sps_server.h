/**
 ****************************************************************************************
 *
 * @file sps_server.h
 *
 * @brief Header file - SPS Server header file
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GATT_SERVICES      GATT Services
 * @brief GATT Services
 * @defgroup SPS_SERVICES       SPS Services
 * @ingroup GATT_SERVICES
 * @brief SPS Services
 * @defgroup SPS_SERVER         SPS Server Service
 * @ingroup SPS_SERVICES
 * @brief SPS Server profile database structure and initialization.
 * @{
 ****************************************************************************************
 */

#ifndef SPS_SERVER_H_
#define SPS_SERVER_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SPS_SERVER)

#include <stdint.h>
#include "prf_types.h"
#include "prf.h"
#include "attm_db_128.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
 #define SPSS_IDX_MAX        (1)
 #define TX_NTF_SEQUENCE_NUMBER 0xF3C2

///Attribute Table Indexes
enum
{
    SPSS_TX_DATA_CHAR,
    SPSS_RX_DATA_CHAR,
    SPSS_FLOW_CTRL_CHAR,

    SPSS_CHAR_MAX,
};

/// Flow Control States type
typedef enum spss_flow_ctrl_states
{
    /// Flow ON
    FLOW_ON     = (uint8_t)0x01,
    /// Flow OFF
    FLOW_OFF    = (uint8_t)0x02,
} spss_flow_ctrl_state_t;


/*
 * STRUCTURES
 ****************************************************************************************
 */

///SPS Server environment variable
struct spss_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Service Start Handle
    uint16_t shdl;
    /// To store the DB max number of attributes
    uint8_t max_nb_att;
    /// On-going operation
    struct ke_msg *operation;
    
    /// Connection Index
    uint8_t conidx;
    
    /// Peer supports DLE
    uint8_t peer_supports_dle;  

    /// Cursor on connection used to notify peer devices
    uint8_t cursor;
    /// CCC handle index, used during notification/indication busy state
    uint8_t ccc_idx;

    /// List of values set by application
    struct co_list values;
    /// CUSTS1 task state
    ke_state_t state[SPSS_IDX_MAX];
    
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

    /// Pending Tx
    bool pending_tx;
    /// Notification completion remaining
    uint8_t pending_ntf_cmp;
    /// Pending notification of data tx
    bool pending_tx_ntf_cmp;
    /// Tx busy flag
    bool tx_busy_flag;

};

///SPS Server database configuration
struct spss_db_cfg
{
    ///Max number of casts1 service characteristics
    uint8_t max_nb_att;
    ///Attributes table
    uint8_t *att_tbl;
    ///Configuration flag
    uint8_t *cfg_flag;
    ///Database configuration
    uint16_t features;
};

///SPS Server value element
struct sps_val_elmt
{
    /// list element header
    struct co_list_hdr hdr;
    /// value identifier
    uint8_t att_idx;
    /// value length
    uint8_t length;
    /// value data
    uint8_t data[__ARRAY_EMPTY];
};

/// SPS statistics
struct sps_counter 
{
    /// SPSS BLE received data bytes
    uint32_t spss_ble_rx_data_bytes;
    /// SPSS BLE transmitted data bytes
    uint32_t spss_ble_tx_data_bytes;
    /// SPSS BLE transmitted successful notifications
    uint32_t spss_ble_tx_ntf_ok;    
    /// SPSS BLE transmitted failed notifications
    uint32_t spss_ble_tx_ntf_fail;    
    /// SPSS BLE transmitted all GATT complete event messages
    uint32_t spss_ble_tx_cmp;
    /// SPSS BLE transmitted successful GATT complete event messages
    uint32_t spss_ble_tx_cmp_ok;  
    /// SPSS BLE transmitted failed GATT complete event messages
    uint32_t spss_ble_tx_cmp_other;  
    /// SPSS bytes transferred from UART to BLE
    uint32_t spss_uart_to_ble_bytes;    
};  

/*
 *  SPS PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
//GEZA
//extern struct spss_env_tag spss_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

 /**
 ****************************************************************************************
 * @brief Create and initialize the SPS database and environment
 * @param[in] env       Pointer to the profile task environment
 * @param[in] start_hdl Pointer to the start handle
 * @param[in] app_task  Application Task Number
 * @param[in] sec_lvl   Security Level
 * @param[in] params    SPS parameters
 * @return success or error
 ****************************************************************************************
 */
uint8_t sps_init(struct prf_task_env *env, uint16_t *start_hdl, uint16_t app_task, uint8_t sec_lvl, struct spss_db_cfg *params);

/**
 ****************************************************************************************
 * @brief  Send data to app
 * @param[in]        data       Pointer to data
 * @param[in]        length     Length of data
 * @param[in]        conidx     Connection Index
 * @return  void
 ****************************************************************************************
 */
void sps_server_indicate_data_rx(const uint8_t *data, uint16_t length, uint8_t conidx);

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 * @param[in]   flow_ctrl  flow control (FLOW_ON, FLOW OFF)
 * @param[in]   conidx     length of data
 * @return  void
 ****************************************************************************************
 */
void sps_server_indicate_tx_flow_ctrl(uint8_t flow_ctrl, uint8_t conidx);

/**
 ****************************************************************************************
 * @brief  Send event to peer device, this function allocates a new message
 * @param[in]   att_idx     attribute index
 * @param[in]   conidx      connection index
 * @param[in]   p_data      pointer to data
 * @param[in]   data_len    data length
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int sps_server_send_event( uint16_t att_idx, uint8_t conidx, uint8_t *p_data, uint16_t data_len );

/**
 ****************************************************************************************
 * @brief Set value of CCC for given attribute and connection index.
 * @param[in] conidx   Connection index.
 * @param[in] att_idx  CCC attribute index.
 * @param[in] ccc       Value to store.
 * @return  void
 ****************************************************************************************
 */
void sps_set_ccc_value(uint8_t conidx, uint8_t att_idx, uint16_t ccc);

/**
 ****************************************************************************************
 * @brief Sets initial values for all Clinet Characteristic Configurations.
 * @param[in]  att_db     Custom service attribute definition table.
 * @param[in]  max_nb_att Number of elements in att_db.
 * @return  void
 ****************************************************************************************
 */
void sps_init_ccc_values(const struct attm_desc_128 *att_db, int max_nb_att);

/**
 ****************************************************************************************
 * @brief Retrieve SPS profile interface
 * @return SPS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* spss_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief  Gets att index from handle
 * @param[in] handle    Connection handle
 * @param[in] att_idx   attribute index
 * @return success or error
 ****************************************************************************************
 */
uint8_t sps_get_att_idx(uint16_t handle, uint8_t *att_idx);

/**
 ****************************************************************************************
 * @brief Read value of CCC for given attribute and connection index.
 * @param[in]  conidx   Connection index.
 * @param[in]  att_idx  Custom attribute index.
 * @return Value of CCC.
 ****************************************************************************************
 */
uint16_t sps_get_ccc_value(uint8_t conidx, uint8_t att_idx);

/**
 ****************************************************************************************
 * @brief Read characteristic value from. Function checks if attribute exists, and if so return its length and pointer to data.
 * @param[in]  att_idx  Custom attribute index.
 * @param[out] length   Pointer to variable that receive length of the attribute.
 * @param[out] data     Pointer to variable that receive pointer characteristic value.
 * @return 0 on success, ATT_ERR_ATTRIBUTE_NOT_FOUND if there is no value for such attribyte.
 ****************************************************************************************
 */
static int sps_att_get_value(uint8_t att_idx, uint16_t *length, const uint8_t **data);

/**
 ****************************************************************************************
 * @brief  Send error indication to app
 * @param[in]   conidx      connection index
 * @param[in]   status      error status
 * @param[in]   ind_msg_id  message id
 * @param[in]   msg_id      error message id
 * @return void
 ****************************************************************************************
 */
void sps_server_error_ind_send(uint8_t conidx, uint8_t status, ke_msg_id_t ind_msg_id, ke_msg_id_t msg_id);

/**
 ****************************************************************************************
 * @brief  Forward data to peer device
 * @param[in]   msg         msg from upper layer
 * @param[in]   att_idx     attribute index
 * @param[in]   conidx      connection index
 * @param[in]   data_len    data length
 * @return true on success, false if notifications are not enabled or the connection index is invalid
 ****************************************************************************************
 */
bool sps_server_send_data( void *msg, uint16_t att_idx, uint8_t conidx, uint16_t data_len );

/**
 ****************************************************************************************
 * @brief  Forwards received data message to app
 * @param[in]   spss_env   spss environment pointer
 * @param[in]   msg        message pointer
 * @return  void
 ****************************************************************************************
 */
void sps_server_data_receive(struct spss_env_tag *spss_env, void *msg );

#endif /* BLE_SPS_SERVER */

#endif // SPS_SERVER_H_

/// @} SPS_SERVER
