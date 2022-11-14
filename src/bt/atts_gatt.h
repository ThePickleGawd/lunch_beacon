/**
 *******************************************************************************
 *
 * @file atts_gatt.h
 *
 * @brief BLE ATT Server Example Header
 *
 * Copyright (C) Atmosic 2020-2022
 *
 *******************************************************************************
 */
#pragma once

#include "ble_atmprfs.h"

#define ATTS_DATA_SIZE 512
#ifndef ATT_SERVER_LINK_MAX
#define ATT_SERVER_LINK_MAX ((BLE_CONNECTION_MAX > CFG_GAP_MAX_LINKS) ?\
    CFG_GAP_MAX_LINKS : BLE_CONNECTION_MAX)
#endif

/**
 * @brief ATT Server attributes enumeration.
 */
enum atts_attr_idx {
    ATTS_SVC_1,
    ATTS_CHAR_RW_1,
    ATTS_CHAR_NT_1,
    ATTS_CHAR_NT_CCCD_1,
    ATTS_SVC_2,
    ATTS_CHAR_IND_2,
    ATTS_CHAR_IND_CCCD_2,
    ATTS_SVC_3,
    ATTS_CHAR_RW_3,
    ATTS_CHAR_RW_CUDD_3,
    ATTS_CHAR_RW_4,
    ATTS_CHAR_RW_UDFD_4,
    ATTS_SVC_ECHO,
    ATTS_CHAR_RW_ECHO,
    ATTS_CHAR_NT_ECHO,
    ATTS_CHAR_NT_CCCD_ECHO,

    ATTS_ATTR_NUM
};

/**
 *******************************************************************************
 * @brief Update connect status for attribute server
 *
 * @param[in] conidx Connection index
 *******************************************************************************
 */
void atts_gatt_update_status(uint8_t conidx);

/**
 *******************************************************************************
 * @brief Clean related handler information
 *
 * @param[in] conidx Connection index
 *******************************************************************************
 */
void atts_gatt_clean_status(uint8_t conidx);

/**
 *******************************************************************************
 * @brief Get current connected link amount as server role
 *
 * @return amount of connected link
 *******************************************************************************
 */
uint8_t atts_gatt_connected_link_amount(void);

/**
 *******************************************************************************
 * @brief Create application specific gatt service
 *******************************************************************************
 */
void atts_create_prf(void);

/**
 *******************************************************************************
 * @brief Indicate atts database is registered
 *******************************************************************************
 */
void atts_init_done(void);

/**
 *******************************************************************************
 * @brief Reflect extra echo data to the echo characteristic triggered by other
 * callback event
 *
 * @param[in] echo_data data to be echoed
 *******************************************************************************
 */
__NONNULL_ALL
void atts_send_echo(char const *echo_data);
