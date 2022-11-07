/**
 *******************************************************************************
 *
 * @file param_gap.h
 *
 * @brief gap configuration
 *
 * Copyright (C) Atmosic 2020-2022
 *
 *******************************************************************************
 */
 #pragma once
 #include "ble_att.h"
//***<<< Use Configuration Wizard in Context Menu >>>***

//<s.17> Generic Access Device Name
//<i> Generic Access Device Name
#define CFG_GAP_DEV_NAME "Atmosic ATT Server"

//<o.0..15> Generic Access Appearance
//<0-0xFFFF>
//<i> Generic Access Appearance (uuid: 0x2A01)
#define CFG_GAP_APPEARANCE 0x0240 // Generic Keyring

//<o.0..15> Attribute Database Configuration
//<0-0xFFFF>
//<i> Attribute Database Configuration
#ifdef CFG_ENABLE_SVC_CHG
#define CFG_GAP_ATT_CFG \
    (BLE_GAP_ATT_PERIPH_PREF_CON_PAR_EN_MASK | BLE_GAP_ATT_SVC_CHG_EN_MASK)
#else
#define CFG_GAP_ATT_CFG BLE_GAP_ATT_PERIPH_PREF_CON_PAR_EN_MASK
#endif

// <o> Security Propery
// <i> Security Propery
//   <0=> No Security
//   <1=> UnAuth
#define SEC_PROP 0

#if (SEC_PROP == 0)
#define ATTS_SVC_SEC_PROPERTY BLE_SEC_PROP_NO_SECURITY
#elif (SEC_PROP == 1)
#define ATTS_SVC_SEC_PROPERTY BLE_SEC_PROP_UNAUTH
#endif

//***<<< end of configuration section >>>***

