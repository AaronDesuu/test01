/**
 ****************************************************************************************
 *
 * @file  user_remote_config_task.c
 *
 * @brief Remote Configuration application Task Implementation.
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

#if (BLE_APP_PRESENT)

#if (BLE_REMOTE_CONFIG)

#include "user_remote_config.h"
#include "user_remote_config_task.h"
#include "app_task.h"                  // Application Task API

#include "app.h" 
#include "app_entry_point.h"
#include "ke_msg.h"
#include "prf_utils.h"

extern user_configuration_struct_tag user_remote_config_env;
extern user_rem_config_function_t user_rem_config_cb;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles write configuration characteristic event indication from remote_config profile
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int user_remote_discover_val_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_discover_request_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t status = 0;
    uint8_t max_resp_elms = 0;
    uint16_t size;
    uint8_t i, j;
    uint16_t* p;
    uint8_t send_elms;
    uint16_t* elems_table;
    struct remote_config_env_tag* remote_config_env =PRF_ENV_GET (REMOTE_CONFIG,remote_config);
    uint16_t max_mtu = gattc_get_mtu(remote_config_env->conidx);

    elems_table = (uint16_t*) ke_malloc(user_remote_config_env.params_num*2 + 2, KE_MEM_NON_RETENTION);
    
    max_resp_elms = (param->max_elms > user_remote_config_env.params_num)? user_remote_config_env.params_num : param->max_elms;
    
    if (max_resp_elms * 2 > max_mtu - 5)
    {
        max_resp_elms = (max_mtu - 5)/2;
    }
    for (i = 0; i < user_remote_config_env.params_num; i+= max_resp_elms)
    {
        if ((i + max_resp_elms) > user_remote_config_env.params_num)
            send_elms = user_remote_config_env.params_num - i;
        else send_elms =  max_resp_elms;

        size = send_elms * 2;
        // Allocate message
        struct remote_discover_response_cmd *cmd = KE_MSG_ALLOC_DYN(REMOTE_DISCOVER_RESPONSE_CMD,
                                                  prf_get_task_from_id(TASK_ID_REMOTE_CONFIG),
                                                  TASK_APP,
                                                  remote_discover_response_cmd, size);
        p = &elems_table[0];
        for (j = i; j < i + send_elms ; j++)
        {
            *p++ = user_remote_config_env.params[j].elem_id;
        }
        if (j >= user_remote_config_env.params_num)
            cmd->more_elms = 0;
        else
            cmd->more_elms = 1;

        cmd->num_of_elms = send_elms;
        cmd->size = size;
        memcpy((char*)cmd->val, (char*)elems_table, cmd->size);
        cmd->conhdl = param->conhdl;
        cmd->status = status;

        ke_msg_send(cmd);
    }
    ke_free(elems_table);
    return (KE_MSG_CONSUMED);
}

static int user_remote_write_request_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_write_request_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t status = NO_ERROR;
    uint8_t i, j;
    uint8_t* p = (uint8_t *)&param->data[0];
    uint16_t    elemid;

    // Allocate message
   struct remote_write_response_cmd *cmd = KE_MSG_ALLOC(REMOTE_WRITE_RESPONSE_CMD,
                                              prf_get_task_from_id(TASK_ID_REMOTE_CONFIG),
                                              TASK_APP,
                                              remote_write_response_cmd);

   cmd->conhdl = param->conhdl;
   cmd->elm_id = 0x0000;
   if ((param->num_of_elms == 0) || (param->num_of_elms > user_remote_config_env.params_num))
   {
        cmd->status = INVALID_ELMS_NUM;
        ke_msg_send(cmd);
        return (KE_MSG_CONSUMED);
   }

   for (i = 0; i < param->num_of_elms && !status; i++)
   {
       elemid = p[0] | ((uint16_t)p[1] << 8);
       for (j = 0; j < user_remote_config_env.params_num; j++)
       {
           if (elemid == user_remote_config_env.params[j].elem_id)
           {
               user_remote_config_env.params[j].changed = false;
               if (p[2] > user_remote_config_env.params[j].max_size)
               {
                   cmd->elm_id = elemid;
                   status = INVALID_ELM_LEN;
                   break;
               }
               if (user_remote_config_env.params[j].validation_cb)
               {
                   if (user_remote_config_env.params[j].validation_cb(&p[3]))
                   {
                       cmd->elm_id = elemid;
                       status = INVALID_ELM_DATA;
                       break;
                   }
               }
               memcpy((uint8_t*)user_remote_config_env.params[j].data, &p[3], p[2]);
               user_remote_config_env.params[j].cur_size = p[2];
               user_remote_config_env.params[j].changed = true;
               p += (3 + user_remote_config_env.params[j].cur_size);
               break;
           }
       }
       if (j == user_remote_config_env.params_num)
       {
           cmd->elm_id = elemid;
           status = UNKNOWN_ELM_ID;
           break;
       }
   }
   if (status || !param->more_elms)
   {
       cmd->status = status;
       ke_msg_send(cmd);
       if (!status && user_rem_config_cb)
       {
           user_rem_config_cb(&user_remote_config_env);
       }
   }
   return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles read command characteristic event indication from remote_config profile
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int user_remote_read_request_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_read_request_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t status = NO_ERROR;
    struct remote_config_env_tag* remote_config_env =PRF_ENV_GET (REMOTE_CONFIG,remote_config);
    uint16_t max_mtu = gattc_get_mtu(remote_config_env->conidx);
    
    // Allocate message
   struct remote_read_response_cmd *cmd = KE_MSG_ALLOC(REMOTE_READ_RESPONSE_CMD,
                                              prf_get_task_from_id(TASK_ID_REMOTE_CONFIG),
                                              TASK_APP,
                                              remote_read_response_cmd);

    if (!status)
    {
        int i;
        for (i = 0; i < user_remote_config_env.params_num; i++)
        {
            if (user_remote_config_env.params[i].elem_id == param->param_id)
                break;
        }
        if (i < user_remote_config_env.params_num)
        {
            if (user_remote_config_env.params[i].cur_size > REM_READ_ELM_MAX_SIZE - 4)
            {
                cmd->size = 0;
                status = INVALID_ELM_LEN;
            }
            else
            {
                cmd->size = user_remote_config_env.params[i].cur_size;
                memcpy(cmd->val, user_remote_config_env.params[i].data, cmd->size);
                if (user_remote_config_env.params[i].cur_size > max_mtu - 7)
                {
                    status = LARGE_ELEMENT;
                }
            }
        }
        else
        {
            cmd->size = 0;
            status = UNKNOWN_ELM_ID;
        }
    }
    else
        cmd->size = 0;
    
    cmd->param_id = param->param_id;
    cmd->conhdl = param->conhdl;
    cmd->status = status;
    
    ke_msg_send(cmd);
    
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles write of characteristic's configuration event indication from remote config profile
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
#if BLE_APP_SEC
static int user_remote_discover_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_discover_cfg_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //do nothing

    return (KE_MSG_CONSUMED);
}

static int user_remote_write_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_write_cfg_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //do nothing

    return (KE_MSG_CONSUMED);
}

static int user_remote_read_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct remote_write_cfg_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{    
    //do nothing
    
    return (KE_MSG_CONSUMED);
}

#endif

/**
 * @brief Remote Config task user handlers
 */
const struct ke_msg_handler user_remote_config_process_handlers[]=
{
    {REMOTE_DISCOVER_VAL_IND,           (ke_msg_func_t) user_remote_discover_val_ind_handler},
    {REMOTE_WRITE_REQUEST_IND,          (ke_msg_func_t) user_remote_write_request_ind_handler},
    {REMOTE_READ_REQUEST_IND,           (ke_msg_func_t) user_remote_read_request_ind_handler},
#if BLE_APP_SEC
    {REMOTE_DISCOVER_CFG_IND,           (ke_msg_func_t) user_remote_discover_cfg_ind_handler},
    {REMOTE_WRITE_CFG_IND,              (ke_msg_func_t) user_remote_write_cfg_ind_handler},
    {REMOTE_READ_CFG_IND,               (ke_msg_func_t) user_remote_read_cfg_ind_handler},
#endif
};

enum process_event_response user_remote_config_process_handler(ke_msg_id_t const msgid,
                            void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id,
                            enum ke_msg_status_tag *msg_ret)
{
   return (app_std_process_event(msgid, param, src_id, dest_id, msg_ret, user_remote_config_process_handlers,
                                         sizeof(user_remote_config_process_handlers) / sizeof(struct ke_msg_handler)));
};


#endif // (BLE_REMOTE_CONFIG)

#endif //(BLE_APP_PRESENT)

/// @}
