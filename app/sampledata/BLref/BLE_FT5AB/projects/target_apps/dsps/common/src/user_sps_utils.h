
 /**
 ****************************************************************************************
 *
 * @file user_sps_utils.h
 *
 * @brief Serial Port Service Server Implementation.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DSPS_SERVICE_COMMON    Common
 * @addtogroup DSPS_SERVICE_COMMON
 * @ingroup DSPS_APPLICATION
 * @brief DSPS Services Common.
 * @{
 ****************************************************************************************
 */

#ifndef _USER_SPS_UTILS_H_
#define _USER_SPS_UTILS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
/**
 ****************************************************************************************
 * @brief Allocate memory for a message
 *
 * This primitive allocates memory for a message that has to be sent. The memory
 * is allocated dynamically on the heap and the length of the variable parameter
 * structure has to be provided in order to allocate the correct size.
 *
 * Several additional parameters are provided which will be preset in the message
 * and which may be used internally to choose the kind of memory to allocate.
 *
 * The memory allocated will be automatically freed by the kernel, after the
 * pointer has been sent to ke_msg_send(). If the message is not sent, it must
 * be freed explicitly with ke_msg_free().
 *
 * Allocation failure is considered critical and should not happen.
 * The difference with ke_msg_alloc is that it does not initialize paramaters to 0
 *
 * @param[in] id        Message identifier
 * @param[in] dest_id   Destination Task Identifier
 * @param[in] src_id    Source Task Identifier
 * @param[in] param_len Size of the message parameters to be allocated
 *
 * @return Pointer to the parameter member of the ke_msg. If the parameter
 *         structure is empty, the pointer will point to the end of the message
 *         and should not be used (except to retrieve the message pointer or to
 *         send the message)
 ****************************************************************************************
 */
 void *ke_msg_alloc_no_init(ke_msg_id_t const id, ke_task_id_t const dest_id,
                            ke_task_id_t const src_id, uint16_t const param_len);
 
 /**
 ****************************************************************************************
 * @brief Convenient wrapper to ke_msg_alloc_no_init()
 *
 * This macro calls ke_msg_alloc() and cast the returned pointer to the
 * appropriate structure with a variable length. Can only be used if a parameter structure exists
 * for this message (otherwise, use ke_msg_send_basic()).Can only be used if the data array is
 * located at the end of the structure. The difference with KE_MSG_ALLOC_DYN_NO_INIT is that it
 * does not initialize to zero the parameter data thus gaining speed.
 *
 * @param[in] id        Message identifier
 * @param[in] dest      Destination Identifier
 * @param[in] src       Source Identifier
 * @param[in] param_str parameter structure tag
 * @param[in] length    length for the data
 *
 * @return Pointer to the parameter member of the ke_msg.
 ****************************************************************************************
 */
#define KE_MSG_ALLOC_DYN_NO_INIT(id, dest, src, param_str,length)  (struct param_str*)ke_msg_alloc_no_init(id, dest, src, \
    (sizeof(struct param_str) + length));

/**
 ****************************************************************************************
 * @brief  Performs a pre-check of the kernel an ble events in order to flow off serial interface. 
 *
 * @return sleep_mode_t Sleep mode
 ****************************************************************************************
 */
#ifdef DMA_UART
sleep_mode_t user_rwip_sleep_check(void);
#endif
 
#endif // _USER_SPS_UTILS_H_

/// @} DSPS_SERVICE_COMMON
