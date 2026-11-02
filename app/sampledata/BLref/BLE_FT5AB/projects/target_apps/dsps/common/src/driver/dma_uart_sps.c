/**
 ****************************************************************************************
 *
 * @file dma_uart_sps.c
 *
 * @brief SPS project source code for dma uart handling functions
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @addtogroup UART
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
#include "gpio.h"
#include "app_easy_timer.h" 
#include "ke_msg.h" 
#include "prf.h" 
#include "prf_utils.h"
#include "lld_evt.h"
#include "dma.h"
#include "user_sps_utils.h"
#if BLE_SPS_CLIENT
#include "sps_client.h"
#include "user_spsc.h"
#include "user_sps_host_dma.h"
#elif BLE_SPS_SERVER
#include "sps_server.h"
#include "user_spss.h"
#include "user_sps_device_dma.h"
#endif
#include "user_periph_setup.h"
#include "dma_uart_sps.h"
#include "user_sps_buffer_dma.h"
#include "user_sps_schedule_dma.h"

/// Struct dma_uart
struct dma_uart_tag dma_uart __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY;

dma_cfg_t DMA_Tx_setup  __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY;
dma_cfg_t DMA_Rx_setup  __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY;

uart_cfg_t dma_uart_cfg __attribute__((section("retention_mem_area0"),zero_init));
#define DMA_CHANNEL_INVALID 0    

#define DSPS_UART UART1
#define DMA_RX_CHANNEL_NUMBER DMA_CHANNEL_0
#define DMA_TX_CHANNEL_NUMBER DMA_CHANNEL_1

//Fix to control IOS Tx to BLE when 9600 baudrate is used
//otherwise less data than expected was sent from IOS to BLE
//It is needed to lower BLE_FLOW_OFF limit in order to avoid data loss
#if defined (__DA14531__) && (BAUDRATE_CONFIG==1)
#define TX_FLOW_ON_LVL (8)
#define TX_FLOW_OFF_LVL (10)
#else
#define TX_FLOW_ON_LVL (12)
#define TX_FLOW_OFF_LVL (16)
#endif


volatile bool rx_cb_done;

void inline dma_uart_assert_rts(void)
{
    GPIO_SetInactive( gpio_uart1_rts.port, gpio_uart1_rts.pin );
}

void inline dma_uart_deassert_rts(void)
{
    GPIO_SetActive( gpio_uart1_rts.port, gpio_uart1_rts.pin );
}

void dma_uart_rx_reactivate(void)
{
    if( user_check_set_flow_on())
    {
        dma_uart_rx_activate();
        dma_uart_assert_rts();
    }
}
 
bool dma_uart_rx_check_empty(uint32_t *rx_length)
{
    volatile uint16_t rx_len = dma_get_idx(DMA_RX_CHANNEL_NUMBER);
    if ( rx_len == 0 ){
        *rx_length = 0; 
        return true;
    }
    else
    {
        *rx_length = rx_len;
        return false;
    }                   
}

void dma_uart_rx_disable(void)
{
    if( (GetWord32(DMA0_CTRL_REG)&DMA_ON)==1) {
        dma_channel_stop(DMA_RX_CHANNEL_NUMBER);
        dma_clear_int_reg(DMA_RX_CHANNEL_NUMBER);
        if (dma_uart.p_rx_active!=NULL)
        {
            KE_MSG_FREE(ke_msg2param(dma_uart.p_rx_active));
            dma_uart.p_rx_active = NULL;
        } 
    }       
}

void dma_uart_timeout(void)
{
    uint32_t rx_length;

    dma_uart_deassert_rts();

    // Wait for 5 character duration to avoid data loss
    for (int i=0;i< 5* uart_wait_byte_counter;i++);

    __disable_irq();
    if (dma_uart.p_rx_active == NULL ){
        __enable_irq();
        return;
    }
    dma_uart_rx_check_empty( &rx_length );

    dma_channel_stop(DMA_RX_CHANNEL_NUMBER);
    dma_clear_int_reg(DMA_RX_CHANNEL_NUMBER);

    /* Place the read buffer in ready list */
    dma_uart.p_rx_active->param_len = rx_length;

    co_list_push_back( &dma_uart.rx_list_ready, &dma_uart.p_rx_active->hdr);
    __enable_irq();
    dma_uart.p_rx_active = NULL;
    /* If there is no reason to flow of continue */
    if( !user_check_set_flow_off())
    {
        dma_uart_rx_activate();
        dma_uart_assert_rts();
    }
    dma_uart.rx_bytes += rx_length;
}

/**
 ****************************************************************************************
 * @brief Allocates a DMA Rx buffer
 * @return a pointer of UART_RX_STRUCT type
 ****************************************************************************************
 */
static struct UART_RX_STRUCT *dma_uart_allocate_rx_buffer(void)
{
    struct UART_RX_STRUCT *p_buff = KE_MSG_ALLOC_DYN_NO_INIT(DATA_TX_REQ,
                                                             prf_get_task_from_id(DEST_TASK_ID),
                                                             TASK_APP,
                                                             UART_RX_STRUCT,
                                                             ELEMENT_SIZE_RX);
    ASSERT_ERROR(p_buff);
    return p_buff;
};  

/**
 ****************************************************************************************
 * @brief Write to UART using DMA
 * @param[in] tx_data   pointer to the data to be written
 * @param[in] tx_length length of the data
 * @return void
 ****************************************************************************************
 */
static void dma_uart_write(uint32_t * tx_data, uint16_t tx_length)
{
    dma_uart.tx_bytes += tx_length;
    if(tx_length > 0)
    {
        uart_send(DSPS_UART, (const uint8_t *)tx_data, tx_length, UART_OP_DMA);
    }
}

/**
 ****************************************************************************************
 * @brief Initializes the DMA Uart Rx operation
 * @param[in] rx_data   address to store the received data
 * @param[in] rx_length length of the data
 * @return void
 ****************************************************************************************
 */
static void dma_uart_read(uint32_t * rx_data, uint16_t rx_length)
{
    if (rx_length > 0) 
        uart_receive(DSPS_UART, (uint8_t *)rx_data, rx_length, UART_OP_DMA);
}

void dma_uart_rx_activate(void)
{
    
    struct UART_RX_STRUCT *p_buff = dma_uart_allocate_rx_buffer();

    __disable_irq();
    dma_uart.p_rx_active  = ( struct ke_msg * )ke_param2msg(p_buff);
    __enable_irq();

    dma_uart_read((uint32_t *) (p_buff->data), EXPECTED_SIZE);
    dma_uart.rx_to_cnt = 0;
}

/**
 ****************************************************************************************
 * @brief The callback of DMA RX Interrupt
 * @param[in] user_data  address to the received data
 * @param[in] len        length of the data
 * @return void
 ****************************************************************************************
 */
static void dma_uart_rx_callback(void *user_data, uint16_t len)
{
    dma_uart_deassert_rts();
    dma_uart.nof_rx_callbacks++;

    __disable_irq();
    if (dma_uart.p_rx_active == NULL ){
        __enable_irq();
        return;
    }
    __enable_irq();

    dma_uart.rx_bytes += len;

    dma_uart.p_rx_active->param_len = len;

    __disable_irq();
    co_list_push_back( &dma_uart.rx_list_ready, &dma_uart.p_rx_active->hdr);
    __enable_irq();
    dma_uart.p_rx_active = NULL;

    rx_cb_done = true;
}

bool dma_uart_test_tx_queue_for_flow_on(void)
{
    if(dma_uart.ble_forced_flow_off == 1)
        return false;
    if ( (dma_uart.tx_list_ready.cnt < TX_FLOW_ON_LVL) && (dma_uart.ble_flow_off==1) )
    {   
        dma_uart.ble_flow_off = 0;
        user_send_ble_flow_ctrl(FLOW_ON);
        return true;
    }   
    return false;
}
/**
 ****************************************************************************************
 * @brief  Checks if the UART Tx should be deactivated by comparing tx_list_ready.cnt
 *               with TX_FLOW_OFF_LVL
 * @return true if the Rx of data from the BLE peer should be blocked, false otherwise
 ****************************************************************************************
 */
static bool dma_uart_test_tx_queue_for_flow_off(void)
{   
    if ( (dma_uart.tx_list_ready.cnt > TX_FLOW_OFF_LVL) && (dma_uart.ble_flow_off==0) )
    {   
        dma_uart.ble_flow_off = 1;
        user_send_ble_flow_ctrl(FLOW_OFF);
        return true;
    }   
    return false;
}

/**
 ****************************************************************************************
 * @brief  Forces DMA Uart Tx for Flow Off. 
 * @return  void
 ****************************************************************************************
 */
static void dma_uart_force_tx_flow_off(void)
{   
    if (dma_uart.ble_forced_flow_off==0)
    {   
        dma_uart.ble_forced_flow_off = 1;
        dma_uart.ble_flow_off=1;
        user_send_ble_flow_ctrl(FLOW_OFF);
    }
}

/**
 ****************************************************************************************
 * @brief  The UART DMA Tx Callback 
 * @param[in] user_data  address to the data for tx
 * @param[in] len        length of the data
 * @return void
 ****************************************************************************************
 */
static void dma_uart_tx_callback(void *user_data, uint16_t len)
{
    /* Free the tx'ed message */
    ASSERT_ERROR(dma_uart.p_tx_active);
    struct UART_TX_STRUCT *p_buff = ke_msg2param(dma_uart.p_tx_active);
    KE_MSG_FREE(p_buff);
    dma_uart.p_tx_active = NULL;

    dma_uart_test_tx_queue_for_flow_on();
    /*Check if buffers are pending for uart TX*/
    if( dma_uart.tx_list_ready.cnt > 0 ){ 
        if(dma_uart_test_tx_queue_for_flow_off())
        {
            return;
        } 
        __disable_irq();
        dma_uart.p_tx_active = ( struct ke_msg * )co_list_pop_front(&dma_uart.tx_list_ready);
        __enable_irq();
        p_buff = (struct UART_TX_STRUCT *)ke_msg2param(dma_uart.p_tx_active);
        dma_uart_write((uint32_t *) (p_buff->data), p_buff->length);
    }
}

/**
 ****************************************************************************************
 * @brief  The UART DMA CTS Callback, tied to a gpio interrupt
 * @return void
 ****************************************************************************************
 */ 
static void dma_uart_gpio_callback(void)
{
    NVIC_DisableIRQ(GPIO0_IRQn);    // Disable this interrupt
    
    if(GPIO_GetPinStatus(gpio_uart1_cts.port, gpio_uart1_cts.pin) == false)
    {
        dma_uart.ble_forced_flow_off = 0;
        dma_uart_test_tx_queue_for_flow_on();         
        //Set interrupt to detect rising edge of CTS
        GPIO_EnableIRQ(gpio_uart1_cts.port, gpio_uart1_cts.pin, GPIO0_IRQn, 0, 1, 0);
#if (WKUP_EXT_PROCESSOR)
        if ((gpio_wakeup_ext_host.port != GPIO_PORT_INVALID) && (gpio_wakeup_ext_host.pin != GPIO_PIN_INVALID))
        {
            if (!((gpio_wakeup_ext_host.port == gpio_uart1_rts.port) && (gpio_wakeup_ext_host.pin == gpio_uart1_rts.pin)))
            {
                GPIO_SetInactive( gpio_wakeup_ext_host.port, gpio_wakeup_ext_host.pin );
            }
        }
#endif
    }
    else if(GPIO_GetPinStatus(gpio_uart1_cts.port, gpio_uart1_cts.pin) == true)
    {
        dma_uart_force_tx_flow_off(); 
        //Set interrupt to detect falling edge of CTS
        GPIO_EnableIRQ(gpio_uart1_cts.port, gpio_uart1_cts.pin, GPIO0_IRQn, 1, 1, 0);
    }
    NVIC_ClearPendingIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO0_IRQn);     // Enable this interrupt
}


/**
 ****************************************************************************************
 * @brief  Callback to handle UART error. Does nothing
 * @param[in] uart_id           UART Id
 * @param[in] uart_err_status   UART error status
 * @return void
 ****************************************************************************************
 */
static void uart_user_no_err_expected_cb(uart_t *uart_id, uint8_t uart_err_status)
{
    //TEST_ASSERT_EQUAL_HEX16_MESSAGE(UART_ERR_NO_ERROR, uart_err_status, "Received interrupt error");
}

void dma_uart_sps_init(UART_BAUDRATE baudr, uint8_t mode)
{
    
    GPIO_RegisterCallback(GPIO0_IRQn, dma_uart_gpio_callback);
        
    if(GPIO_GetPinStatus(gpio_uart1_cts.port, gpio_uart1_cts.pin) == false)
    {
        dma_uart.ble_forced_flow_off = 0;
        dma_uart_test_tx_queue_for_flow_on();  
        GPIO_EnableIRQ(gpio_uart1_cts.port, gpio_uart1_cts.pin, GPIO0_IRQn, 0, 1, 0);
    }
    else if(GPIO_GetPinStatus(gpio_uart1_cts.port, gpio_uart1_cts.pin) == true)
    {
        dma_uart_force_tx_flow_off();
        GPIO_EnableIRQ(gpio_uart1_cts.port, gpio_uart1_cts.pin, GPIO0_IRQn, 1, 1, 0);
    }
    
    dma_uart_cfg.uart_err_cb = (uart_err_cb_t) uart_user_no_err_expected_cb;
    dma_uart_cfg.uart_tx_cb = (uart_cb_t) dma_uart_tx_callback;
    dma_uart_cfg.uart_rx_cb = (uart_cb_t) dma_uart_rx_callback;
    dma_uart_cfg.data_bits = UART_DATABITS_8;
    dma_uart_cfg.parity = UART_PARITY_NONE;
    dma_uart_cfg.stop_bits = UART_STOPBITS_1;
    dma_uart_cfg.use_fifo = UART_FIFO_EN;
    dma_uart_cfg.auto_flow_control=UART_AFCE_EN;
    dma_uart_cfg.uart_dma_priority = DMA_PRIO_7;
    dma_uart_cfg.baud_rate = baudr;
    dma_uart_cfg.uart_dma_channel=UART_DMA_CHANNEL_01;
    
    uart_initialize(DSPS_UART, &dma_uart_cfg);
    
}

bool dma_uart_check_active(void){
    if( (GetWord32(DMA0_CTRL_REG)&DMA_ON)==1)
        return true;

    if( (GetWord32(DMA1_CTRL_REG)&DMA_ON)==1)
        return true;

    return false;
}   

void dma_uart_on_connect(void){
    dma_uart.tx_size = DEFAULT_TX_SIZE;
    if( app_default_sleep_mode == ARCH_SLEEP_OFF ){
        dma_uart_sps_init(cfg_uart_sps_baudrate, 3);
        dma_uart_rx_activate();
        dma_uart_assert_rts();
    }   
}

void dma_uart_tx_async(void)
{
    __disable_irq();
    if( dma_uart.p_tx_active != NULL )
    {
        __enable_irq();         
            return;
    }
    if(GPIO_GetPinStatus(gpio_uart1_cts.port, gpio_uart1_cts.pin) == true)
    {
#if (WKUP_EXT_PROCESSOR)
        if( dma_uart.tx_list_ready.cnt !=0 ){
            if ((gpio_wakeup_ext_host.port != GPIO_PORT_INVALID) && (gpio_wakeup_ext_host.pin != GPIO_PIN_INVALID))
            {
                if (!((gpio_wakeup_ext_host.port == gpio_uart1_rts.port) && (gpio_wakeup_ext_host.pin == gpio_uart1_rts.pin)))
                {
                    GPIO_SetActive( gpio_wakeup_ext_host.port, gpio_wakeup_ext_host.pin );
                }
            }
        }
#endif
        __enable_irq();
        return;
    }

    if( dma_uart.tx_list_ready.cnt !=0 ){   
        dma_uart.p_tx_active = ( struct ke_msg * )co_list_pop_front(&dma_uart.tx_list_ready);
        struct UART_TX_STRUCT *p_buff = (struct UART_TX_STRUCT *)ke_msg2param(dma_uart.p_tx_active);
        dma_uart_write((uint32_t *) (p_buff->data), p_buff->length);
    }   
    __enable_irq();
    
    dma_uart_test_tx_queue_for_flow_off();
}

void dma_uart_set_tx_size(uint32_t mtu)
{
    dma_uart.mtu = mtu;
    dma_uart.tx_size = mtu-3;
}
/// @} UART 
