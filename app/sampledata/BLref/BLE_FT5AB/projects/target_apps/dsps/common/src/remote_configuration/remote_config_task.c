/**
 ****************************************************************************************
 *
 * @file remote_config_task.c
 *
 * @brief Remote COnfiguration Service Task implementation.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
 
 /**
 ****************************************************************************************
 * @addtogroup REMOTE_CONFIG_GATT_SERVICE
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
*/

#if (BLE_REMOTE_CONFIG)

#include "remote_config_task.h"
#include "prf_utils.h"
#include "user_periph_setup.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send Remote configuration discover request to application
 * @param[in] dest_id   Destination kernel identifier
 * @param[in] src_id    Source kernel identifier
 * @param[in] param     Parameters of the requested attribute
 ****************************************************************************************
 */
static void remote_config_discover_val_request(ke_task_id_t const dest_id, ke_task_id_t const src_id, struct gattc_write_req_ind const *param)
{

    struct remote_config_env_tag* remote_config_env =PRF_ENV_GET (REMOTE_CONFIG,remote_config);
    // Allocate the alert value change indication
    struct remote_config_val_ind *ind = KE_MSG_ALLOC(REMOTE_DISCOVER_VAL_IND,
                                        src_id,
                                        dest_id,
                                        remote_config_val_ind);

    // Fill in the parameter structure
    ind->conhdl = gapc_get_conhdl(remote_config_env->conidx);
    ind->size = 1;
    ind->val[0] = param->value[0];

    // Send the message
    ke_msg_send(ind);
}

 /**
 ****************************************************************************************
 * @brief Send Remote configuration read request to application
 * @param[in] dest_id       Destination kernel identifier
 * @param[in] src_id        Source kernel identifier
 * @param[in] param_value   Parameter Id of the requested attribute
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_read_val_request(ke_task_id_t const dest_id, ke_task_id_t const src_id, const uint8_t *param_value)
{
   struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG,remote_config);
    // Allocate message
   struct remote_read_request_ind *ind = KE_MSG_ALLOC(REMOTE_READ_REQUEST_IND,
                                                src_id,
                                                dest_id,
                                                remote_read_request_ind);
    
    // Fill in the parameter structure
    ind->conhdl   = gapc_get_conhdl(remote_config_env->conidx);
    ind->param_id = co_read16p(&param_value[0]);

    // Send the message
    ke_msg_send(ind);
   
    return (KE_MSG_CONSUMED);
}

 /**
 ****************************************************************************************
 * @brief Send Remote configuration write request to application
 * @param[in] dest_id       Destination kernel identifier
 * @param[in] src_id        Source kernel identifier
 * @param[in] param_value   Parameters of the requested attribute
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_write_val_request(ke_task_id_t const dest_id, ke_task_id_t const src_id, struct gattc_write_req_ind const *param)
{
   struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG,remote_config);
    // Allocate message
   struct remote_write_request_ind *ind = KE_MSG_ALLOC(REMOTE_WRITE_REQUEST_IND,
                                                src_id,
                                                dest_id,
                                                remote_write_request_ind);

    // Fill in the parameter structure
   ind->conhdl   = gapc_get_conhdl(remote_config_env->conidx);

   ind->num_of_elms = param->value[0];
   ind->more_elms = param->value[1];
   memcpy(ind->data, &param->value[2], param->length);
    // Send the message
   ke_msg_send(ind);

   return (KE_MSG_CONSUMED);
}

#if BLE_APP_SEC
/**
 ********************************************************************************************************
 * @brief Send indication to application task that DISCOVER's client configuration has been written
 * @param[in] dest_id   Destination kernel identifier
 * @param[in] src_id    Source kernel identifier
 * @param[in] val       The updated value to be sent
 * @return void
 ********************************************************************************************************
 */
static void remote_discover_send_cfg(ke_task_id_t const dest_id, ke_task_id_t const src_id, uint16_t val)
{

    struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);

    // Allocate the alert value change indication
    struct remote_discover_cfg_ind *ind = KE_MSG_ALLOC(REMOTE_DISCOVER_CFG_IND,
                                        src_id,
                                        dest_id,
                                        remote_discover_cfg_ind);

    // Fill in the parameter structure
    ind->conhdl = gapc_get_conhdl(remote_config_env->conidx);
    ind->val = val;

    // Send the message
    ke_msg_send(ind);
}

/**
 ********************************************************************************************************
 * @brief Send indication to application task that WRITE's client configuration has been written
 * @param[in] dest_id   Destination kernel identifier
 * @param[in] src_id    Source kernel identifier
 * @param[in] val       The updated value to be sent
 * @return void
 ********************************************************************************************************
 */
static void remote_write_send_cfg(ke_task_id_t const dest_id, ke_task_id_t const src_id, uint16_t val)
{

    struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);

    // Allocate the alert value change indication
    struct remote_write_cfg_ind *ind = KE_MSG_ALLOC(REMOTE_WRITE_CFG_IND,
                                        src_id,
                                        dest_id,
                                        remote_write_cfg_ind);

    // Fill in the parameter structure
    ind->conhdl = gapc_get_conhdl(remote_config_env->conidx);
    ind->val = val;

    // Send the message
    ke_msg_send(ind);
}

/**
 ********************************************************************************************************
 * @brief Send indication to application task that READ's client configuration has been written
 * @param[in] dest_id   Destination kernel identifier
 * @param[in] src_id    Source kernel identifier
 * @param[in] val       The updated value to be sent
 * @return void
 ********************************************************************************************************
 */
static void remote_read_send_cfg(ke_task_id_t const dest_id, ke_task_id_t const src_id, uint16_t val)
{
    
    struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);  
    
    // Allocate the alert value change indication
    struct remote_read_cfg_ind *ind = KE_MSG_ALLOC(REMOTE_READ_CFG_IND,
                                        src_id,
                                        dest_id,
                                        remote_read_cfg_ind);
      
    // Fill in the parameter structure
    ind->conhdl = gapc_get_conhdl(remote_config_env->conidx);
    ind->val = val;
    
    // Send the message
    ke_msg_send(ind);
}
#endif

/**
 ********************************************************************************************************
 * @brief Handles the new characteristics client configuration value
 * @param[in] ntf_cfg                      New value
 * @param[in] remote_config_env_ntf_cfg    Address of the old value
 * @return True if the update was successful, else False
 ********************************************************************************************************
 */
static uint8_t handle_ntf_change(uint16_t ntf_cfg, uint8_t* remote_config_env_ntf_cfg)
{
    if ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg ==PRF_CLI_START_NTF))
    {
        // Conserve information in environment
        if (ntf_cfg == PRF_CLI_START_NTF)
        {
            // Ntf cfg bit set to 1
            *remote_config_env_ntf_cfg |= (REMOTE_CONFIG_NTF_EN );
        }
        else
        {
            // Ntf cfg bit set to 0
            *remote_config_env_ntf_cfg &= ~(REMOTE_CONFIG_NTF_EN );
        }
        return 1;
    }
    return 0;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_ATT_INFO_REQ_IND.
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the message. Handles reception of @ref gattc_read_req message
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_att_info_req_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gattc_att_info_cfm * cfm;

    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;
    cfm->length = 0;
    cfm->status = 0;
    ke_msg_send(cfm);

   return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATT_WRITE_CMD_IND message.
 * @param[in] msgid     Id of the message received (probably unused).
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_req_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;

    // retrieve handle information
    uint8_t status = remote_config_get_att_idx(param->handle, &att_idx);  

    if (status == GAP_ERR_NO_ERROR)
    {
        struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);
          // Extract value before check
        uint16_t ntf_cfg = co_read16p(&param->value[0]);

        switch (param->handle - remote_config_env->remote_config_shdl)
        {
            case REMOTE_DISCOVER_ELM_IDX_VAL:
            {
                if (param->length > REM_DISCOVER_ELM_WRITE_MAX_LEN || param->value[0] > REM_DISCOVER_ELM_WRITE_MAX_VAL)
                {
                    status  = PRF_APP_ERROR;
                    break;
                }
                remote_config_discover_val_request(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), param);
                break;
            }
            case REMOTE_WRITE_ELM_IDX_VAL:
            {
                if (param->length > REM_WRITE_ELM_MAX_SIZE || param->length < REM_WRITE_ELM_MIN_LEN)
                {
                    status = PRF_ERR_UNEXPECTED_LEN; 
                    break;
                }
                remote_config_write_val_request(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), param);
                break;
            }
            case REMOTE_READ_ELM_IDX_VAL:
            {
                if (param->length != REM_READ_ELM_MIN_LEN)
                {
                    status = PRF_ERR_UNEXPECTED_LEN;
                    break;
                }
                remote_config_read_val_request(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), param->value);
                break;
            }
            case REMOTE_DISCOVER_ELM_IDX_CFG:
            {
#if BLE_APP_SEC
                if (handle_ntf_change(ntf_cfg, &(remote_config_env->discover_ntf_cfg)))
                    //Inform APP on configuration change
                    remote_discover_send_cfg(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), remote_config_env->discover_ntf_cfg);
#else
                handle_ntf_change(ntf_cfg, &(remote_config_env->discover_ntf_cfg));
#endif
                break;
            }
            case REMOTE_WRITE_ELM_IDX_CFG:
            {
#if BLE_APP_SEC
                if (handle_ntf_change(ntf_cfg, &(remote_config_env->write_ntf_cfg)))
                    remote_write_send_cfg(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), remote_config_env->write_ntf_cfg);
#else
                handle_ntf_change(ntf_cfg, &(remote_config_env->write_ntf_cfg));
#endif
                break;
            }
            case REMOTE_READ_ELM_IDX_CFG:
            {
#if BLE_APP_SEC
                if (handle_ntf_change(ntf_cfg, &(remote_config_env->read_ntf_cfg)))
                    remote_read_send_cfg(dest_id, prf_dst_task_get(&(remote_config_env->prf_env), KE_IDX_GET(src_id)), remote_config_env->read_ntf_cfg);
#else
                handle_ntf_change(ntf_cfg, &(remote_config_env->read_ntf_cfg));
#endif
                break;
            }
            default:
            {
                status = PRF_APP_ERROR;
                break;
            }
        }
    }
    else
    {
        status  = PRF_APP_ERROR;
    }
    
    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    ke_msg_send(cfm);
    
   return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief GATTC read request indication handler
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the message. Handles reception of @ref gattc_read_req message
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = remote_config_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    uint16_t ntf_cfg = 0 ;
   
     struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
       if (att_idx == REMOTE_DISCOVER_ELM_IDX_CFG)
       {
            ntf_cfg = (remote_config_env->discover_ntf_cfg & REMOTE_CONFIG_NTF_EN ) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
            length = sizeof(uint16_t);
       }
       if (att_idx == REMOTE_WRITE_ELM_IDX_CFG)
       {
            ntf_cfg = (remote_config_env->write_ntf_cfg & REMOTE_CONFIG_NTF_EN ) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
            length = sizeof(uint16_t);
       }
       if (att_idx == REMOTE_READ_ELM_IDX_CFG)
       {
            ntf_cfg = (remote_config_env->read_ntf_cfg & REMOTE_CONFIG_NTF_EN ) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
            length = sizeof(uint16_t);
       }

    }

    // Send read response
    cfm = KE_MSG_ALLOC(GATTC_READ_CFM,
                           KE_BUILD_ID(TASK_GATTC,conidx),
                           dest_id,
                           gattc_read_cfm);
        
    cfm->handle = param->handle;
    cfm->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        if ((att_idx == REMOTE_DISCOVER_ELM_IDX_CFG) || (att_idx == REMOTE_WRITE_ELM_IDX_CFG) || (att_idx == REMOTE_READ_ELM_IDX_CFG))
            co_write16p(cfm->value, ntf_cfg);
    }
    cfm->length = length;

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Remote_config enable handler. Handles reception of @ref remote_config_enable_req message
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_config_enable_req_handler(ke_msg_id_t const msgid,
                                    struct remote_config_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{ 
    struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);
       
     // Keep source of message, to respond to it further on
    remote_config_env->prf_env.app_task = src_id;
    // Store the connection handle for which this profile is enabled
    remote_config_env->prf_env.prf_task = gapc_get_conidx(param->conidx);
    remote_config_env->conidx = param->conidx;
    // Check if the provided connection exist
    if (remote_config_env->conidx == GAP_INVALID_CONIDX)
    {
        // The connection doesn't exist, request disallowed
         struct prf_server_error_ind *ind = KE_MSG_ALLOC(REMOTE_CONFIG_ERROR_IND,
                                                    remote_config_env->prf_env.app_task , remote_config_env->prf_env.prf_task,
                                                    prf_server_error_ind);

        ind->status    = PRF_ERR_REQ_DISALLOWED;
        ind->msg_id    = REMOTE_CONFIG_ENABLE_REQ;

        // send the message
        ke_msg_send(ind);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the DISCOVER response from the application and send it to GATTC
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the response.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_discover_rsp_cmd_handler(ke_msg_id_t const msgid,
                                    struct remote_discover_response_cmd const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t size;
    struct remote_config_env_tag *remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);

    // Check provided values
    if(param->conhdl == gapc_get_conhdl(remote_config_env->conidx))
    {
        uint8_t temp_buffer[128 + 2];

        temp_buffer[0] = param->num_of_elms;
        temp_buffer[1] = param->more_elms;

        if ((param->status == 0) && (param->size < 128))
        {
            size = param->size + 2;
            memcpy (&temp_buffer[2], param->val, param->size);
        }
        else
            size = 2;

        if(remote_config_env->discover_ntf_cfg & PRF_CLI_START_NTF)
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                              KE_BUILD_ID(TASK_GATTC,remote_config_env->conidx),
                                                              dest_id,
                                                              gattc_send_evt_cmd,
                                                              size);

            // Fill in the parameter structure
            req->operation = GATTC_NOTIFY;
            req->handle = remote_config_env->remote_config_shdl+REMOTE_DISCOVER_ELM_IDX_VAL;
            req->length = size;
            memcpy(req->value,(uint8_t*)temp_buffer, size);
            // Send the event
            ke_msg_send(req);
        }

    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the WRITE response from the application and send it to GATTC
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the response.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_write_rsp_cmd_handler(ke_msg_id_t const msgid,
                                    struct remote_write_response_cmd const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t size;
    struct remote_config_env_tag *remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);

    // Check provided values
    if(param->conhdl == gapc_get_conhdl(remote_config_env->conidx))
    {
        uint8_t temp_buffer[3];

        temp_buffer[0] = param->status;
        temp_buffer[1] = (uint8_t)param->elm_id;
        temp_buffer[2] = (uint8_t)(param->elm_id >> 8);
        
        size = 3;

        if(remote_config_env->write_ntf_cfg & PRF_CLI_START_NTF)
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                            KE_BUILD_ID(TASK_GATTC,remote_config_env->conidx),
                                                            dest_id,
                                                            gattc_send_evt_cmd,
                                                            size);

            // Fill in the parameter structure
            req->operation = GATTC_NOTIFY;
            req->handle = remote_config_env->remote_config_shdl+REMOTE_WRITE_ELM_IDX_VAL;
            req->length = size;
            memcpy(req->value,(uint8_t*)temp_buffer, size);

            // Send the event
            ke_msg_send(req);
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the READ response from the application and send it to GATTC
 * @param[in] msgid     Id of the message received
 * @param[in] param     Pointer to the parameters of the response.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int remote_read_rsp_cmd_handler(ke_msg_id_t const msgid,
                                    struct remote_read_response_cmd const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t payload_size;
    struct remote_config_env_tag *remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);
    uint8_t frag_payload_size = 0;
    uint8_t last_frag_payload_size = 0;
    uint8_t fragments = 1;
    uint16_t max_mtu = gattc_get_mtu(remote_config_env->conidx);

    // Check provided values
    if(param->conhdl == gapc_get_conhdl(remote_config_env->conidx))
    {
        uint8_t temp_buffer[REM_READ_ELM_MAX_SIZE];

        temp_buffer[0] = (uint8_t)param->param_id;
        temp_buffer[1] = (uint8_t)(param->param_id >> 8);
        temp_buffer[2] = param->size;
        temp_buffer[3] = param->status;

        if ((param->status == NO_ERROR) && (param->size < REM_READ_ELM_MAX_SIZE - 4))
        {
            frag_payload_size = payload_size = param->size;
            memcpy (&temp_buffer[4], param->val, param->size);
        }
        else
        {
            last_frag_payload_size = payload_size = 0;
            temp_buffer[2] = 0;
            if (param->status == LARGE_ELEMENT)
            {
                temp_buffer[2] = param->size;
                payload_size = frag_payload_size = max_mtu - 3 - 4;
                fragments = param->size / frag_payload_size;
                last_frag_payload_size = param->size % frag_payload_size;
                if (last_frag_payload_size) fragments++;
                memcpy (&temp_buffer[4], param->val, param->size);
            }
        }

        if(remote_config_env->read_ntf_cfg & PRF_CLI_START_NTF)
        {      
            uint8_t *p = &temp_buffer[4];
            do {
                if (fragments == 1 && last_frag_payload_size)
                {
                    payload_size = last_frag_payload_size;
                    temp_buffer[3] = NO_ERROR;
                }

                // Allocate the GATT notification message
                struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                        KE_BUILD_ID(TASK_GATTC, remote_config_env->conidx), dest_id,
                        gattc_send_evt_cmd, payload_size + 4);

                // Fill in the parameter structure
                req->operation = GATTC_NOTIFY;
                req->handle = remote_config_env->remote_config_shdl+REMOTE_READ_ELM_IDX_VAL;
                req->length = payload_size + 4;
                memcpy(req->value,(uint8_t*)temp_buffer, 4);
                memcpy(req->value+4,(uint8_t*)p, payload_size);
                p += (payload_size);
                fragments--;

                // Send the event
                ke_msg_send(req);
            } while(fragments);
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/**
 * @brief Remote Config task handlers
 */
const struct ke_msg_handler remote_config_default_state[] =
{
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) remote_config_gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) remote_config_gattc_read_req_ind_handler},
    {REMOTE_CONFIG_ENABLE_REQ,      (ke_msg_func_t) remote_config_enable_req_handler},
    {REMOTE_DISCOVER_RESPONSE_CMD,  (ke_msg_func_t) remote_discover_rsp_cmd_handler},
    {REMOTE_WRITE_RESPONSE_CMD,     (ke_msg_func_t) remote_write_rsp_cmd_handler},
    {REMOTE_READ_RESPONSE_CMD,      (ke_msg_func_t) remote_read_rsp_cmd_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) remote_config_gattc_att_info_req_ind_handler},
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler remote_config_default_handler = KE_STATE_HANDLER(remote_config_default_state);

#endif //BLE_REMOTE_CONFIG

/// @} 

