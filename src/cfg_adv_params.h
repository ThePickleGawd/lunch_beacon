#pragma once

#include "ble_gap.h"

/*
 * ADV0 (Normal Beaconing)
 *******************************************************************************
 */

// Units in dbm (ex. 0x04 = 4dbm or 0xFF = -1dbm)
// For negative number 0xFF-val+1
// 0xEC --> -20dbm
#define CFG_ADV0_CREATE_MAX_TX_POWER 0x00
#define CFG_ADV0_CREATE_PROPERTY ADV_LEGACY_NON_CONN_SCAN_MASK
#define CFG_ADV0_START_DURATION 30000 // 300s (unit of 10ms) TODO: change

#define ADV0_INTERVAL 100 // ms
#define CFG_ADV0_CREATE_INTERVAL_MIN ((uint32_t)ADV0_INTERVAL*1000/625)
#define CFG_ADV0_CREATE_INTERVAL_MAX ((uint32_t)ADV0_INTERVAL*1000/625)

#define CFG_ADV0_DATA_ADV_PAYLOAD \
    /* Complete service list: 0x2af5 = fixed string 16 */ \
    0x03, 0x03, 0xf5, 0x2a, \
    /* Service Data */ \
    0x13, 0x2a, 0xf5, 0x2a, \
    /* First 6 bytes of SHA-1 Hash of school name (GUNN)*/ \
    0xb4, 0xb8, 0x85, 0xe7, 0x11, 0x79, \
    /* 10 byte Student ID in ascii (pad with 0x00)*/ \
    '9', '5', '0', '0', '0', '0', '0', '0', 0x00, 0x00
    
#define CFG_ADV0_DATA_SCANRSP_PAYLOAD \
    0x09,0xff,0x00,0x60,'A','T','M','B','L','E'

/*
 * ADV1 (Pairing Mode)
 *******************************************************************************
 */

#define CFG_ADV1_CREATE_MAX_TX_POWER 0x00
#define CFG_ADV1_CREATE_PROPERTY ADV_LEGACY_UNDIR_CONN_MASK
#define CFG_ADV1_START_DURATION 3000 // 30s (unit of 10ms)

// TODO: Add name here too
#define CFG_ADV1_DATA_ADV_PAYLOAD \
    /* Complete service list (128-bit): */ \
    0x11, 0x07, \
    /* Gatt Service UUID (LSB): 11435b92-3653-4ab9-8c50-399456922854 */ \
    0x54, 0x28, 0x92, 0x56, 0x94, 0x39, 0x50, 0x8c, 0xb9, 0x4a, 0x53, 0x36, 0x92, 0x5b, 0x43, 0x11

// Same scan response as Adv0
#define CFG_ADV1_DATA_SCANRSP_PAYLOAD \
    CFG_ADV0_DATA_SCANRSP_PAYLOAD
    