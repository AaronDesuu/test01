/**
 ****************************************************************************************
 *
 * @file user_sps_device_dma.c
 *
 * @brief DSPS Device project source code file .
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_DEVICE_CONFIG
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "app_api.h"
#include "arch_console.h"
#include "gattc.h"
#include "user_sps_device_dma.h"
#include "dma_uart_sps.h"
#include "user_sps_buffer_dma.h"
#include "user_sps_schedule_dma.h"
#include "gapc_task.h"
#include "prf_utils.h"
#include "user_remote_config.h"
#include "user_remote_config_task.h"
#include "user_config_storage.h"
#include "user_sps_device_config.h"
#include "app.h"

extern user_config_elem_t spss_configuration_table[USER_CONF_ELEMENTS_NUM];

timer_hnd features_timer;
uint8_t conn_idx;
bool dev_connected;
#if BLE_REMOTE_CONFIG
extern bool mtu_reset;
#endif

/*
 * EXTERNAL DEFINITIONS
 ****************************************************************************************
 */                                                                              
#if defined(CFG_BLE_METRICS) && defined(CFG_PRINTF)
extern struct sps_counter ble_data_counter;     
#endif
extern struct dma_uart_tag dma_uart;                                                                                 
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * @brief Sends a exchange MTU command
 * @param[in]   conidx Connection index
 * @return void
 ****************************************************************************************
 */
static void user_gattc_exc_mtu_cmd(uint8_t conidx)
{
    struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
                                                  KE_BUILD_ID(TASK_GATTC, conidx),
                                                  TASK_APP,
                                                  gattc_exc_mtu_cmd);

    cmd->operation=GATTC_MTU_EXCH;
    cmd->seq_num = 0;
    ke_msg_send(cmd);
}

/**
 ****************************************************************************************
 * @brief  Gets the peer device fatures
 * @return void
  ****************************************************************************************
 */
static void get_features(void)
{
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    sps_env->peer_supports_dle = 0;
    
    struct gapc_get_info_cmd* info_cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
                                                      KE_BUILD_ID(TASK_GAPC,conn_idx),
                                                      TASK_APP, 
                                                      gapc_get_info_cmd);
    // request peer device name.
    info_cmd->operation = GAPC_GET_PEER_FEATURES;
    
    // send command
    ke_msg_send(info_cmd);
} 

void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    default_app_on_connection(connection_idx, param);
    
    conn_idx = connection_idx;
    
    features_timer = app_easy_timer(60, get_features);
    
    dma_uart_on_connect();

    dev_connected = true;
}

void user_on_disconnect( struct gapc_disconnect_ind const *param )
{
#if BLE_REMOTE_CONFIG
    if (mtu_reset)
        platform_reset(RESET_NO_ERROR);
#endif
#if BLE_SUOTA_RECEIVER
    if (suota_state.reboot_requested)
        platform_reset(RESET_AFTER_SUOTA_UPDATE);
#endif

    dma_uart_set_tx_size(23);
    default_app_on_disconnect(param);
    if( dma_uart.p_rx_ready_active!=NULL)
    {    
        __disable_irq();
        co_list_push_front(&dma_uart.rx_list_ready, &dma_uart.p_rx_ready_active->hdr);
        dma_uart.p_rx_ready_active=NULL;
        __enable_irq();
    }    
    
    dma_uart_deassert_rts();  
    dma_uart_rx_disable();
}

void user_on_connect_failed (void)
{
    ASSERT_WARNING(0);
    dma_uart_deassert_rts();  
    dma_uart_rx_disable(); 
}

/*
 * MAIN LOOP CALLBACKS
 ****************************************************************************************
 */

void user_on_init(void)
{
#if BLE_REMOTE_CONFIG
    user_rem_config_function_t callback = NULL;
#endif
#ifdef CFG_CONFIG_STORAGE
    spss_conf_struct_version_len = strlen(spss_conf_struct_version);
#endif

    // SPS application initialization
    default_app_on_init();

#ifdef CFG_CONFIG_STORAGE
    user_conf_storage_init((user_config_elem_t *)spss_configuration_table, sizeof(spss_configuration_table)/sizeof(user_config_elem_t),
        spss_conf_struct_version, &spss_conf_struct_version_len);
#endif

    user_sps_apply_uart_baudrate();
    periph_init();

#if BLE_REMOTE_CONFIG
    callback = user_sps_apply_config;
    user_remote_config_init((user_config_elem_t *)spss_configuration_table, sizeof(spss_configuration_table)/sizeof(user_config_elem_t),
                            spss_conf_struct_version, spss_conf_struct_version_len, callback);
#endif

}

void user_on_db_init_complete(void)
{
    default_app_on_db_init_complete();
    user_init_queues();
}  

#if defined(CFG_BLE_METRICS) && defined(CFG_PRINTF)
void user_statistics(void){
    uint8_t ble_evt;
    uint8_t  sys_state=ke_state_get(TASK_APP);
    static uint8_t app_last_ble_evt=0;
    int16_t rx_rssi;
    
    if ( sys_state != APP_CONNECTED)
        return;
    
    ble_evt = arch_last_rwble_evt_get();
    if (ble_evt != app_last_ble_evt) 
    {
        app_last_ble_evt = ble_evt;
        if (app_last_ble_evt == BLE_EVT_END ){
            if (!(metrics.rx_pkt%40) ){
                rx_rssi = ((474 * metrics.rx_rssi) /1000)-112;
            }
        }            
   }    
    return;
}
#endif
 
void user_before_sleep(void)
{
    user_dma_sps_sleep_check();
}

void user_on_data_length_change(struct gapc_le_pkt_size_ind *param, uint16_t src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    user_gattc_exc_mtu_cmd(conidx);
}

/**
 ****************************************************************************************
 * @brief From the features indication msg it checks if DLE is supported
 * @param[in]   gapc_peer_features_ind pointer to features message indication parameters
 * @return void
 ****************************************************************************************
 */
static void check_features(struct gapc_peer_features_ind *param )
{
    volatile uint8_t features1 =   *(uint8_t *)&param->features[0];
    volatile uint8_t features2 =   *(uint8_t *)&param->features[1];
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    if( features1 & 0x20 )
    {
        sps_env->peer_supports_dle = 1;
    }
    else
    {
        sps_env->peer_supports_dle = 0;
    }
}

void user_on_gapc_peer_features_ind(const uint8_t conidx, struct gapc_peer_features_ind const *param)
{
    check_features( (struct gapc_peer_features_ind *)param);
    app_easy_gap_param_update_start(conidx);
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    struct gapc_cmp_evt const *cmp_param = param;
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    uint8_t conidx = KE_IDX_GET(src_id);

    switch(msgid)
    {
        case GAPC_CMP_EVT:
            switch(cmp_param->operation)
            {   
                case GAPC_GET_PEER_FEATURES:
                    if ( cmp_param->status == GAP_ERR_NO_ERROR )
                        break;
                    else if ( cmp_param->status == LL_ERR_UNSUPPORTED_REMOTE_FEATURE )
                    {   
                        sps_env->peer_supports_dle = 0;
                        app_easy_gap_param_update_start(conidx);
                    }   
                    else        
                    {   
                        sps_env->peer_supports_dle = 0;
                        app_easy_gap_param_update_start(conidx);
                    }   
                break;
                default:
                break;
            }                 
        break;
        default:
        break;  
    
    }
}

/**
 ****************************************************************************************
 * @brief Sets the default tx size
 * @return void
 ****************************************************************************************
 */
static void user_set_default_tx_size(void)
{
    //We need to set them in advance in case that length os not set
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    uint8_t conidx = sps_env->conidx;
    uint16_t mtu = gattc_get_mtu(conidx);

    sps_env->mtu = mtu;
    sps_env->tx_size =  mtu-3;
    sps_env->tx_wait_level =  (3*mtu)/4;
}

void user_on_update_params_complete(void)
{
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    uint8_t conidx=sps_env->conidx;
    
    #ifdef USE_DLE
    if( sps_env->peer_supports_dle )
    {   
        app_easy_gap_set_data_packet_length(conidx, 251, 2120);
        user_set_default_tx_size();
        user_gattc_exc_mtu_cmd(conidx);
    }   
    else
    #endif 
        user_gattc_exc_mtu_cmd(conidx);
}

void user_on_update_params_rejected(uint8_t status)
{
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    uint8_t conidx=sps_env->conidx;
    
    if( status == LL_ERR_LMP_COLLISION || status == LL_ERR_DIFF_TRANSACTION_COLLISION ) {
        app_easy_gap_param_update_start(conidx);
    }
    else
    {
        #ifdef USE_DLE
        if( sps_env->peer_supports_dle )
        {
            app_easy_gap_set_data_packet_length(conidx, 251, 2120);
            user_set_default_tx_size();
            user_gattc_exc_mtu_cmd(conidx);
        }
        else
        #endif
            user_gattc_exc_mtu_cmd(conidx);
    }
}

/**
 ****************************************************************************************
 * @brief Appends the device name in the advertising frame or the scan response frame
 * @param[in]   cmd     Pointer to the frame's parameters
 * @return void
 ****************************************************************************************
 */
static void user_append_device_name(struct gapm_start_advertise_cmd* cmd)
{
	if (device_info.dev_name.length > 0)
    {
        // Get remaining space in the Advertising Data ( plus 2 bytes are used for the length and flag bytes of the Device Name and 3 bytes for the AD type flags)
        uint16_t total_adv_space = 3 + cmd->info.host.adv_data_len + 2 + device_info.dev_name.length;
        // Get remaining space in the Scan Response Data ( plus 2 bytes are used for the length and flag bytes of the Device Name)
        uint16_t total_scan_space = cmd->info.host.scan_rsp_data_len + 2 + device_info.dev_name.length;

        if (total_adv_space <= ADV_DATA_LEN)
        {
                append_device_name(&cmd->info.host.adv_data_len,
                                                     device_info.dev_name.length,
                                                     &(cmd->info.host.adv_data[cmd->info.host.adv_data_len]),
                                                     device_info.dev_name.name);
        }
        else if (total_scan_space <= SCAN_RSP_DATA_LEN)
        {
                append_device_name(&cmd->info.host.scan_rsp_data_len,
                                                     device_info.dev_name.length,
                                                     &(cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len]),
                                                     device_info.dev_name.name);
        }
	}
}

void user_on_adv_undirect_complete(uint8_t status)
{
	
	if ((status == GAP_ERR_INVALID_PARAM) || (status == GAP_ERR_ADV_DATA_INVALID))
	{
				struct gapm_start_advertise_cmd* cmd;
				cmd = app_easy_gap_undirected_advertise_get_active();
		
				user_append_device_name(cmd); 
				// Send the message
				app_easy_gap_undirected_advertise_start();
				 
				// We are now connectable
				ke_state_set(TASK_APP, APP_CONNECTABLE);
	}
}

void user_advertise_operation(void)
{

        struct gapm_start_advertise_cmd* cmd;
				cmd = app_easy_gap_undirected_advertise_get_active();

			  cmd->channel_map = user_adv_conf.channel_map;
			  cmd->intv_max = user_adv_conf.intv_max;
			  cmd->intv_min = user_adv_conf.intv_min;
			  cmd->info.host.adv_data_len = user_advertise_data_len;
				memcpy(cmd->info.host.adv_data, user_advertise_data, cmd->info.host.adv_data_len);
			  cmd->info.host.scan_rsp_data_len = user_adv_scan_resp_data_len;
			  memcpy(cmd->info.host.scan_rsp_data, user_adv_scan_resp_data, cmd->info.host.scan_rsp_data_len);
	
				user_append_device_name(cmd); 
				// Send the message
				app_easy_gap_undirected_advertise_start();
				 
				// We are now connectable
				ke_state_set(TASK_APP, APP_CONNECTABLE);
    
}

/// @} DSPS_DEVICE_CONFIG
