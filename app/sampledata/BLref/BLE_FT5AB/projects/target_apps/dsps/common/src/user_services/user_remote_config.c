/**
 ****************************************************************************************
 *
 * @file  user_remote_config.c
 *
 * @brief Remote Configuration application.
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
#include "app_prf_perm_types.h"

user_configuration_struct_tag user_remote_config_env __attribute__((section("retention_mem_area0"),zero_init));

uint16_t user_remote_config_conhdl __attribute__((section("retention_mem_area0"),zero_init));

user_rem_config_function_t user_rem_config_cb __attribute__((section("retention_mem_area0"),zero_init));

void user_remote_config_init(user_config_elem_t *params_struct, uint8_t params_num, char * version, uint8_t version_length, user_rem_config_function_t callback)
{
    user_remote_config_env.params_num = params_num;
    user_remote_config_env.params = params_struct;
    memcpy (user_remote_config_env.version, version, version_length);
    user_remote_config_env.version_length =  version_length;
        
    user_rem_config_cb = callback;
}
 
void user_remote_config_enable(uint8_t conhdl)
{        
    // Allocate the message
    struct remote_config_enable_req* req = KE_MSG_ALLOC(REMOTE_CONFIG_ENABLE_REQ, 
                                                        prf_get_task_from_id(TASK_ID_REMOTE_CONFIG),
                                                        TASK_APP,
                                                        remote_config_enable_req);

    user_remote_config_conhdl =conhdl;
    req->conidx = conhdl;
    req->ntf_cfg = SRV_PERM_DISABLE;
    
    memcpy(req->version, user_remote_config_env.version, 16);
    req->version_length =  user_remote_config_env.version_length;
    
    // Send the message
    ke_msg_send(req);
}

void user_remote_config_create_db(void)
{
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                             TASK_GAPM,
                                                             TASK_APP,
                                                             gapm_profile_task_add_cmd,
                                                             sizeof(struct remote_config_db_cfg));

    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = get_user_prf_srv_perm(TASK_ID_REMOTE_CONFIG);
    req->prf_task_id = TASK_ID_REMOTE_CONFIG;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    struct remote_config_db_cfg *db_cfg = (struct remote_config_db_cfg *)req->param;
        
    // Configuration fields, for future use
    db_cfg->att_tbl = NULL;
    db_cfg->cfg_flag = 0;
    db_cfg->features = 0;

    ke_msg_send(req);
}

#endif //BLE_REMOTE_CONFIG
#endif //BLE_APP_PRESENT

/// @}
