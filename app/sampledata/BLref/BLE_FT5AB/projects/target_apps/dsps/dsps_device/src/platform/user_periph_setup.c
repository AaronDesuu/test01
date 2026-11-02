/**
 ****************************************************************************************
 *
 * @file user_periph_setup.c
 *
 * @brief Peripherals setup and initialization.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_DEVICE_PLATFORM
 * @brief DSPS Device peripherals
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "user_periph_setup.h"            // peripheral configuration
#include "gpio.h"
#include "uart.h"                    // UART initialization
#include "user_sps_buffer_dma.h"
#include "dma_uart_sps.h"
#if defined (__DA14531__)
#include "spi_531.h"
#endif
#include "spi_flash.h"

#if defined (CFG_SPI_FLASH_ENABLE)
// Configuration struct for SPI
static const spi_cfg_t spi_cfg = {
    .spi_ms = SPI_MS_MODE,
    .spi_cp = SPI_CP_MODE,
    .spi_speed = SPI_SPEED_MODE,
    .spi_wsz = SPI_WSZ,
    .spi_cs = SPI_CS,
    .cs_pad.port = SPI_EN_PORT,
    .cs_pad.pin = SPI_EN_PIN,
#if defined (__DA14531__)
    .spi_capture = SPI_EDGE_CAPTURE,
#endif
};

static const spi_flash_cfg_t spi_flash_cfg = {
    .chip_size = SPI_FLASH_DEV_SIZE,
};
#endif

UART_BAUDRATE cfg_uart_sps_baudrate;
UART_BAUD cfg_uart_sps_baud;
uint32_t uart_wait_byte_time;
uint32_t uart_wait_byte_counter;
extern struct dma_uart_tag dma_uart;

#if defined (__DA14531__)
    #if defined (CFG_BLE_FLOW_CONTROL)
    void patch_func(void);
    #endif
#else
void patch_func(void);
#endif

#if DEVELOPMENT_DEBUG
void GPIO_reservations(void)
{
/*
* Globally reserved GPIOs reservation
*/

/*
* Application specific GPIOs reservation. Used only in Development mode (#if DEVELOPMENT_DEBUG)

i.e.
    RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);    //Reserve P_01 as Generic Purpose I/O
*/
#if (EXTERNAL_WAKEUP)
    RESERVE_GPIO( EXTERNAL_WAKEUP, gpio_ext_wakeup.port,  gpio_ext_wakeup.pin, PID_GPIO);
#endif
#if (WKUP_EXT_PROCESSOR)
    RESERVE_GPIO( WKUP_EXT_PROCESSOR, gpio_wakeup_ext_host.port,  gpio_wakeup_ext_host.pin, PID_GPIO);
#endif
#if (BLE_SPS_SERVER)
    // UART1
    RESERVE_GPIO( UART1_TX, gpio_uart1_tx.port,  gpio_uart1_tx.pin, PID_UART1_TX);
    RESERVE_GPIO( UART1_RX, gpio_uart1_rx.port,  gpio_uart1_rx.pin, PID_UART1_RX);
    RESERVE_GPIO( UART1_RTS, gpio_uart1_rts.port,  gpio_uart1_rts.pin, PID_GPIO);
    RESERVE_GPIO( UART1_CTS, gpio_uart1_cts.port,  gpio_uart1_cts.pin, PID_UART1_CTSN);
#endif
#ifdef __DA14531__
    RESERVE_GPIO( ACTIVE_STATUS, gpio_reset_status.port,  gpio_reset_status.pin, PID_GPIO);
#endif

#ifdef CFG_PRINTF_UART2
    RESERVE_GPIO( UART2_TX, GPIO_UART2_TX_PORT,  GPIO_UART2_TX_PIN, PID_UART2_TX);
    RESERVE_GPIO( UART2_RX, GPIO_UART2_RX_PORT,  GPIO_UART2_RX_PIN, PID_UART2_RX);
#endif

}
#endif //DEVELOPMENT_DEBUG

void set_pad_functions(void)        // set gpio port function mode
{

    #ifdef __DA14531__
    SetBits16(HWR_CTRL_REG, DISABLE_HWR, 1);

    // GP ADC Fix
    SetBits16(GP_ADC_CTRL_REG, GP_ADC_MUTE, 1);
    #endif
    #if (EXTERNAL_WAKEUP)
    if ((gpio_ext_wakeup.port != GPIO_PORT_INVALID) && (gpio_ext_wakeup.pin != GPIO_PIN_INVALID))
        GPIO_ConfigurePin(gpio_ext_wakeup.port, gpio_ext_wakeup.pin, INPUT_PULLUP, PID_GPIO, false );
    #endif
    #if (WKUP_EXT_PROCESSOR)
    if ((gpio_wakeup_ext_host.port != GPIO_PORT_INVALID) && (gpio_wakeup_ext_host.pin != GPIO_PIN_INVALID))
        GPIO_ConfigurePin(gpio_wakeup_ext_host.port, gpio_wakeup_ext_host.pin, OUTPUT, PID_GPIO, false);
    #endif

    GPIO_ConfigurePin( gpio_uart1_tx.port, gpio_uart1_tx.pin, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( gpio_uart1_rx.port, gpio_uart1_rx.pin, INPUT_PULLUP, PID_UART1_RX, false );
    GPIO_ConfigurePin( gpio_uart1_rts.port, gpio_uart1_rts.pin, OUTPUT, PID_GPIO, true );
    GPIO_ConfigurePin( gpio_uart1_cts.port, gpio_uart1_cts.pin, INPUT_PULLUP, PID_UART1_CTSN, false );
#if defined (__DA14531__)
    if ((gpio_reset_status.port != GPIO_PORT_INVALID) && (gpio_reset_status.pin != GPIO_PIN_INVALID))
        GPIO_ConfigurePin( gpio_reset_status.port, gpio_reset_status.pin, INPUT_PULLUP, PID_GPIO, true );
#endif
    if ((gpio_por_pin.port != GPIO_PORT_INVALID) && (gpio_por_pin.pin != GPIO_PIN_INVALID))
        GPIO_EnablePorPin(gpio_por_pin.port, gpio_por_pin.pin, (GPIO_POR_PIN_POLARITY)gpio_por_pin_polarity, gpio_por_pin_timeout);

#ifdef CFG_PRINTF_UART2
    GPIO_ConfigurePin( GPIO_UART2_TX_PORT, GPIO_UART2_TX_PIN, OUTPUT, PID_UART2_TX, false );
    GPIO_ConfigurePin( GPIO_UART2_RX_PORT, GPIO_UART2_RX_PIN, INPUT, PID_UART2_RX, false );
#endif
}

void periph_init(void)
{
    // Power up peripherals' power domain
#if !defined (__DA14531__)
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ;

    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);
#endif
    //rom patch

#if defined (__DA14531__)
    #if defined (CFG_BLE_FLOW_CONTROL)
    patch_func();
    #endif
#else
    patch_func();
#endif

    //Init pads
    set_pad_functions();

    // Initialize UART2 controller
#ifdef CFG_PRINTF_UART2
    uart_cfg_t uart_cfg = {
        .baud_rate = UART_BAUDRATE_115200,
        .data_bits = UART_DATABITS_8,
        .parity = UART_PARITY_NONE,
        .stop_bits = UART_STOPBITS_1,
        .auto_flow_control = UART_AFCE_DIS,
        .use_fifo = UART_FIFO_EN,
        .tx_fifo_tr_lvl = UART_TX_FIFO_LEVEL_0,
        .rx_fifo_tr_lvl = UART_RX_FIFO_LEVEL_0,
        .intr_priority = 0,
    };
    uart_initialize(UART2, &uart_cfg);
#endif

#if !defined (__DA14531__)
   // Enable the pads
    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
#else
    // Enable the pads
    SetBits16(PAD_LATCH_REG, PAD_LATCH_EN, 1);
#endif

}

#if (!SUOTAR_SPI_DISABLE) || defined CFG_CONFIG_STORAGE
void user_spi_flash_init(uint32_t gpio_map)
{
    spi_gpio_config_t spi_conf;
    uint8_t dev_id;

    spi_conf.clk.port      = ((GPIO_PORT)((gpio_map & 0x000000f0) >>  4));
    spi_conf.clk.pin       = ((GPIO_PIN) ((gpio_map & 0x0000000f)));
    spi_conf.cs.port       = ((GPIO_PORT)((gpio_map & 0x0000f000) >> 12));
    spi_conf.cs.pin        = ((GPIO_PIN) ((gpio_map & 0x00000f00) >> 8));
    spi_conf.mosi.port     = ((GPIO_PORT)((gpio_map & 0x00f00000) >> 20));
    spi_conf.mosi.pin      = ((GPIO_PIN) ((gpio_map & 0x000f0000) >> 16));
    spi_conf.miso.port     = ((GPIO_PORT)((gpio_map & 0xf0000000) >> 28));
    spi_conf.miso.pin      = ((GPIO_PIN) ((gpio_map & 0x0f000000) >> 24));

#if DEVELOPMENT_DEBUG
    RESERVE_GPIO( SPI_CLK, spi_conf.clk.port, spi_conf.clk.pin, PID_SPI_CLK);
    RESERVE_GPIO( SPI_DO, spi_conf.mosi.port, spi_conf.mosi.pin, PID_SPI_DO);
    RESERVE_GPIO( SPI_DI, spi_conf.miso.port, spi_conf.miso.pin, PID_SPI_DI);
    RESERVE_GPIO( SPI_EN, spi_conf.cs.port, spi_conf.cs.pin, PID_SPI_EN);
#endif

    GPIO_ConfigurePin( spi_conf.cs.port, spi_conf.cs.pin, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( spi_conf.clk.port, spi_conf.clk.pin, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( spi_conf.mosi.port, spi_conf.mosi.pin, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( spi_conf.miso.port, spi_conf.miso.pin, INPUT, PID_SPI_DI, false );

#if defined (CFG_SPI_FLASH_ENABLE)
    // Configure SPI Flash environment
    spi_flash_configure_env(&spi_flash_cfg);

    // Initialize SPI
    spi_initialize(&spi_cfg);
#endif

    // Release Flash from power down
    spi_flash_release_from_power_down();

    // Try to auto-detect the device
    spi_flash_auto_detect(&dev_id);

}

void user_spi_flash_release(void)
{
    // Power down flash
    spi_flash_power_down();
    // Release SPI controller
    spi_release();
}

int8_t user_erase_flash_sectors(uint32_t starting_address, uint32_t size)
{
    int i;
    uint32_t starting_sector;
    uint32_t sector;
    int8_t ret = -1;

    starting_sector = (starting_address/SPI_FLASH_SECTOR_SIZE) * SPI_FLASH_SECTOR_SIZE;

    sector=(size / SPI_FLASH_SECTOR_SIZE);
    if (size % SPI_FLASH_SECTOR_SIZE)
    {
        sector++;
    }
    if (!sector)
    {
        sector++;
    }
    for (i=0; i < sector; i++)
    {
        ret = spi_flash_block_erase(starting_sector, SPI_FLASH_OP_SE);
        starting_sector += SPI_FLASH_SECTOR_SIZE;
        if (ret != SPI_FLASH_ERR_OK)
        {
            break;
        }
    }
    return ret;
}

int32_t user_flash_write_data (uint8_t *data, uint32_t address, uint32_t size)
{
    uint32_t actual_size;

    uint32_t upper_limit = address + size;
    uint32_t starting_sector = (address/SPI_FLASH_SECTOR_SIZE) * SPI_FLASH_SECTOR_SIZE;
    uint32_t next_sector_start;

    if (starting_sector == address)
    {
        // We start at a place where new sector starts - erase from here.
        user_erase_flash_sectors(address, size);
    } else if ((upper_limit/SPI_FLASH_SECTOR_SIZE) != (address/SPI_FLASH_SECTOR_SIZE)) {
        // We start in the middle of already erased sector - start erase from next one.
        next_sector_start = starting_sector + SPI_FLASH_SECTOR_SIZE;

        user_erase_flash_sectors(next_sector_start, upper_limit - next_sector_start);
    }

    return spi_flash_write_data(data, address, size, &actual_size);
}

#endif

/// @} DSPS_DEVICE_PLATFORM
