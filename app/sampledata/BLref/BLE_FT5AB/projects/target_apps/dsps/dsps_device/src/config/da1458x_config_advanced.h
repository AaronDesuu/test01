/**
 ****************************************************************************************
 *
 * @file da1458x_config_advanced.h
 *
 * @brief Advanced compile configuration file.
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
 * @ingroup USER_CONFIGURATION
 * @{
 ****************************************************************************************
 */

#ifndef _DA1458X_CONFIG_ADVANCED_H_
#define _DA1458X_CONFIG_ADVANCED_H_

#include "da1458x_stack_config.h"

/****************************************************************************************************************/
/* Low Power clock selection.                                                                                   */
/*      -LP_CLK_XTAL32      External XTAL32 oscillator                                                          */
/*      -LP_CLK_RCX20       External internal RCX20 clock                                                       */
/*      -LP_CLK_FROM_OTP    Use the selection in the corresponding field of OTP Header                          */
/****************************************************************************************************************/
#if !defined (__DA14531__)
#define CFG_LP_CLK              LP_CLK_XTAL32
#else
#define CFG_LP_CLK              LP_CLK_RCX20
#endif

/****************************************************************************************************************/
/* If defined, Configuration Storage mechanism is enabled.                                                      */
/* CRC check is not used.                                                                                       */
/****************************************************************************************************************/
#define CFG_CONFIG_STORAGE
#undef USE_CONFIG_STORAGE_CRC

/****************************************************************************************************************/
/* If defined the application uses a hardcoded value for XTAL16M trimming. Should be disabled for devices       */
/* where XTAL16M is calibrated and trim value is stored in OTP.                                                 */
/* Important note. The hardcoded value is the average value of the trimming values giving the optimal results   */
/* for DA14585 DK devices. May not be applicable in other designs                                               */
/****************************************************************************************************************/
#if !defined (__DA14531__)
#undef CFG_USE_DEFAULT_XTAL16M_TRIM_VALUE_IF_NOT_CALIBRATED
#else
//#define CFG_USE_DEFAULT_XTAL32M_TRIM_VALUE_IF_NOT_CALIBRATED
#endif

/****************************************************************************************************************/
/* Periodic wakeup period to poll GTL iface. Time in msec.                                                      */
/****************************************************************************************************************/
#define CFG_MAX_SLEEP_DURATION_PERIODIC_WAKEUP_MS                  500  // 0.5s

/****************************************************************************************************************/
/* Periodic wakeup period if GTL iface is not enabled. Time in msec.                                            */
/****************************************************************************************************************/
#define CFG_MAX_SLEEP_DURATION_EXTERNAL_WAKEUP_MS                  10000  // 10s

/****************************************************************************************************************/
/* Wakeup from external processor running host application.                                                     */
/****************************************************************************************************************/
#define CFG_EXTERNAL_WAKEUP

/****************************************************************************************************************/
/* Wakeup external processor when a message is sent to GTL                                                      */
/****************************************************************************************************************/
#define CFG_WAKEUP_EXT_PROCESSOR

/****************************************************************************************************************/
/* Enables True Random number Generator. A random number is generated at system initialization and used to seed */
/* the C standard library random number generator.                                                              */
/****************************************************************************************************************/
#define CFG_TRNG (1024)
 
/****************************************************************************************************************/
/* NVDS configuration                                                                                           */
/* - CFG_NVDS_TAG_BD_ADDRESS            Default bdaddress. If bdaddress is written in OTP header this value is  */
/*                                      ignored                                                                 */
/* - CFG_NVDS_TAG_LPCLK_DRIFT           Low power clock drift. Permitted values in ppm are:                     */
/*      + DRIFT_20PPM                                                                                           */
/*      + DRIFT_30PPM                                                                                           */
/*      + DRIFT_50PPM                                                                                           */
/*      + DRIFT_75PPM                                                                                           */
/*      + DRIFT_100PPM                                                                                          */
/*      + DRIFT_150PPM                                                                                          */
/*      + DRIFT_250PPM                                                                                          */
/*      + DRIFT_500PPM                  Default value (500 ppm)                                                 */
/* - CFG_NVDS_TAG_BLE_CA_TIMER_DUR      Channel Assessment Timer duration (Multiple of 10ms)                    */
/* - CFG_NVDS_TAG_BLE_CRA_TIMER_DUR     Channel Reassessment Timer duration (Multiple of CA timer duration)     */
/* - CFG_NVDS_TAG_BLE_CA_MIN_RSSI       Minimal RSSI Threshold                                                  */
/* - CFG_NVDS_TAG_BLE_CA_NB_PKT         Number of packets to receive for statistics                             */
/* - CFG_NVDS_TAG_BLE_CA_NB_BAD_PKT     Number  of bad packets needed to remove a channel                       */
/****************************************************************************************************************/
#define CFG_NVDS_TAG_BD_ADDRESS             {0x02, 0x00, 0xF4, 0x35, 0x23, 0x48}

#define CFG_NVDS_TAG_LPCLK_DRIFT            DRIFT_500PPM
#define CFG_NVDS_TAG_BLE_CA_TIMER_DUR       2000
#define CFG_NVDS_TAG_BLE_CRA_TIMER_DUR      6
#define CFG_NVDS_TAG_BLE_CA_MIN_RSSI        0x40
#define CFG_NVDS_TAG_BLE_CA_NB_PKT          100
#define CFG_NVDS_TAG_BLE_CA_NB_BAD_PKT      50

/****************************************************************************************************************/
/* Enables the logging of heap memories usage. The feature can be used in development/debug mode.               */
/* Application must be executed in Keil debugger environment and "system_library.lib" must be replaced with     */
/* "system_library_with_heap_logging.lib" in project structure under sdk_arch. Developer must stop execution    */
/* and type disp_heaplog in debugger's command window. Heap memory statistics will be displayed on window       */
/****************************************************************************************************************/
#if defined (__DA14531__)
#define CFG_LOG_HEAP_USAGE
#else
#undef CFG_LOG_HEAP_USAGE
#if defined (CFG_LOG_HEAP_USAGE)
#  error "CFG_LOG_HEAP_USAGE is ONLY supported for DA14531"
# endif
#endif

/****************************************************************************************************************/
/* Enables BLE flow control mechanism based on heap usage                                                       */
/****************************************************************************************************************/
#if defined (__DA14531__)
#define CFG_BLE_FLOW_CONTROL
#else
#undef CFG_BLE_FLOW_CONTROL
#if defined (CFG_BLE_FLOW_CONTROL)
#  error "CFG_BLE_FLOW_CONTROL is ONLY supported for DA14531"
# endif
#endif

#ifdef CFG_BLE_FLOW_CONTROL
#define CFG_RF_DIAG_INT
#endif
/****************************************************************************************************************/
/* Enables the BLE statistics measurement feature.                                                              */
/****************************************************************************************************************/
#undef CFG_BLE_METRICS

/****************************************************************************************************************/
/* Output the Hardfault arguments to serial/UART interface.                                                     */
/****************************************************************************************************************/
#undef CFG_PRODUCTION_DEBUG_OUTPUT

/****************************************************************************************************************/
/* Maximum supported TX data packet length.                                                                     */
/* Range: 27 - 251                                                                                              */
/****************************************************************************************************************/
#ifdef USE_DLE
#define CFG_MAX_TX_PACKET_LENGTH        (251)
#else
#define CFG_MAX_TX_PACKET_LENGTH        (27)
#endif

/****************************************************************************************************************/
/* Maximum supported RX data packet length.                                                                     */
/* Range: 27 - 251                                                                                              */
/****************************************************************************************************************/
#ifdef USE_DLE
#define CFG_MAX_RX_PACKET_LENGTH        (251)
#else
#define CFG_MAX_RX_PACKET_LENGTH        (27)
#endif

/****************************************************************************************************************/
/* Select external application/host transport layer:                                                            */
/*     - 0 = GTL (auto)                                                                                         */
/*     - 1 = HCI (auto)                                                                                         */
/*     - 8 = GTL (fixed)                                                                                        */
/*     - 9 = HCI (fixed)                                                                                        */
/****************************************************************************************************************/
#define CFG_USE_H4TL                    (0)

/****************************************************************************************************************/
/* Duplicate filter max value for the scan report list. The maximum value shall be 100.                         */
/****************************************************************************************************************/
#define CFG_BLE_DUPLICATE_FILTER_MAX    (10)

/****************************************************************************************************************/
/* Duplicate filter flag for the scan report list. This flag controls what will be reported if the              */
/* CFG_BLE_DUPLICATE_FILTER_MAX number is exceeded.                                                             */
/*     - If the flag is defined, the extra devices are considered to be in the list and will not be reported.   */
/****************************************************************************************************************/
#undef CFG_BLE_DUPLICATE_FILTER_FOUND

/****************************************************************************************************************/
/* Resolving list maximum size.                                                                                 */
/****************************************************************************************************************/
#define CFG_LLM_RESOLVING_LIST_MAX      LLM_RESOLVING_LIST_MAX


/****************************************************************************************************************/
/* Maximum retention memory in bytes. The base address of the retention data is calculated from the selected    */
/* size.                                                                                                        */
/****************************************************************************************************************/
#define CFG_RET_DATA_SIZE    (1536)

/****************************************************************************************************************/
/* Maximum uninitialized retained data required by the application.                                             */
/****************************************************************************************************************/
#define CFG_RET_DATA_UNINIT_SIZE (0)

/****************************************************************************************************************/
/* The Keil scatter file may be provided by the user. If the user provides his own scatter file, the system has */
/* to be aware which RAM blocks has to retain. The 4th RAM block is always retained, since it contains the ROM  */
/* data.                                                                                                        */
/*     - CFG_RETAIN_RAM_1_BLOCK: if defined, the 1st RAM block must be retained.                                */
/*     - CFG_RETAIN_RAM_2_BLOCK: if defined, the 2nd RAM block must be retained.                                */
/*     - CFG_RETAIN_RAM_3_BLOCK: if defined, the 3rd RAM block must be retained.                                */
/*                                                                                                              */
/* If the CFG_CUSTOM_SCATTER_FILE flag is undefined, the system knows which blocks to retain based on the       */
/* default SDK scatter file.                                                                                    */
/****************************************************************************************************************/
#undef CFG_CUSTOM_SCATTER_FILE
#ifdef CFG_CUSTOM_SCATTER_FILE
    #define CFG_RETAIN_RAM_1_BLOCK
    #define CFG_RETAIN_RAM_2_BLOCK
    #define CFG_RETAIN_RAM_3_BLOCK
#endif

/****************************************************************************************************************/
/* Temperature range selection.                                                                                 */
/* - CFG_HIGH_TEMPERATURE:         Device is configured to operate at high temperature range (-40C to +105C).   */
/* - CFG_AMB_TEMPERATURE:          Device is configured to operate at ambient temperature range (-40C to +40C). */
/* - CFG_MID_TEMPERATURE:          Device is configured to operate at mid temperature range (-40C to +60C).     */
/* - CFG_EXT_TEMPERATURE:          Device is configured to operate at ext temperature range (-40C to +85C).     */
/* NOTE 1: High temperature support is not compatible with power optimizations. User shall undefine the         */
/*         CFG_POWER_OPTIMIZATIONS flag, if device is to support the high temperature range feature.            */
/****************************************************************************************************************/
#define CFG_AMB_TEMPERATURE


/****************************************************************************************************************/
/* Code location selection.                                                                                     */
/*     - CFG_CODE_LOCATION_EXT: Code is loaded from SPI flash / I2C EEPROM / UART                               */
/*     - CFG_CODE_LOCATION_OTP: Code is burned in the OTP                                                       */
/* The above options are mutually exclusive and exactly one of them must be enabled.                            */
/****************************************************************************************************************/
#define CFG_CODE_LOCATION_EXT
#undef CFG_CODE_LOCATION_OTP

/****************************************************************************************************************/
/* Uses long range extender (e.g. SKY66111).                                                                    */
/****************************************************************************************************************/
#undef CFG_RANGE_EXT

/****************************************************************************************************************/
/* Custom heap sizes                                                                                            */
/****************************************************************************************************************/

#if defined (__DA14531__)
#define DB_HEAP_SZ              (1024)
#if defined (CFG_BLE_FLOW_CONTROL)
#define MSG_HEAP_SZ             (5*1024)
#else
#define MSG_HEAP_SZ             (6*1024)
#endif
#else
#define DB_HEAP_SZ              (1024)
#define MSG_HEAP_SZ             (32*1024)
#endif

#if defined (__DA14531__)
#define CFG_DISABLE_QUADEC_ON_START_UP
#endif

#define __EXCLUDE_GPIO1_HANDLER__
#define __EXCLUDE_GPIO2_HANDLER__
#define __EXCLUDE_GPIO3_HANDLER__
#define __EXCLUDE_GPIO4_HANDLER__

#define CFG_POWER_MODE_BYPASS

#endif // _DA1458X_CONFIG_ADVANCED_H_

/// @} DSPS_DEVICE_CONFIG
