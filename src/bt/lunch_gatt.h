/**
 *******************************************************************************
 *
 * @file lunch_gatt.h
 *
 * @brief Lunch GATT Server Source Code
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */
#pragma once

#include "ble_atmprfs.h"

/**
 * @brief ATT Server attributes enumeration
*/
enum atts_attr_idx {
    ATTS_SVC_LUNCH,
    ATTS_CHAR_RW_STUDENT_ID,
    ATTS_CHAR_RW_SCHOOL_ID,
    ATTS_CHAR_CCCD,

    ATTS_ATTR_NUM
};

#define ATTS_DATA_SIZE 512

#define ATTS_SVC_SEC_PROPERTY BLE_SEC_PROP_NO_SECURITY
#define ATTS_RW_SEC_PROPERTY \
    (BLE_ATT_READ_NO_SECURITY | BLE_ATT_WRITE_REQ_NO_SECURITY)

// 11435b92-3653-4ab9-8c50-399456922854
#define SVC_LUNCH_UUID 0x11, 0x43, 0x5b, 0x92, 0x36, 0x53, 0x4a, 0xb9, 0x8c, 0x50, 0x39, 0x94, 0x56, 0x92, 0x28, 0x54

// 228f4919-f4f8-4bb5-ba38-243a110b7a24
#define CHAR_SCHOOL_ID_UUID 0x22, 0x8f, 0x49, 0x19, 0xf4, 0xf8, 0x4b, 0xb5, 0xba, 0x38, 0x24, 0x3a, 0x11, 0x0b, 0x7a, 0x24

// 33f68a3f-e981-4fd8-a13c-b6a0edd1928d
#define CHAR_STUDENT_ID_UUID 0x33, 0xf6, 0x8a, 0x3f, 0xe9, 0x81, 0x4f, 0xd8, 0xa1, 0x3c, 0xb6, 0xa0, 0xed, 0xd1, 0x92, 0x8d

/**
 *******************************************************************************
 * @brief Create application specific gatt service
 *******************************************************************************
 */
void lunch_atts_create_prf(void);