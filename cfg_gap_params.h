#pragma once

#include "ble_att.h"
#include "atm_gap.h"

#define CFG_GAP_DEV_NAME "Atmosic Beacon"
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