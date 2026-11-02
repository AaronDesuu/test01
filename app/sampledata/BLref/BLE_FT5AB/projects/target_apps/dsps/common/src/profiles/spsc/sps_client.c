/**
 ****************************************************************************************
 *
 * @file sps_client.c
 *
 * @brief SPS client implementation.
 *
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
#include "prf.h"
#include "sps_client.h"
#include "sps_client_task.h"
#include "prf_utils.h"
#include "prf_utils_128.h"
#include "arch_console.h"
#ifdef DMA_UART
#include "user_sps_host_dma.h"
#else
#include "user_sps_host.h"
#endif


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 struct sps_counter ble_data_counter;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief     Profile Initialization
 * @param[in] env enviroment 
 * @param[in] start_hdl start handle 
 * @param[in] app_task application task
 * @param[in] sec_lvl security level
 * @param[in] params configuration parameters
 * @return    success
 ****************************************************************************************
 */
static uint8_t spsc_init(struct prf_task_env *env, uint16_t *start_hdl, uint16_t app_task, uint8_t sec_lvl, struct spsc_db_cfg *params)
{
    //-------------------- allocate memory required for the profile  ---------------------

    struct spsc_env_tag* spsc_env =
            (struct spsc_env_tag* ) ke_malloc(sizeof(struct spsc_env_tag), KE_MEM_ATT_DB);
    
    memset (spsc_env, 0, sizeof(struct spsc_env_tag));

    // allocate SPSC required environment variable
    env->env = (prf_env_t*) spsc_env;

    spsc_env->prf_env.app_task = app_task | PERM(PRF_MI, DISABLE);
    spsc_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

    // initialize environment variable
    env->id                     = TASK_ID_SPS_CLIENT;
    env->desc.idx_max           = SPSC_IDX_MAX;
    env->desc.state             = spsc_env->state;
    env->desc.default_handler   = &sps_client_default_handler;

    ke_state_set(spsc_env->prf_env.prf_task, SPS_CLIENT_IDLE);
   

    return GAP_ERR_NO_ERROR;
}

void sps_client_enable_cfm_send(struct spsc_env_tag *spsc_env, uint8_t conidx, uint8_t status)
{
   
    struct sps_client_enable_cfm * cfm = KE_MSG_ALLOC(SPS_CLIENT_ENABLE_CFM,
                                                      TASK_APP,
                                                      prf_src_task_get(&(spsc_env->prf_env), conidx),
                                                      sps_client_enable_cfm);
    
    cfm->status = status;

    if (status == ATT_ERR_NO_ERROR)
    {
        uint8_t flow;
        
        cfm->sps  = spsc_env->sps;
        
        // Register SPS_CLIENT task in gatt for indication/notifications
         
        //prf_register_atthdl2gatt(&(spsc_env->prf_env), conidx, NULL); 
        prf_register_atthdl2gatt(&(spsc_env->prf_env), conidx, &spsc_env->sps.svc); 
        
        //Set value 0x0001 to CFG
        prf_gatt_write_ntf_ind(&spsc_env->prf_env, conidx, spsc_env->sps.descs[SPSC_SRV_TX_DATA_CLI_CFG].desc_hdl, PRF_CLI_START_NTF);
        prf_gatt_write_ntf_ind(&spsc_env->prf_env, conidx, spsc_env->sps.descs[SPSC_FLOW_CTRL_CLI_CFG].desc_hdl, PRF_CLI_START_NTF);
        
        //Initialise flow flags
        spsc_env->tx_flow_en = true;
        spsc_env->rx_flow_en = true;
        
        // Reset counter
        spsc_env->pending_wr_no_rsp_cmp = 0;
        spsc_env->pending_tx_ntf_cmp = false;

        flow = (spsc_env->rx_flow_en ? FLOW_ON : FLOW_OFF);
        prf_gatt_write(&spsc_env->prf_env, conidx, spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl, &flow, sizeof (uint8_t),GATTC_WRITE_NO_RESPONSE);
        spsc_env->pending_wr_no_rsp_cmp++;
        //Place in connected state after discover state   
        ke_state_set(spsc_env->prf_env.prf_task, SPS_CLIENT_CONNECTED);
    }
    ke_msg_send(cfm);
}

void sps_client_confirm_data_tx(struct spsc_env_tag *spsc_env, uint8_t status)
{
    struct sps_client_data_tx_cfm * cfm = KE_MSG_ALLOC(SPS_CLIENT_DATA_TX_CFM,
                                                       TASK_APP,
                                                       prf_src_task_get(&(spsc_env->prf_env), 0),
                                                       sps_client_data_tx_cfm);

    cfm->status = status;

    ke_msg_send(cfm);
}

void sps_client_data_receive(struct spsc_env_tag *spsc_env, void *msg )
{
    // Forward the message
    struct ke_msg * kmsg = ke_param2msg(msg);
    kmsg->dest_id = TASK_APP;
    kmsg->src_id  = prf_src_task_get(&(spsc_env->prf_env), 0);
    kmsg->id      = SPS_CLIENT_DATA_RX_IND;
    
    struct sps_client_data_rx_ind *req =  ( struct sps_client_data_rx_ind *) msg; 
    
    ke_msg_send(req);
}

void sps_client_indicate_tx_flow_ctrl(struct spsc_env_tag *spsc_env, uint8_t flow_ctrl)
{
    struct sps_client_tx_flow_ctrl_ind * ind = KE_MSG_ALLOC(SPS_CLIENT_TX_FLOW_CTRL_IND,
                                                            TASK_APP,
                                                            prf_src_task_get(&(spsc_env->prf_env), 0),
                                                            sps_client_tx_flow_ctrl_ind);

    ind->flow_control_state = flow_ctrl;

    ke_msg_send(ind);
}

uint8_t spsc_get_next_desc_char_code(struct spsc_env_tag *spsc_env,
                                     const struct prf_char_desc_def *desc_def)
{
    uint8_t i;
    uint8_t next_char_code;

    for (i=0; i<SPSC_DESC_MAX; i++)
    {
        next_char_code = desc_def[i].char_code;

        if (next_char_code > spsc_env->last_char_code)
        {
            //If Char. has been found and number of attributes is upper than 2
            if ((spsc_env->sps.chars[next_char_code].char_hdl != 0)
                    & (spsc_env->sps.chars[next_char_code].char_ehdl_off > 2))
            {
                return next_char_code;
            }
        }
    }

    return SPSC_CHAR_MAX;
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 * @param[in|out]    env        environment data.
 * @param[in]        conidx     Connection index
 * @return    void
 ****************************************************************************************
 */
static void spsc_create(struct prf_task_env *env, uint8_t conidx)
{
     struct spsc_env_tag *sps_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    
     ke_state_set(env->task, SPS_CLIENT_CONNECTED);
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 * @return    void
 ****************************************************************************************
 */
static void spsc_cleanup(struct prf_task_env *env, uint8_t conidx, uint8_t reason)
{
    int i;
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    for(i=0;i<SPSC_DESC_MAX;i++)
        spsc_env->sps.descs[i].desc_hdl=ATT_INVALID_HANDLE;
    ke_state_set(env->task, SPS_CLIENT_IDLE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the sps module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @return    void
 ****************************************************************************************
 */
static void spsc_destroy(struct prf_task_env *env)
{
    struct spsc_env_tag *spsc_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    
    // free profile environment variables
    env->env = NULL;
    ke_free(spsc_env);
}

void sps_client_write_data_tx(void *msg, prf_env_t *prf_env, uint8_t conidx, uint16_t handle, uint8_t* value, uint16_t length, uint8_t operation)
{
    if(handle != ATT_INVALID_HANDLE)
    {
        struct ke_msg * kmsg = ke_param2msg(msg);
        
        kmsg->dest_id = KE_BUILD_ID(TASK_GATTC, conidx);
        kmsg->src_id  =  prf_src_task_get(prf_env, conidx);
        kmsg->id = GATTC_WRITE_CMD;
        
        struct gattc_write_cmd *wr_char =  ( struct gattc_write_cmd *) msg; 
            
        // Offset
        wr_char->offset         = 0x0000;
        // cursor always 0
        wr_char->cursor         = 0x0000;
        // Write Type
        wr_char->operation       = operation;
        // Characteristic Value attribute handle
        wr_char->handle         = handle;
        // Value Length
        wr_char->length         = length;
        // Auto Execute
        wr_char->auto_execute   = true;
        // Value
        
        
        // Send the message
        ke_msg_send(wr_char);
    }
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// SPS Task interface required by profile manager
const struct prf_task_cbs spc_itf =
{
        (prf_init_fnct) spsc_init,
        spsc_destroy,
        spsc_create,
        spsc_cleanup,
};

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* spsc_prf_itf_get(void)
{
    return &spc_itf;
}


#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENT
