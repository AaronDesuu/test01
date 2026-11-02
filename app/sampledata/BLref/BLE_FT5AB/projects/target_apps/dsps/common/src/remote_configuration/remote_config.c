/**
 ****************************************************************************************
 *
 * @file remote_config.c
 *
 * @brief Remote Configuration Service Implementation.
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
#include "attm_db_128.h"

extern user_configuration_struct_tag user_remote_config_env;

/*
 *  REMOTE_CONFIG PROFILE ATTRIBUTES VALUES DEFINITION
 ****************************************************************************************
 */
static const uint16_t att_decl_svc       = ATT_DECL_PRIMARY_SERVICE;
static const uint16_t att_decl_char      = ATT_DECL_CHARACTERISTIC;
static const uint16_t att_decl_cfg       = ATT_DESC_CLIENT_CHAR_CFG;
static const uint16_t att_decl_user_desc = ATT_DESC_CHAR_USER_DESCRIPTION;

   
struct attm_desc_128 remote_config_att_db [REMOTE_CONFIG_IDX_NB] =
{
/// Remote Config Service Declaration
    [REMOTE_CONFIG_IDX_SVC]              = {(uint8_t*)&att_decl_svc, ATT_UUID_128_LEN, PERM(RD, ENABLE),
                                                sizeof(remote_config_service), sizeof(remote_config_service), (uint8_t*)&remote_config_service},
/// Version Characteristic          ==========================================================================================================
    [REMOTE_VERSION_IDX_CHAR]            = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),0,0,NULL},
  
     // Characteristic Value
    [REMOTE_VERSION_IDX_VAL]             = {(uint8_t*)rem_version_val.uuid, ATT_UUID_128_LEN,
                                                PERM(RD, ENABLE),REM_VERSION_MAX_SIZE, 0,NULL},

    // Characteristic Configuration Descriptor
    [REMOTE_VERSION_IDX_CFG]        = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE)  | PERM(WRITE_REQ, ENABLE),
                                                sizeof(uint16_t), 0, NULL},

    // Characteristic User Description
    [REMOTE_VERSION_IDX_DESC]            = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                                sizeof(REM_VERSION_DESC) - 1, sizeof(REM_VERSION_DESC) - 1,(uint8_t*) REM_VERSION_DESC},                                                                                
/// Discover elm Characteristic     ==========================================================================================================
    //Characteristic Declaration
    [REMOTE_DISCOVER_ELM_IDX_CHAR]       = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(WR, ENABLE)  | PERM(WRITE_REQ, ENABLE),
                                                0,0,NULL}, 
		
    // Characteristic Value
    [REMOTE_DISCOVER_ELM_IDX_VAL]        = {(uint8_t*)rem_discover_elm_val.uuid, ATT_UUID_128_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE),
                                                PERM(RI, ENABLE) | REM_DISCOVER_ELM_MAX_SIZE,0,NULL},
                                                 
    // Characteristic Configuration Descriptor
    [REMOTE_DISCOVER_ELM_IDX_CFG]        = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE)  | PERM(WRITE_REQ, ENABLE),
                                                sizeof(uint16_t), 0, NULL},

    // Characteristic User Description
    [REMOTE_DISCOVER_ELM_IDX_DESC]       = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                                sizeof(REM_DISCOVER_ELM_DESC) - 1, sizeof(REM_DISCOVER_ELM_DESC) - 1, (uint8_t*)REM_DISCOVER_ELM_DESC},
/// Write elm Characteristic        ==========================================================================================================
    //Characteristic Declaration
    [REMOTE_WRITE_ELM_IDX_CHAR]          = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN,PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                                0,0,NULL},
  
    // Characteristic Value
    [REMOTE_WRITE_ELM_IDX_VAL]           = {(uint8_t*)rem_write_elm_val.uuid, ATT_UUID_128_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE),
                                                PERM(RI, ENABLE) | REM_WRITE_ELM_MAX_SIZE,0,NULL},
                                                 
    // Characteristic Configuration Descriptor
    [REMOTE_WRITE_ELM_IDX_CFG]           = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE)  | PERM(WRITE_REQ, ENABLE),
                                                sizeof(uint16_t), 0, NULL},

    // Characteristic User Description
    [REMOTE_WRITE_ELM_IDX_DESC]          = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                                sizeof(REM_WRITE_ELM_DESC) - 1, sizeof(REM_WRITE_ELM_DESC) - 1,(uint8_t*) REM_WRITE_ELM_DESC},
/// Read elm Characteristic         ==========================================================================================================
    //Characteristic Declaration
    [REMOTE_READ_ELM_IDX_CHAR]           = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                                0,0,NULL},
  
     // Characteristic Value
    [REMOTE_READ_ELM_IDX_VAL]            = {(uint8_t*)rem_read_elm_val.uuid, ATT_UUID_128_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE),
                                                PERM(RI, ENABLE) | REM_READ_ELM_MAX_SIZE,0,NULL},

    // Characteristic Configuration Descriptor
    [REMOTE_READ_ELM_IDX_CFG]            = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE)  | PERM(WRITE_REQ, ENABLE),
                                                sizeof(uint16_t), 0, NULL},
                                                 
    // Characteristic User Description
    [REMOTE_READ_ELM_IDX_DESC]           = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                                sizeof(REM_READ_ELM_DESC) - 1, sizeof(REM_READ_ELM_DESC) - 1, (uint8_t*)REM_READ_ELM_DESC}
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Remote configuration task creation
 * @param[in] env       Pointer to the profile task environment
 * @param[in] start_hdl Pointer to the start handle
 * @param[in] app_task  Application Task Number 
 * @param[in] sec_lvl   Security Level
 * @param[in] features  Notification property status
 * @return status
 ****************************************************************************************
 */
static uint8_t remote_config_init(struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl, uint16_t features)
{
    
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;
    uint8_t remote_version_value[16];
    
    struct remote_config_env_tag* remote_config_env = NULL;

    memcpy(remote_version_value, user_remote_config_env.version, user_remote_config_env.version_length);
    status = attm_svc_create_db_128(REMOTE_CONFIG_IDX_SVC,start_hdl, NULL, REMOTE_CONFIG_IDX_NB,
             NULL, env->task, &remote_config_att_db[0],
            (sec_lvl & PERM_MASK_SVC_AUTH) | (sec_lvl & PERM_MASK_SVC_EKS) | PERM(SVC_PRIMARY, ENABLE));  
    
    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        remote_config_env = (struct remote_config_env_tag* ) ke_malloc(sizeof(struct remote_config_env_tag), KE_MEM_ATT_DB);

        // allocate REMOTE_CONFIG required environment variable
        env->env = (prf_env_t*) remote_config_env;
        remote_config_env->remote_config_shdl = *start_hdl;
        remote_config_env->max_nb_att = REMOTE_CONFIG_IDX_NB;
        remote_config_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        remote_config_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_REMOTE_CONFIG;
        env->desc.idx_max           = REMOTE_CONFIG_IDX_MAX;
        env->desc.default_handler   = &remote_config_default_handler;


       //service
        attmdb_att_set_value(remote_config_env->remote_config_shdl+REMOTE_VERSION_IDX_VAL, user_remote_config_env.version_length,0,(uint8_t *)&remote_version_value);
            
        // service is ready, go into an Idle state
        ke_state_set(env->task, REMOTE_CONFIG_IDLE);
    }
    else if(remote_config_env != NULL)
    {
        ke_free(remote_config_env);
    }
    

    return status;
}

/**
 ****************************************************************************************
 * @brief Destroys Remote configuration task
 * @param[in] env   Pointer to the profile task environment
 * @return void
 ****************************************************************************************
 */
static void remote_config_destroy (struct prf_task_env* env)
{  
    struct remote_config_env_tag* remote_config_ntf_env = (struct remote_config_env_tag*) env->env;

    // clear on-going operation
    if(remote_config_ntf_env->operation != NULL)
    {
        ke_free(remote_config_ntf_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(remote_config_ntf_env);
}

/**
 ****************************************************************************************
 * @brief Creates Remote configuration task
 * @param[in] env       Pointer to the profile task environment
 * @param[in] conidx    Connection index
 * @return void
 ****************************************************************************************
 */
static void remote_config_create(struct prf_task_env* env, uint8_t conidx)
{
    struct remote_config_env_tag* remote_config_env = (struct remote_config_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer remote is connected
    remote_config_env->discover_ntf_cfg = REMOTE_CONFIG_NTF_EN;
    remote_config_env->write_ntf_cfg = REMOTE_CONFIG_NTF_EN;
    remote_config_env->read_ntf_cfg = REMOTE_CONFIG_NTF_EN;
}

/**
 ****************************************************************************************
 * @brief Cleanup Remote configuration task
 * @param[in] env       Pointer to the profile task environment
 * @param[in] conidx    Connection index
 * @param[in] reason    Reason for the cleanup (not currently used)
 * @return void
 ****************************************************************************************
 */
static void remote_config_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct remote_config_env_tag* remote_config_env = (struct remote_config_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer remote is disconnected
    remote_config_env->discover_ntf_cfg = 0;
    remote_config_env->write_ntf_cfg = 0;
    remote_config_env->read_ntf_cfg = 0;
}

uint8_t remote_config_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct remote_config_env_tag* remote_config_env = PRF_ENV_GET(REMOTE_CONFIG, remote_config);
    uint8_t status = PRF_APP_ERROR;

    if (handle >= remote_config_env->remote_config_shdl && handle < remote_config_env->remote_config_shdl + remote_config_env->max_nb_att)
    {
        *att_idx = handle - remote_config_env->remote_config_shdl;
        status = GAP_ERR_NO_ERROR;
    }

    return status;
}

/**
 ****************************************************************************************
 * @brief  Remote_Config task callbacks
 ****************************************************************************************
 */
const struct prf_task_cbs rem_conf_itf =
{
    (prf_init_fnct) remote_config_init,
    remote_config_destroy,
    remote_config_create,
    remote_config_cleanup,
};


const struct prf_task_cbs* remote_config_prf_itf_get(void)
{
   return &rem_conf_itf;
}
#endif //BLE_REMOTE_CONFIG

/// @} REMOTE_CONFIG_GATT_SERVICE
