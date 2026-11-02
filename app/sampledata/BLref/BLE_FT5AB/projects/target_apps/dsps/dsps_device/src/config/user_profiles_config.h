/**
 ****************************************************************************************
 *
 * @file user_profiles_config.h
 *
 * @brief Configuration file for the profiles used in the application. 
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

#ifndef _USER_PROFILES_CONFIG_H_
#define _USER_PROFILES_CONFIG_H_

/*
 * DEFINITIONS
 ****************************************************************************************
 */

/***************************************************************************************/
/* Used BLE profiles (used by "rwprf_config.h").                                       */
/***************************************************************************************/
#define CFG_PRF_SPSS

//#define CFG_PRF_REMOTE_CONFIG

//#define CFG_PRF_SUOTAR

#define CFG_PRF_DISS
/*
 * PROFILE CONFIGURATION
 ****************************************************************************************
 */

/// Add profile specific configurations

/* SUOTAR configurations
 * -----------------------------------------------------------------------------------------
 */
 
#define SUOTAR_PATCH_AREA   1   // Placed in the RetRAM when SUOTAR_PATCH_AREA is 0 and in SYSRAM when 1

#ifdef  CFG_PRF_DISS
/*
 ****************************************************************************************
 * DISS application profile configuration
 ****************************************************************************************
 */

#define APP_DIS_FEATURES                (DIS_MANUFACTURER_NAME_CHAR_SUP | \
                                        DIS_MODEL_NB_STR_CHAR_SUP | \
                                        DIS_SYSTEM_ID_CHAR_SUP | \
                                        DIS_SW_REV_STR_CHAR_SUP | \
                                        DIS_FIRM_REV_STR_CHAR_SUP | \
                                        DIS_PNP_ID_CHAR_SUP)

/// Manufacturer Name (up to 18 chars)
//#define APP_DIS_MANUFACTURER_NAME     ("Dialog Semi")
#define APP_DIS_MANUFACTURER_NAME       ("Fuji Electric")
#define APP_DIS_MANUFACTURER_NAME_LEN   (13)

/// Model Number String (up to 18 chars). Either "DA14531" or "DA14585".
#if defined (__DA14531__)
//#define APP_DIS_MODEL_NB_STR            ("DA14531")
  #define APP_DIS_MODEL_NB_STR            ("FT5AB")
#else
#define APP_DIS_MODEL_NB_STR            ("DA14585")
#endif

#define APP_DIS_MODEL_NB_STR_LEN        (5)

/// System ID - LSB -> MSB (FIXME)
#define APP_DIS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define APP_DIS_SYSTEM_ID_LEN           (8)

/// Serial Number
#define APP_DIS_SERIAL_NB_STR           ("1.0.0.0-LE")
#define APP_DIS_SERIAL_NB_STR_LEN       (10)

/// Hardware Revision String. Either "DA14531" or "DA14585".
#if defined (__DA14531__)
//#define APP_DIS_HARD_REV_STR            ("DA14531")
  #define APP_DIS_HARD_REV_STR            ("F5NWF01")
#else
#define APP_DIS_HARD_REV_STR            ("DA14585")
#endif
#define APP_DIS_HARD_REV_STR_LEN        (7)

/// Firmware Revision
#define APP_DIS_FIRM_REV_STR            SDK_VERSION
#define APP_DIS_FIRM_REV_STR_LEN        (sizeof(APP_DIS_FIRM_REV_STR) - 1)

/// Software Revision String
#define APP_DIS_SW_REV_STR              SW_VERSION
#define APP_DIS_SW_REV_STR_LEN          (sizeof(APP_DIS_SW_REV_STR) - 1)

/// IEEE
#define APP_DIS_IEEE                    ("\xFF\xEE\xDD\xCC\xBB\xAA")
#define APP_DIS_IEEE_LEN                (6)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementers Forum assigned Vendor ID value)
 *      Vendor ID : 0x045E      (Microsoft Corp)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 * e.g. define APP_DIS_PNP_ID          ("\x02\x5E\x04\x40\x00\x00\x03")
 */
#define APP_DIS_PNP_ID                  ("\x01\xD2\x00\x80\x05\x00\x01")
#define APP_DIS_PNP_ID_LEN              (7)
#endif
/*
 ****************************************************************************************
 * BASS application profile configuration
 ****************************************************************************************
 */

// Measured in timer units (10ms)
#define APP_BASS_POLL_INTERVAL           (6000)  //  (6000*10ms)/60sec = Every 1 minute

#endif // _USER_PROFILES_CONFIG_H_

/// @} DSPS_DEVICE_CONFIG
