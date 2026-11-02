/**
 ****************************************************************************************
 *
 * @file user_sps_device_dma.h
 *
 * @brief DSPS Device project source code include file.
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

#ifndef _USER_SPS_DEVICE_DMA_H_
#define _USER_SPS_DEVICE_DMA_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions

/****************************************************************************
Add here supported profiles' application header files.
i.e.
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif
*****************************************************************************/
#include "user_spss.h"
#include "user_spss_task.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */


#ifdef DEBUG_PRINT
#define DBG_PRN(...) arch_printf(__VA_ARGS__) 
#else
#define DBG_PRN(...)
#endif

#ifdef DEBUG_PRINT_L2
#define DBG_PRN2(...) arch_printf(__VA_ARGS__)  // 2nd Level debug
#else
#define DBG_PRN2(...)
#endif
 
#define DBG1_ON GPIO_SetActive( GPIO_DBG1_PORT, GPIO_DBG1_PIN);
#define DBG1_OFF GPIO_SetInactive( GPIO_DBG1_PORT, GPIO_DBG1_PIN);

#define DBG2_ON GPIO_SetActive( GPIO_DBG2_PORT, GPIO_DBG2_PIN);
#define DBG2_OFF GPIO_SetInactive( GPIO_DBG2_PORT, GPIO_DBG2_PIN);
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles connection event
 * @param[in]   connection_idx Connection index
 * @param[in]   param Parameters of connection
 * @return void
 ****************************************************************************************
 */
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles disconnection event
 * @param[in]   param Parameters of disconnect message
 * @return void
 ****************************************************************************************
 */
void user_on_disconnect(struct gapc_disconnect_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles connection failure
 * @return void
 ****************************************************************************************
 */
void user_on_connect_failed (void);

/**
 ****************************************************************************************
 * @brief Handles GAPC peer features indication
 * @param[in]   conidx  Connection index
 * @param[in]   param   pointer to features message indication parameters
 * @return void
 ****************************************************************************************
 */
void user_on_gapc_peer_features_ind(const uint8_t conidx, struct gapc_peer_features_ind const *param);

/*
 * MAIN LOOP CALLBACKS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Called on device initialization
 * @return void
 ****************************************************************************************
 */
void user_on_init(void);

/**
 ****************************************************************************************
 * @brief Called before entering sleep
 * @return void
 ****************************************************************************************
 */
void user_before_sleep(void);

/**
 ****************************************************************************************
 * @brief Called on database initialization complete
 * @return void
 ****************************************************************************************
 */
void user_on_db_init_complete(void);

#if defined(CFG_BLE_METRICS) && defined(CFG_PRINTF)
/**
 ****************************************************************************************
 * @brief BLE statistics print
 * @return void
 ****************************************************************************************
 */
void user_statistics(void);
#endif

/**
 ****************************************************************************************
 * @brief Called on the reception of GAPC_LE_PKT_SIZE_IND
 * @return void
 ****************************************************************************************
 */
void user_on_data_length_change (struct gapc_le_pkt_size_ind *param, uint16_t src_id);

/**
 ****************************************************************************************
 * @brief Handles the messages that are not handled by the SDK internal mechanisms.
 * @param[in] msgid      Id of the message received.
 * @param[in] param      Pointer to the parameters of the message.
 * @param[in] dest_id    ID of the receiving task instance.
 * @param[in] src_id     ID of the sending task instance.
 * @return void
 ****************************************************************************************
*/
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Callback of the GAPC_UPDATE_PARAMS (parameter update is complete)
 * @return void
 ****************************************************************************************
 */
void user_on_update_params_complete(void);

/**
 ****************************************************************************************
 * @brief Callback of the GAPC_UPDATE_PARAMS rejection (parameter update is rejected)
 * @param[in]   status status of the message
 * @return void
 ****************************************************************************************
 */
void user_on_update_params_rejected(uint8_t status);
		
/**
 ****************************************************************************************
 * @brief Callback of the GAPM_ADV_UNDIRECT operation completion (GAPM_CMP_EVENT). Undirected advertise is completed. 
 *				Check if status indicates that parameters are rejected and restart advertise with default values.
 * @param[in]   status status of the message
 * @return void
 ****************************************************************************************
 */													
void user_on_adv_undirect_complete(uint8_t status);
	
/**
 ****************************************************************************************
 * @brief Default advertise operation. Overrides standard operation function to use user configuration values. 
 * @return void
 ****************************************************************************************
 */
void user_advertise_operation(void);

#endif //_USER_SPS_DEVICE_DMA_H_

/// @} DSPS_DEVICE_CONFIG
