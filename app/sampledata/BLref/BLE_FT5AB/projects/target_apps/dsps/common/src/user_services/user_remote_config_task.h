/**
 ****************************************************************************************
 *
 * @file  user_remote_config_task.h
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

#ifndef USER_REMOTE_CONFIG_TASK_H_
#define USER_REMOTE_CONFIG_TASK_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if BLE_REMOTE_CONFIG

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
enum process_event_response user_remote_config_process_handler(ke_msg_id_t const msgid,
                            void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id,
                            enum ke_msg_status_tag *msg_ret);
                            
                               
#endif //BLE_REMOTE_CONFIG

/// @}

#endif // USER_REMOTE_CONFIG_TASK_H_
