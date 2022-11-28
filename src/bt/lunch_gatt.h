/**
 *******************************************************************************
 *
 * @file lunch_gatt.h
 *
 * @brief BLE ATT Server Example Header
 *
 * Copyright (C) Atmosic 2020-2022
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

#define SVC_LUNCH_UUID 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, \
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0
#define CHAR_STUDENT_ID_UUID 0xA1, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, \
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA1
#define CHAR_SCHOOL_ID_UUID 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, \
    0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0

/**
 *******************************************************************************
 * @brief Create application specific gatt service
 *******************************************************************************
 */
void lunch_atts_create_prf(void);