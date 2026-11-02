/**
 ****************************************************************************************
 *
 * @file  user_spss.c
 *
 * @brief SPS application.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_DEVICE_SERVICE
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "rwip_config.h"            // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_SPS_SERVER)

#include "app_task.h"                // application task definitions
#include "sps_server_task.h"        // SPS functions
#include "sps_server.h"        // SPS functions
#include "user_spss.h"
#include "app_prf_perm_types.h"
#include "arch_console.h"
#include "user_sps_device_dma.h"
#include "gpio.h"
#include "user_periph_setup.h"
#include "user_sps_utils.h"
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void user_spss_init(void)
{
}

void user_spss_create_db(void)
{
    
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                             TASK_GAPM,
                                                             TASK_APP,
                                                             gapm_profile_task_add_cmd,
                                                             sizeof(struct spss_db_cfg));
   
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = get_user_prf_srv_perm(TASK_ID_SPS_SERVER);
    req->prf_task_id = TASK_ID_SPS_SERVER;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    struct spss_db_cfg *db_cfg = (struct spss_db_cfg *)req->param;
        
    // Configuration fields, for future use
    db_cfg->att_tbl = NULL;
    db_cfg->cfg_flag = 0;
    db_cfg->features = 0;

    // Send the message
    ke_msg_send(req);
        
}

void user_spss_enable(uint8_t conidx)
{
    // Allocate the message
    struct sps_server_enable_req *req = KE_MSG_ALLOC(SPS_SERVER_ENABLE_REQ,
                                                     prf_get_task_from_id(TASK_ID_SPS_SERVER),
                                                     TASK_APP,
                                                     sps_server_enable_req);

    // Fill in the parameter structure
    req->conidx             = conidx;
    req->sec_lvl            = SRV_PERM_DISABLE;
    
    ke_msg_send(req);
        
}

void user_send_ble_data(const uint8_t *data, uint16_t length)
{
      
    struct sps_server_data_tx_req * req = KE_MSG_ALLOC_DYN_NO_INIT(SPS_SERVER_DATA_TX_REQ,
                                                                   prf_get_task_from_id(TASK_ID_SPS_SERVER),
                                                                   TASK_APP,
                                                                   sps_server_data_tx_req,
                                                                   length);

    req->length = length;
        
    memcpy(&req->data[0], data, length);
    
    ke_msg_send(req);
         
}

void user_send_dummy_ble_data(uint16_t length)
{
      
    struct sps_server_data_tx_req * req = KE_MSG_ALLOC_DYN_NO_INIT(SPS_SERVER_DATA_TX_REQ,
                                                                   prf_get_task_from_id(TASK_ID_SPS_SERVER),
                                                                   TASK_APP,
                                                                   sps_server_data_tx_req,
                                                                   length);

    req->length = length;
    ke_msg_send(req);
         
}

void user_send_ble_flow_ctrl(uint8_t flow_ctrl)
{
    struct sps_server_rx_flow_ctrl_req * req = KE_MSG_ALLOC(SPS_SERVER_RX_FLOW_CTRL_REQ,
                                                            prf_get_task_from_id(TASK_ID_SPS_SERVER),
                                                            TASK_APP,
                                                            sps_server_rx_flow_ctrl_req);
    
    req->flow_control_state = (uint8_t) flow_ctrl;
    ke_msg_send(req);
}

#endif // (BLE_SPS_SERVER)
#endif // (BLE_APP_PRESENT)

/// @} DSPS_DEVICE_SERVICE
