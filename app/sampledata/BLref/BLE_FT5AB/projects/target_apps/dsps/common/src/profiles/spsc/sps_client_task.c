/**
 ****************************************************************************************
 *
 * @file sps_client_task.c
 *
 * @brief SPS client Task implementation.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SPS_CLIENT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_SPS_CLIENT)
#include "gap.h"
#include "gattc_task.h"
#include "prf.h"
#include "sps_client.h"
#include "sps_client_task.h"
#include "prf_utils.h"
#include "prf_utils_128.h"
#include "arch_console.h"
#include "user_buffer.h"
#include "lld_evt.h"

#ifdef DMA_UART
#include "user_sps_host_dma.h"
#else
#include "user_sps_host.h"
#endif

/// State machine used to retrieve SPS characteristics information
const struct prf_char_def_128 spsc_sps_char[SPSC_CHAR_MAX] =
{
    [SPSC_SRV_TX_DATA_CHAR]    = {SPS_SERVER_TX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_NTF},
    [SPSC_SRV_RX_DATA_CHAR]    = {SPS_SERVER_RX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_WR_NO_RESP},
    [SPSC_FLOW_CTRL_CHAR]      = {SPS_FLOW_CTRL_UUID, ATT_MANDATORY, ATT_CHAR_PROP_RD|ATT_CHAR_PROP_NTF|ATT_CHAR_PROP_WR_NO_RESP }
};
/// State machine used to retrieve SPS service characteristic description information
const struct prf_char_desc_def spsc_sps_char_desc[SPSC_DESC_MAX] =
{
    ///SPS Client Config
    [SPSC_SRV_TX_DATA_CLI_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, SPSC_SRV_TX_DATA_CHAR},
    [SPSC_FLOW_CTRL_CLI_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, SPSC_FLOW_CTRL_CHAR},
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * EXTERNAL DEFINITIONS
 ****************************************************************************************
 */     
extern struct sps_counter ble_data_counter;
extern RingBuffer ble_to_periph_buffer;
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the SPS client role, used after connection.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_enable_req_handler(ke_msg_id_t const msgid,
                                    struct sps_client_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
   
    // SPS Client Role Task Environment
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);

    if (param->con_type == PRF_CON_DISCOVERY)
    { 
        // Start discovering SPS on peer
        uint8_t sps_data_service_uuid[]=SPS_SERVICE_UUID;
        spsc_env->last_uuid_req = ATT_DECL_PRIMARY_SERVICE;
        memcpy(spsc_env->last_svc_req, sps_data_service_uuid, ATT_UUID_128_LEN);
        prf_disc_svc_send_128(&spsc_env->prf_env, spsc_env->last_svc_req);
        // Set state to discovering
        ke_state_set(dest_id, SPS_CLIENT_DISCOVERING );
            
    }
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_SVC_IND message.
 * Get the start and ending handle of sps in the enviroment.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */ 
static int gattc_disc_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_disc_svc_ind const *param,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    // Get the address of the environment
     struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);

    // Even if we get multiple responses we only store 1 range
    spsc_env->sps.svc.shdl = param->start_hdl;
    spsc_env->sps.svc.ehdl = param->end_hdl;
    spsc_env->nb_svc++;

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_IND message.
 * Characteristics for the currently desired service handle range are obtained and kept.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_disc_char_ind_handler(ke_msg_id_t const msgid,
                                          struct gattc_disc_char_ind const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    
    prf_search_chars_128(spsc_env->sps.svc.ehdl, SPSC_CHAR_MAX, &spsc_env->sps.chars[0], &spsc_sps_char[0], param, &spsc_env->last_char_code);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_DESC_IND message.
 * This event can be received 2-4 times, depending if measurement interval has seevral properties.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
                                           struct gattc_disc_char_desc_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Get the address of the environment
   struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);

   // Retrieve SPS descriptors
   prf_search_descs(SPSC_DESC_MAX, &spsc_env->sps.descs[0], &spsc_sps_char_desc[0], param, spsc_env->last_char_code);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY message meaning that
 * notification has been correctly sent to peer device (but not confirmed by peer device).
 * *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    uint8_t status;

    if(state == SPS_CLIENT_DISCOVERING)
    {
        if ((param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||(param->status == ATT_ERR_NO_ERROR))
        {
            //Currently discovering sps_server Service
            uint8_t sps_service_uuid[]=SPS_SERVICE_UUID;
            if (!memcmp(spsc_env->last_svc_req, sps_service_uuid, ATT_UUID_128_LEN))
            {
                if (spsc_env->last_uuid_req == ATT_DECL_PRIMARY_SERVICE)
                {
                    if (spsc_env->sps.svc.shdl == ATT_INVALID_HANDLE)
                    {
                        // stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, conidx, PRF_ERR_STOP_DISC_CHAR_MISSING);
                    }
                    // Too many services found only one such service should exist
                    else if(spsc_env->nb_svc > 1)
                    {
                        // stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, conidx, PRF_ERR_MULTIPLE_SVC);
                    }
                    else
                    {
                        // Discover SPS Device characteristics
                        prf_disc_char_all_send(&(spsc_env->prf_env),  &(spsc_env->sps.svc), conidx); 

                        // Keep last UUID requested and for which service in env
                        spsc_env->last_uuid_req = ATT_DECL_CHARACTERISTIC;
                    }
                }
                else if (spsc_env->last_uuid_req == ATT_DECL_CHARACTERISTIC)
                {
                    status = prf_check_svc_char_validity_128(SPSC_CHAR_MAX, spsc_env->sps.chars, spsc_sps_char);
                    
                    // Check for characteristic properties.
                    if (status == ATT_ERR_NO_ERROR)
                    {
                        spsc_env->last_uuid_req = ATT_INVALID_HANDLE;
                        spsc_env->last_char_code = spsc_sps_char_desc[SPSC_SRV_TX_DATA_CLI_CFG].char_code;

                        //Discover characteristic descriptor 
                        prf_disc_char_desc_send(&(spsc_env->prf_env), &(spsc_env->sps.chars[spsc_env->last_char_code]), conidx); 
                    }
                    else
                    {
                        // Stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, conidx, status);
                    }
                }
                else //Descriptors
                {
                    //Get code of next char. having descriptors
                    spsc_env->last_char_code = spsc_get_next_desc_char_code(spsc_env, &spsc_sps_char_desc[0]);
                    if (spsc_env->last_char_code != SPSC_CHAR_MAX)
                    {
                        prf_disc_char_desc_send(&(spsc_env->prf_env), &(spsc_env->sps.chars[spsc_env->last_char_code]), conidx); 
                    }
                    else
                    {
                         status = prf_check_svc_char_desc_validity(SPSC_DESC_MAX,
                                                                                                                                     spsc_env->sps.descs,
                                                                                                                                     spsc_sps_char_desc,
                                                                                                                                     spsc_env->sps.chars);

                        spsc_env->nb_svc = 0;
                        sps_client_enable_cfm_send(spsc_env,conidx, status);
                    }
                }
            }
        }
        else
        {
            sps_client_enable_cfm_send(spsc_env, conidx, param->status);
        }
    }
    else if (state == SPS_CLIENT_CONNECTED)
    {
        if(param->operation == GATTC_WRITE_NO_RESPONSE)
        {
            ASSERT_WARNING(spsc_env->pending_wr_no_rsp_cmp > 0);
            spsc_env->pending_wr_no_rsp_cmp--;
            if (!spsc_env->pending_wr_no_rsp_cmp && spsc_env->pending_tx_ntf_cmp)
            {
                spsc_env->pending_tx_ntf_cmp = false;
                sps_client_confirm_data_tx(spsc_env, param->status);
            }
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_EVENT_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                        struct gattc_event_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    
    if(spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl == param->handle)
    {
        //update ble flow control flag
        if(param->value[0] == FLOW_ON)
        {
            spsc_env->tx_flow_en = true;
            sps_client_indicate_tx_flow_ctrl(spsc_env, FLOW_ON);
        }
        else if (param->value[0] == FLOW_OFF)
        {
            spsc_env->tx_flow_en = false;
            sps_client_indicate_tx_flow_ctrl(spsc_env, FLOW_OFF);
        }
    }
    else if (spsc_env->sps.chars[SPSC_SRV_TX_DATA_CHAR].val_hdl == param->handle)
    {
        ble_data_counter.spsc_ble_rx_data_bytes += param->length;
        sps_client_data_receive( spsc_env, (void *)param);
        return (KE_MSG_NO_FREE);
    }

    return (KE_MSG_CONSUMED);
        
}

/**
 ****************************************************************************************
 * @brief Data transmitt request handler
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_data_tx_req_handler(ke_msg_id_t const msgid,
                                          struct sps_client_data_tx_req const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    uint8_t conidx = KE_IDX_GET(dest_id);
    
    if (ke_state_get(KE_BUILD_ID(spsc_env->prf_env.prf_task, conidx))==SPS_CLIENT_CONNECTED)
    {
        {
            sps_client_write_data_tx((void *)param, &spsc_env->prf_env, conidx, spsc_env->sps.chars[SPSC_SRV_RX_DATA_CHAR].val_hdl, (uint8_t *)param->data, sizeof(uint8_t) * param->length, GATTC_WRITE_NO_RESPONSE);
            spsc_env->pending_tx_ntf_cmp = true;
            spsc_env->pending_wr_no_rsp_cmp++;
            ble_data_counter.spsc_ble_tx_data_bytes +=  param->length ;
            return (KE_MSG_NO_FREE);
        } 

    }
    
    return (KE_MSG_CONSUMED);
}    
/**
 ****************************************************************************************
 * @brief Send flow control state @ref sps_client_send_notify_flow_control_state_req message.
 * @param[in] msgid Id of the message received 
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_rx_flow_ctrl_req_handler(ke_msg_id_t const msgid,
                                   struct sps_client_rx_flow_ctrl_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    uint16_t conhdl =  gapc_get_conhdl(KE_IDX_GET(dest_id));
   
    if( conhdl != GAP_INVALID_CONHDL ){
        // Send notification to the client
        prf_gatt_write(&spsc_env->prf_env, KE_IDX_GET(dest_id), spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl, (uint8_t*) &param->flow_control_state,sizeof (uint8_t),GATTC_WRITE_NO_RESPONSE);
        spsc_env->pending_wr_no_rsp_cmp++;
    }
    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

const struct ke_msg_handler spsc_default_state[] =
{
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {SPS_CLIENT_DATA_TX_REQ,        (ke_msg_func_t)sps_client_data_tx_req_handler},
    {SPS_CLIENT_RX_FLOW_CTRL_REQ,   (ke_msg_func_t)sps_client_rx_flow_ctrl_req_handler},
    {GATTC_DISC_CHAR_IND,           (ke_msg_func_t)gattc_disc_char_ind_handler},
    {GATTC_DISC_SVC_IND,            (ke_msg_func_t)gattc_disc_svc_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND,      (ke_msg_func_t)gattc_disc_char_desc_ind_handler},
    {SPS_CLIENT_ENABLE_REQ,         (ke_msg_func_t)sps_client_enable_req_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
};

// Specifies the message handlers that are common to all states.
const struct ke_state_handler sps_client_default_handler = KE_STATE_HANDLER(spsc_default_state);

#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENT

