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
 #include "atm_gap.h"

#define CFG_GAP_DEV_NAME "Atmosic Lunch Beacon"
#define CFG_GAP_APPEARANCE ATM_GAP_APPEARANCE_GENERIC_TAG

//<i> Attribute Database Configuration
// CFG_ENABLE_SVC_CHG is for the "unlock attribute" thingy
// This is for configuring the write permissions for the attribute database
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

/*
 * MY STUFF
 *******************************************************************************
 */

#define CFG_ADV0_CREATE_MAX_TX_POWER 0

#define CFG_ADV0_DATA_ADV_PAYLOAD \
    0x0c,0x09,'L','u','n','c','h','-','L','i','n','e','s'

#define CFG_ADV0_DATA_SCANRSP_PAYLOAD \
    0x09,0xff,0x00,0x60,'A','T','M','B','L','E'
    