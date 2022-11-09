/**
 *******************************************************************************
 *
 * @file BLE_adv.h
 *
 * @brief BLE Advertising Application
 *
 * Copyright (C) Atmosic 2020
 *
 *******************************************************************************
 */
#pragma once

typedef enum {
    S_INIT,
    S_IDLE,
    S_ADV_STARTING,
    S_ADV_STARTED,
    S_ADV_STOPPED,
    S_CONNECTED,
} APP_STATE;

typedef enum {
    OP_MODULE_INIT,
    OP_CREATE_ADV,
    OP_START_ADV,
    OP_START_ADV_CFM,
    OP_RESTART_ADV,
    OP_ADV_TIMEOUT,
    OP_CONNECTED,
    OP_DISCONNECTED,
    OP_END = 0xFF
} APP_OP;

/**
 *******************************************************************************
 * @brief Setup BLE_adv
 *******************************************************************************
 */
void setup_BLE_adv(void);