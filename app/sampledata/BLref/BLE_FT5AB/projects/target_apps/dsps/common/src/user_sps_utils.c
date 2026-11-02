 /**
 ****************************************************************************************
 *
 * @file user_sps_utils.c
 *
 * @brief Serial Port Service Server Utilites
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
#include "rwip.h"
#include "ke_env.h"
#include "ke.h"
#include "lld_sleep.h"
#include "ea.h"
#include "prf_utils.h"
#include "gpio.h"
#include "user_periph_setup.h"
#include "user_sps_buffer_dma.h"
#include "user_sps_utils.h"

#if BLE_SPS_CLIENT
#include "sps_client.h"
#include "user_spsc.h"
#elif BLE_SPS_SERVER
#include "sps_server.h"
#include "user_spss.h"
#endif

 /*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

void *ke_msg_alloc_no_init(ke_msg_id_t const id, ke_task_id_t const dest_id,
                           ke_task_id_t const src_id, uint16_t const param_len)
{
    struct ke_msg *msg = (struct ke_msg*) ke_malloc(sizeof(struct ke_msg) + param_len - sizeof (uint32_t), KE_MEM_KE_MSG);
    void *param_ptr = NULL;

    ASSERT_ERR(msg != NULL);
    msg->hdr.next  = KE_MSG_NOT_IN_QUEUE;
    msg->id        = id;
    msg->dest_id   = dest_id;
    msg->src_id    = src_id;
    msg->param_len = param_len;
    #if defined (__DA14531__) 
    msg->saved     = 0;
    #endif

    param_ptr = ke_msg2param(msg);

    return param_ptr;
}

#ifdef DMA_UART
sleep_mode_t user_rwip_sleep_check(void)
{
    sleep_mode_t proc_sleep = mode_active;
    uint32_t sleep_duration =rom_cfg_table[max_sleep_duration_external_wakeup_pos];

#if (BLE_APP_PRESENT)
    if ( arch_ble_ext_wakeup_get() || (rwip_env.ext_wakeup_enable == 2) )  // sleep forever!
        sleep_duration = 0;
#else
#if (!EXTERNAL_WAKEUP) // sleep_duration will remain as it was set above....
    if (rwip_env.ext_wakeup_enable == 2)
        sleep_duration = 0;
#endif
#endif
    
    do
    {
        
        // Do not allow sleep if system is in startup period
        if (check_sys_startup_period())
            break;
        
        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (!ke_sleep_check())
            break;
        
        // Processor sleep can be enabled
        proc_sleep = mode_idle;
        
        /************************************************************************
         **************           CHECK RADIO POWER DOWN           **************
         ************************************************************************/
        // Check if BLE + Radio are still sleeping
        if(GetBits16(SYS_STAT_REG, RAD_IS_DOWN)) {
            // If BLE + Radio are in sleep return the appropriate mode for ARM
            proc_sleep = mode_sleeping;
            break;
        }

        /************************************************************************
         **************              CHECK RW FLAGS                **************
         ************************************************************************/
        // First check if no pending procedure prevents us from going to sleep
        if (rwip_prevent_sleep_get() != 0)
            break;

        /************************************************************************
         **************           CHECK EXT WAKEUP FLAG            **************
         ************************************************************************/
        /* If external wakeup is enabled, sleep duration can be set to maximum, otherwise
         * the system must be woken-up periodically to poll incoming packets from HCI */
        if((BLE_APP_PRESENT == 0) || (BLE_INTEGRATED_HOST_GTL == 1 )) // No need for periodic wakeup if we have full-hosted system
        {
            if(!rwip_env.ext_wakeup_enable)
                sleep_duration = rom_cfg_table[max_sleep_duration_periodic_wakeup_pos];
        }

        
        
        /************************************************************************
         *                                                                      *
         *                   CHECK DURATION UNTIL NEXT EVENT                    *
         *                                                                      *
         ************************************************************************/
        // If there's any timer pending, compute the time to wake-up to serve it
        if (ke_env.queue_timer.first != NULL)  
            sleep_duration =  rom_cfg_table[max_sleep_duration_external_wakeup_pos];

        /************************************************************************
         **************            CHECK KERNEL TIMERS             **************
         ************************************************************************/
        // Compute the duration up to the next software timer expires
        if (!ke_timer_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        
        #if (BLE_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BLE                  **************
         ************************************************************************/
        // Compute the duration up to the next BLE event
        if (!ea_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        #endif // BLE_EMB_PRESENT
        
        proc_sleep = mode_sleeping;
     } while(0);
     
     return proc_sleep;
}
#endif
/// @} DSPS_SERVICE_COMMON
