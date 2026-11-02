/**
 ****************************************************************************************
 *
 * @file user_periph_setup.h
 *
 * @brief Peripherals setup header file. 
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DSPS_DEVICE_PLATFORM    DSPS Device Platform
 * @addtogroup DSPS_DEVICE_PLATFORM
 * @ingroup USER_PLATFORM
 * @brief DSPS Device Platform handling
 * @{
 ****************************************************************************************
 */

#ifndef _USER_PERIPH_SETUP_H_
#define _USER_PERIPH_SETUP_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "uart.h"
#include "arch.h"
#include "da1458x_periph_setup.h"
#include "dma.h"
#include "dma_uart_sps.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#ifndef PRODUCT_HEADER_POSITION
#if defined (__DA14531__)
#define PRODUCT_HEADER_POSITION     0x1F000
#else
#define PRODUCT_HEADER_POSITION     0x39000
#endif
#endif

#define HW_CONFIG_PRO_DK 1
//*** <<< Use Configuration Wizard in Context Menu >>> ***

// <o> Baud Rate selection <1=> 9600 <2=> 19200 <3=> 38400 <4=> 57600 <5=> 115200 <6=> 230400 <7=> 460800 <8=> 921600
#define BAUDRATE_CONFIG 2
#define BAUDRATE_9K6 (BAUDRATE_CONFIG==1)
#define BAUDRATE_19K2 (BAUDRATE_CONFIG==2)
#define BAUDRATE_38K4 (BAUDRATE_CONFIG==3)
#define BAUDRATE_57K6 (BAUDRATE_CONFIG==4)
#define BAUDRATE_115K2 (BAUDRATE_CONFIG==5)
#define BAUDRATE_230K4 (BAUDRATE_CONFIG==6)
#define BAUDRATE_460K8 (BAUDRATE_CONFIG==7)
#define BAUDRATE_921K6 (BAUDRATE_CONFIG==8)


#if BAUDRATE_9K6
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_9600
#define CFG_UART_SPS_BAUD                UART_BAUD_9600
#endif

#if BAUDRATE_19K2
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_19200
#define CFG_UART_SPS_BAUD                UART_BAUD_19200
#endif

#if BAUDRATE_38K4
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_38400
#define CFG_UART_SPS_BAUD                UART_BAUD_38400
#endif

#if BAUDRATE_57K6
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_57600
#define CFG_UART_SPS_BAUD                UART_BAUD_57600
#endif

#if BAUDRATE_115K2
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_115200
#define CFG_UART_SPS_BAUD                UART_BAUD_115200
#endif

#if BAUDRATE_230K4
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_230400
#define CFG_UART_SPS_BAUD                UART_BAUD_230400
#endif

#if BAUDRATE_460K8
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_460800
#define CFG_UART_SPS_BAUD                UART_BAUD_460800
#endif

#if BAUDRATE_921K6
#define CFG_UART_SPS_BAUDRATE            UART_BAUDRATE_921600
#define CFG_UART_SPS_BAUD                UART_BAUD_921600
#endif

/// UART Baud enumeration
typedef enum {
    /// 1M Baud Rate
    UART_BAUD_1000000   = 1000000,

    /// 921600 Baud Rate
    UART_BAUD_921600    = 921600,

    /// 500000 Baud Rate
    UART_BAUD_500000    = 500000,

    /// 460800 Baud Rate
    UART_BAUD_460800    = 460800,

    /// 230400 Baud Rate
    UART_BAUD_230400    = 230400,

    /// 115200 Baud Rate
    UART_BAUD_115200    = 115200,

    /// 57600 Baud Rate
    UART_BAUD_57600     = 57600,

    /// 38400 Baud Rate
    UART_BAUD_38400     = 38400,

    /// 28800 Baud Rate
    UART_BAUD_28800     = 28800,

    /// 19200 Baud Rate
    UART_BAUD_19200     = 19200,

    /// 14400 Baud Rate
    UART_BAUD_14400     = 14400,

    /// 9600 Baud Rate
    UART_BAUD_9600      = 9600,

    /// 4800 Baud Rate
    UART_BAUD_4800      = 4800,

    /// 2400 Baud Rate
    UART_BAUD_2400      = 2400,
} UART_BAUD;

/// UART Baudrate structure
static const UART_BAUDRATE uart_baudrate_t[9] = {
    UART_BAUDRATE_2400, //NOT USED
    UART_BAUDRATE_9600,
    UART_BAUDRATE_19200,
    UART_BAUDRATE_38400,
    UART_BAUDRATE_57600,
    UART_BAUDRATE_115200,
    UART_BAUDRATE_230400,
    UART_BAUDRATE_460800,
    UART_BAUDRATE_921600
};

/// UART Baud structure
static const UART_BAUD uart_baud_t[9] = {
    UART_BAUD_2400, //NOT USED
    UART_BAUD_9600,
    UART_BAUD_19200,
    UART_BAUD_38400,
    UART_BAUD_57600,
    UART_BAUD_115200,
    UART_BAUD_230400,
    UART_BAUD_460800,
    UART_BAUD_921600
};

extern UART_BAUDRATE cfg_uart_sps_baudrate;
extern UART_BAUD cfg_uart_sps_baud;
extern uint32_t uart_wait_byte_time;
extern uint32_t uart_wait_byte_counter;

/* Enable WKUPCT. Required by wkupct_quadec driver. */
#define WKUP_ENABLED
#define nAUTO_RTS

#define UART_WAIT_BYTE_TIME             (CFG_UART_SPS_BAUDRATE * 10)        // time in uSec
#define UART_WAIT_BYTE_COUNTER          (UART_WAIT_BYTE_TIME * 4) + 30      //convert in for loop counter 

/// GPIO type
typedef struct gpio_func
{
    /// GPIO Pin
    GPIO_PIN                    pin;
    /// GPIO Port
    GPIO_PORT                   port;
} gpio_func_t;

extern gpio_func_t gpio_uart1_tx;
extern gpio_func_t gpio_uart1_rx;
extern gpio_func_t gpio_uart1_rts;
extern gpio_func_t gpio_uart1_cts;
#if (EXTERNAL_WAKEUP)
extern gpio_func_t gpio_ext_wakeup;
extern uint8_t gpio_ext_wakeup_polarity;
extern uint16_t gpio_ext_wakeup_debounce;
#endif
extern gpio_func_t gpio_reset_status;
#if WKUP_EXT_PROCESSOR
extern gpio_func_t gpio_wakeup_ext_host;
#endif
extern gpio_func_t gpio_por_pin;
extern uint8_t gpio_por_pin_polarity;
extern uint8_t gpio_por_pin_timeout;

/****************************************************************************************/ 
/* UART configuration                                                                   */
/****************************************************************************************/  
#if defined (__DA14531__)    
#define GPIO_UART1_TX_PORT   GPIO_PORT_0
#define GPIO_UART1_TX_PIN    GPIO_PIN_9
#define GPIO_UART1_RX_PORT   GPIO_PORT_0
#define GPIO_UART1_RX_PIN    GPIO_PIN_8

#define GPIO_UART1_RTS_PORT  GPIO_PORT_0
#define GPIO_UART1_RTS_PIN   GPIO_PIN_7
#define GPIO_UART1_CTS_PORT  GPIO_PORT_0
#define GPIO_UART1_CTS_PIN   GPIO_PIN_6
#else
#define GPIO_UART1_TX_PORT   GPIO_PORT_0
#define GPIO_UART1_TX_PIN    GPIO_PIN_4
#define GPIO_UART1_RX_PORT   GPIO_PORT_0
#define GPIO_UART1_RX_PIN    GPIO_PIN_2
#define GPIO_UART1_RTS_PORT  GPIO_PORT_0
#define GPIO_UART1_RTS_PIN   GPIO_PIN_7
#define GPIO_UART1_CTS_PORT  GPIO_PORT_0
#define GPIO_UART1_CTS_PIN   GPIO_PIN_6
#endif

/****************************************************************************************/
/* Reset pin configuration                                                              */
/****************************************************************************************/
#if defined (__DA14531__)
#define GPIO_ACTIVE_STATUS_PORT     GPIO_PORT_0
#define GPIO_ACTIVE_STATUS_PIN      GPIO_PIN_11

#define GPIO_POR_PORT               GPIO_PORT_0
#define GPIO_POR_PIN                GPIO_PIN_5
#else
#define GPIO_ACTIVE_STATUS_PORT     GPIO_PORT_2
#define GPIO_ACTIVE_STATUS_PIN      GPIO_PIN_5

#define GPIO_POR_PORT               GPIO_PORT_2
#define GPIO_POR_PIN                GPIO_PIN_9
#endif

#if (EXTERNAL_WAKEUP)
#define EXTERNAL_WAKEUP_GPIO_PORT   GPIO_PORT_INVALID
#define EXTERNAL_WAKEUP_GPIO_PIN    GPIO_PIN_INVALID
#endif
#if (WKUP_EXT_PROCESSOR)
#define WKUP_EXT_PROC_GPIO_PORT     GPIO_PORT_INVALID
#define WKUP_EXT_PROC_GPIO_PIN      GPIO_PIN_INVALID
#endif
/****************************************************************************************/ 
/* UART2 GPIO configuration                                                             */
/****************************************************************************************/ 
 
#ifdef CFG_PRINTF_UART2
    #define  GPIO_UART2_TX_PORT     GPIO_PORT_0
    #define  GPIO_UART2_TX_PIN      GPIO_PIN_1
    
    #define  GPIO_UART2_RX_PORT     GPIO_PORT_0
    #define  GPIO_UART2_RX_PIN      GPIO_PIN_2
#endif

#define GPIO_DBG1_PORT   GPIO_PORT_1
#define GPIO_DBG1_PIN    GPIO_PIN_1
#define GPIO_DBG2_PORT   GPIO_PORT_1
#define GPIO_DBG2_PIN    GPIO_PIN_0

/****************************************************************************************/
/* SPI configuration                                                                    */
/****************************************************************************************/
// Define SPI Pads
#if defined (__DA14531__)
    #define SPI_EN_PORT             GPIO_PORT_0
    #define SPI_EN_PIN              GPIO_PIN_1

    #define SPI_CLK_PORT            GPIO_PORT_0
    #define SPI_CLK_PIN             GPIO_PIN_4

    #define SPI_DO_PORT             GPIO_PORT_0
    #define SPI_DO_PIN              GPIO_PIN_0

    #define SPI_DI_PORT             GPIO_PORT_0
    #define SPI_DI_PIN              GPIO_PIN_3

#elif !defined (__DA14586__)
    #define SPI_EN_PORT             GPIO_PORT_0
    #define SPI_EN_PIN              GPIO_PIN_3

    #define SPI_CLK_PORT            GPIO_PORT_0
    #define SPI_CLK_PIN             GPIO_PIN_0

    #define SPI_DO_PORT             GPIO_PORT_0
    #define SPI_DO_PIN              GPIO_PIN_6

    #define SPI_DI_PORT             GPIO_PORT_0
    #define SPI_DI_PIN              GPIO_PIN_5
#endif

// Define SPI Configuration
    #define SPI_MS_MODE             SPI_MS_MODE_MASTER
    #define SPI_CP_MODE             SPI_CP_MODE_0
    #define SPI_WSZ                 SPI_MODE_8BIT
    #define SPI_CS                  SPI_CS_0

#if defined(__DA14531__)
    #define SPI_SPEED_MODE          SPI_SPEED_MODE_4MHz
//    #define SPI_SPEED_MODE          SPI_SPEED_MODE_2MHz

    #define SPI_EDGE_CAPTURE        SPI_MASTER_EDGE_CAPTURE
#else // (DA14585, DA14586)
    #define SPI_SPEED_MODE          SPI_SPEED_MODE_4MHz
#endif

#define SPI_FLASH_GPIO_MAP   ((uint32_t)SPI_CLK_PIN | ((uint32_t)SPI_CLK_PORT << 4) | \
                        ((uint32_t)SPI_EN_PIN << 8) | ((uint32_t)SPI_EN_PORT << 12) | \
                        ((uint32_t)SPI_DO_PIN << 16) | ((uint32_t)SPI_DO_PORT << 20) | \
                        ((uint32_t)SPI_DI_PIN << 24) | ((uint32_t)SPI_DI_PORT << 28))

/****************************************************************************************/
/* SPI Flash configuration                                                              */
/****************************************************************************************/
#if !defined (__DA14586__)
#define SPI_FLASH_DEV_SIZE          (256 * 1024)
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable pad and peripheral clocks assuming that peripheral power domain
 *        is down. The UART and SPI clocks are set.
 * @return void
 ****************************************************************************************
 */
void periph_init(void);

/**
 ****************************************************************************************
 * @brief Each application reserves its own GPIOs here.
 * @return void
 ****************************************************************************************
 */
void GPIO_reservations(void);

/**
 ****************************************************************************************
 * @brief Map port pins. The UART and SPI port pins and GPIO ports are mapped.
 * @return void
 ****************************************************************************************
 */
void set_pad_functions(void);

#if (!SUOTAR_SPI_DISABLE) || defined CFG_CONFIG_STORAGE
/**
 ****************************************************************************************
 * @brief Initializes the SPI to access the flash.
 * @param[in] gpio_map  Bitmap of the GPIOs to be used
 * @return void
 ****************************************************************************************
 */
void user_spi_flash_init(uint32_t gpio_map);

/**
 ****************************************************************************************
 * @brief Powers down the flash and releases the SPI controller.
 * @return void
 ****************************************************************************************
 */
void user_spi_flash_release(void);

/**
 ****************************************************************************************
 * @brief Erases sectors of the flash. This functions can erase whole sectors, not part of it.
 * @param[in] starting_address  The address in the flash where we want to erase.
 * @param[in] size              Size of data we want to erase.
 * @return 0 if success, else error code.
 ****************************************************************************************
 */
int8_t user_erase_flash_sectors(uint32_t starting_address, uint32_t size);

/**
 ****************************************************************************************
 * @brief Writes data to flash.
 * @param[in] data      Pointer to the data to be written.
 * @param[in] address   Address in the flash.
 * @param[in] size      Size of data we want to write.
 * @return 0 if success, else error code.
 ****************************************************************************************
 */
int32_t user_flash_write_data (uint8_t *data, uint32_t address, uint32_t size);
#endif

#endif // _USER_PERIPH_SETUP_H_

/// @} DSPS_DEVICE_PLATFORM
