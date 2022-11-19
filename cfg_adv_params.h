#pragma once

#include "ble_gap.h"

/*
 * ADV0 (Normal Beaconing)
 *******************************************************************************
 */

// TODO: Set power to be something reasonable
#define CFG_ADV0_CREATE_MAX_TX_POWER 0

// At first, we don't want our beacon to be connectable
#define CFG_ADV0_CREATE_PROPERTY ADV_LEGACY_NON_CONN_SCAN_MASK

#define CFG_ADV0_DATA_ADV_PAYLOAD \
    /* Complete service list: 0x2af5 = fixed string 16 */ \
    0x03, 0x03, 0xf5, 0x2a, \
    /* Service Data */ \
    0x13, 0x2a, 0xf5, 0x2a, \
    /* First 6 bytes of SHA-1 Hash of school name (GUNN)*/ \
    0xb4, 0xb8, 0x85, 0xe7, 0x11, 0x79, \
    /* 10 byte student id (pad with 0xFF)*/ \
    0xFF, 0xFF, 9, 5, 0, 3, 0, 4, 8, 6
    
#define CFG_ADV0_DATA_SCANRSP_PAYLOAD \
    0x09,0xff,0x00,0x60,'A','T','M','B','L','E'
    
/*
 * ADV1 (Pairing Mode)
 *******************************************************************************
 */

#define CFG_ADV1_CREATE_PROPERTY ADV_LEGACY_UNDIR_CONN_MASK

// Same scan response as Adv0
#define CFG_ADV1_DATA_SCANRSP_PAYLOAD \
    CFG_ADV0_DATA_SCANRSP_PAYLOAD
