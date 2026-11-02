/**
 ****************************************************************************************
 *
 * @file user_sps_schedule_dma.c
 *
 * @brief SPS Scheduling of DMA SPS operations
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
#include "user_sps_buffer_dma.h"
#include "gpio.h"
#include "app_easy_timer.h" 
#include "ke_msg.h" 
#include "prf.h" 
#include "prf_utils.h"
#include "lld_evt.h"
#include "dma.h"
#include "user_sps_utils.h"

#include "dma_uart_sps.h"
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
extern dma_cfg_t DMA_Tx_setup;
extern dma_cfg_t DMA_Rx_setup;
extern bool dev_connected;
extern bool rx_cb_done;

extern uint8_t heap_usage;

void user_dma_sps_sleep_check(void)
{
    uint32_t rx_length;
    if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
    {
        do 
        {
            if ( dma_uart.p_rx_ready_active != NULL )
                break;
            
            if (user_rwip_sleep_check() != mode_sleeping)
                break;
            
            if (dma_uart.tx_list_ready.cnt>0)
                break;
            
            if (dma_uart.rx_list_ready.cnt>0)
                break;
            
            /* DMA TX active */
            if ((GetWord32(DMA1_CTRL_REG)&DMA_ON)==1)
                break;
            
            if (!dma_uart_rx_check_empty(&rx_length) )
                break;
            
            if (dev_connected == true )
            {
                if( dma_uart.rx_zero_data_cnt < 50 )
                    break;
            }
            
           // if( !uart_temt_getf())
            if (dev_connected == true )
            {
                if (!uart_tx_empty_getf(UART1))
                    break;
            }
            
            dma_uart_deassert_rts();
            
            // Wait for 1 character duration to ensure host has not started a transmission at the
            // same time
            
            for (int i=0;i<uart_wait_byte_counter;i++);
            
            if( !dma_uart_rx_check_empty(&rx_length) )
            {
                dma_uart_assert_rts();
                break;
            }   
            
            dma_uart_rx_disable();
            
            if( dma_uart_check_active())
                break;
            
            dma_uart.power_on_state = PON_STATE_IDLE;
            
            arch_set_extended_sleep(0);
            return;
            
        }
        while(0);
    }
    arch_disable_sleep();
};


void user_check_dma_uart_rx_to(void)
{
    uint32_t rx_length;
    if( dma_uart_rx_check_empty( &rx_length ) == false )
    {
        if( dma_uart.rx_cnt_prev == rx_length )
        {
            dma_uart.rx_to_cnt++;
            if (dma_uart.rx_to_cnt>DMA_UART_RX_TO_ROUNDS)
            {
                dma_uart_timeout(); 
            }
        }
        else
        {
            //dma_uart.rx_cnt_prev=0;
            dma_uart.rx_to_cnt = 0;
            dma_uart.rx_cnt_prev = rx_length;
        }
        dma_uart.rx_zero_data_cnt = 0;
    } else {
        dma_uart.rx_zero_data_cnt++;
    }
}

bool user_check_set_flow_off(void)
{
    __disable_irq(); 
    #if (USE_BLE_FLOW_CONTROL)
    extern uint8_t get_heap_usage(void);
    uint8_t heap_usage = get_heap_usage();
    if ((heap_usage > RX_FLOW_OFF_HEAP_LIMIT) && (dma_uart.uart_flow_off == 0) && dma_uart.rx_list_ready.cnt > 1)
    #else
    if ((dma_uart.rx_list_ready.cnt > RX_FLOW_OFF_LVL) && (dma_uart.uart_flow_off == 0))
    #endif
    {
        __enable_irq();
        dma_uart.uart_flow_off = 1;
        dma_uart_deassert_rts(); 
        return true;
    }   
    
    __enable_irq(); 
    return false;
}

bool user_check_set_flow_on(void)
{
    __disable_irq();
    #if (USE_BLE_FLOW_CONTROL)
    extern uint8_t get_heap_usage(void);
    uint8_t heap_usage = get_heap_usage();
    if ((heap_usage < RX_FLOW_ON_HEAP_LIMIT) && (dma_uart.uart_flow_off == 1))
    #else
    if ((dma_uart.rx_list_ready.cnt < RX_FLOW_ON_LVL) && (dma_uart.uart_flow_off == 1))
    #endif
    {
        __enable_irq();
        dma_uart.uart_flow_off = 0;
        //dma_uart_assert_rts();
        return true;
    }   
    __enable_irq(); 
    return false;
}

arch_main_loop_callback_ret_t user_on_ble_powered(void)
{
    uint8_t  sys_state;
    
    if(rx_cb_done == true)
    {
        if( !user_check_set_flow_off())
        {
            dma_uart_rx_activate();
            dma_uart_assert_rts();
        }
        rx_cb_done = false;
    }
    switch( dma_uart.power_on_state )
    {
        case PON_STATE_IDLE:
            sys_state=ke_state_get(TASK_APP);
            if( sys_state != APP_CONNECTED )
                break;
            if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
            {
                if(dma_uart.lists_initialized == true)
                {
                    dma_uart_sps_init(cfg_uart_sps_baudrate, 3);
                    dma_uart_rx_activate();
                    dma_uart_assert_rts();
                    dma_uart.power_on_state = PON_STATE_WAIT_RTS;
                }   
            }
            else
            {
                dma_uart.power_on_state = PON_STATE_WAIT_RTS;
            }
        case PON_STATE_WAIT_RTS:
           dma_uart_tx_async();
           user_check_dma_uart_rx_to();
           user_dma_uart_to_ble();
           dma_uart_rx_reactivate();
           dma_uart_test_tx_queue_for_flow_on();  
        break;
        default:
        break;
    }   
    
    return GOTO_SLEEP;
}
/// @} DSPS_SERVICE_COMMON
