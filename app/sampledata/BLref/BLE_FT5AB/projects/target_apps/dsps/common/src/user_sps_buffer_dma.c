/**
 ****************************************************************************************
 *
 * @file user_sps_buffer_dma.c
 *
 * @brief SPS project source file for handling DMA buffers.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_SERVICE_COMMON
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdlib.h>

#include "ke_mem.h"
#include "arch_console.h"
#include "user_periph_setup.h"
#include "gpio.h"
#include "app_easy_timer.h" 
#include "ke_msg.h" 
#include "prf.h" 
#include "prf_utils.h"
#include "lld_evt.h"
#include "dma.h"
#include "user_sps_utils.h"
#include "dma_uart_sps.h"
#include "user_sps_buffer_dma.h"
#include "user_sps_schedule_dma.h"

#if BLE_SPS_CLIENT
#include "sps_client.h"
#include "user_spsc.h"
#include "user_sps_host_dma.h"
#elif BLE_SPS_SERVER
#include "sps_server.h"
#include "user_spss.h"
#include "user_sps_device_dma.h"
#endif
 
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */ 
extern struct dma_uart_tag dma_uart;

void user_init_queues(void)
{
    memset((void *)&dma_uart, 0, sizeof(struct dma_uart_tag)); 
    
    co_list_init(&dma_uart.rx_list_ready);
    co_list_init(&dma_uart.tx_list_ready);
    
    dma_uart.p_rx_active = NULL;
    dma_uart.p_tx_active = NULL;
    dma_uart.p_rx_ready_active = NULL;
    
    dma_uart.lists_initialized = true;
}
    
void user_ble_to_dma_uart(void *msg)
{
    struct ke_msg *msg_tx = ke_param2msg( msg ); 
    
    __disable_irq();
    co_list_push_back(&dma_uart.tx_list_ready, &msg_tx->hdr);
    __enable_irq();
}
 
bool user_dma_uart_to_ble (void)
{
    #if BLE_SPS_SERVER    
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    #else
    struct spsc_env_tag *sps_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    #endif
    if (!sps_env->tx_flow_en)
    {
        return false;
    }
    __disable_irq();
    if( dma_uart.rx_list_ready.cnt !=0 )
    {
        __enable_irq(); 
        
        if( dma_uart.p_rx_ready_active != 0 )
            return false;
        __disable_irq();
        dma_uart.p_rx_ready_active = (struct ke_msg *)co_list_pop_front(&dma_uart.rx_list_ready);
        __enable_irq();
        
        struct UART_RX_STRUCT * req = (struct UART_RX_STRUCT *)ke_msg2param(dma_uart.p_rx_ready_active );
        req->length = dma_uart.p_rx_ready_active->param_len;
        ASSERT_ERROR(req);
        ke_msg_send(req);
        return true;
    }
    __enable_irq();
    
    return false;  
}

void user_dma_uart_to_ble_confirm(bool success)
{
    #if BLE_SPS_SERVER    
    struct spss_env_tag *sps_env = PRF_ENV_GET(SPS_SERVER, spss);
    #else
    struct spsc_env_tag *sps_env = PRF_ENV_GET(SPS_CLIENT, spsc);
    #endif
    
    if ( success ) {
        dma_uart.p_rx_ready_active = NULL;
        user_dma_uart_to_ble();
    }
    else 
    {   
        if (!sps_env->tx_flow_en)
        {
            return;
        }
       
        struct UART_RX_STRUCT * req = (struct UART_RX_STRUCT *)ke_msg2param( dma_uart.p_rx_ready_active );
        req->length = dma_uart.p_rx_ready_active->param_len;
        ke_msg_send(req);
    }
}
/// @} DSPS_SERVICE_COMMON
