/**
 ****************************************************************************************
 *
 * @file  user_spss_task.c
 *
 * @brief SPS application Message Handlers.
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
#include "rwip_config.h"        // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_SPS_SERVER)

#include "user_spss_task.h"
#include "user_spss.h" 
#include "app_task.h"           // Application Task API
#include "sps_server.h"         // SPS functions
#include "sps_server_task.h"

#include "user_sps_config.h"
#include "gpio.h"
#include "arch_console.h"
#include "user_sps_device_dma.h"
#include "prf_utils.h"
#include "app_entry_point.h"
#include "user_sps_buffer_dma.h"
#include "dma_uart_sps.h"

/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/

/*
 * EXTERNAL DEFINITIONS
 ****************************************************************************************
 */
 
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct ke_msg_handler user_spss_process_handlers[]=
{
    {SPS_SERVER_ENABLE_CFM,         (ke_msg_func_t)user_sps_server_enable_cfm_handler},
    {SPS_SERVER_DATA_TX_CFM,        (ke_msg_func_t)user_sps_server_data_tx_cfm_handler},
    {SPS_SERVER_TX_FLOW_CTRL_IND,   (ke_msg_func_t)user_sps_server_tx_flow_ctrl_ind_handler},
    {SPS_SERVER_ERROR_IND,          (ke_msg_func_t)user_sps_server_error_ind_handler},
    {SPS_SERVER_DATA_WRITE_IND,     (ke_msg_func_t)user_sps_server_data_write_ind_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t)gattc_mtu_changed_ind_handler},
    {GAPC_PARAM_UPDATED_IND,        (ke_msg_func_t)user_on_updated_params_complete}
};


enum process_event_response user_spss_process_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret)
{
    return (app_std_process_event(msgid, param,src_id,dest_id,msg_ret, user_spss_process_handlers,
                                         sizeof(user_spss_process_handlers)/sizeof(struct ke_msg_handler)));
}

int user_sps_server_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    user_dma_uart_to_ble();

    return (KE_MSG_CONSUMED);
}

int user_sps_server_data_tx_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_data_tx_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if(param->status == ATT_ERR_NO_ERROR){
        user_dma_uart_to_ble_confirm(true);
    } else {
        user_dma_uart_to_ble_confirm(false);
    }    

    return (KE_MSG_CONSUMED);
}

int user_sps_server_data_write_ind_handler(ke_msg_id_t const msgid,
                                           struct sps_val_write_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    user_ble_to_dma_uart( (void *)param ); 
    dma_uart_tx_async();
    return (KE_MSG_NO_FREE);
}

int user_sps_server_tx_flow_ctrl_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_server_tx_flow_ctrl_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

int user_sps_server_error_ind_handler(ke_msg_id_t const msgid,
                                      struct prf_server_error_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    ASSERT_WARNING(0);
    return (KE_MSG_CONSUMED);
}

int user_on_updated_params_complete(ke_msg_id_t const msgid, 
                                    const struct gapc_param_updated_ind *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{  
    return (KE_MSG_CONSUMED);
}


int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid, const struct gattc_mtu_changed_ind *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    dma_uart_set_tx_size(param->mtu);
    return (KE_MSG_CONSUMED);
};
#endif //(BLE_SPS_SERVER)

#endif //(BLE_APP_PRESENT)

/// @} DSPS_DEVICE_SERVICE

