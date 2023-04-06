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

#include "atm_adv_param.h"

typedef enum {
    S_INIT,               // 0
    S_IDLE,               // 1
    S_STARTING_LUNCH_ADV, // 2
    S_STARTING_PAIR_ADV,  // 3
    S_ADV_STARTED,        // 4
    S_ADV_STOPPED,        // 5
    S_CONNECTED,          // 6
} APP_STATE;

typedef enum {
    OP_MODULE_INIT,       // 0
    OP_CREATE_LUNCH_ADV,  // 1
    OP_CREATE_PAIR_ADV,   // 2
    OP_DELETE_PAIR_ADV,   // 3
    OP_CREATE_LUNCH_CFM,  // 4
    OP_CREATE_PAIR_CFM,   // 5
    OP_SLEEP,             // 6
    OP_ADV_TIMEOUT,       // 7
    OP_CONNECTED,         // 8
    OP_DISCONNECTED,      // 9
    OP_END = 0xFF
} APP_OP;

typedef enum {
    LUNCH_ADV_TYPE,
    PAIR_ADV_TYPE
} current_adv_t;

typedef enum {
    IDX_LUNCH,
    IDX_PAIR_ADV,
    IDX_MAX,
} adv_set_t;

STATIC_ASSERT(IDX_MAX <= CFG_GAP_ADV_MAX_INST, "CFG_GAP_ADV_MAX_INST too small!");

typedef struct {
    __ATM_ADV_CREATE_PARAM_CONST atm_adv_create_t *create[CFG_GAP_ADV_MAX_INST];
    __ATM_ADV_START_PARAM_CONST atm_adv_start_t *start[CFG_GAP_ADV_MAX_INST];
    __ATM_ADV_DATA_PARAM_CONST atm_adv_data_t *adv_data[CFG_GAP_ADV_MAX_INST];
    __ATM_ADV_DATA_PARAM_CONST atm_adv_data_t *scan_data[CFG_GAP_ADV_MAX_INST];
    current_adv_t current_adv_idx;
    // act_idx managed by adv api so we don't know if it's just 0 or 1, need to hash it here
    uint8_t act_idx[CFG_GAP_ADV_MAX_INST];
} app_env_t;

void testing_press_init(void);
