#pragma once

#include "ble_att.h"
#include "atm_gap.h"

#define CFG_GAP_DEV_NAME "LunchTrak Beacon"
#define CFG_GAP_APPEARANCE ATM_GAP_APPEARANCE_GENERIC_TAG


//<i> Attribute Database Configuration
// This is for configuring the write permissions for the attribute database
#define CFG_GAP_ATT_CFG BLE_GAP_ATT_PERIPH_PREF_CON_PAR_EN_MASK

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