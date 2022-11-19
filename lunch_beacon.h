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

typedef enum {
    LUNCH_ADV_TYPE,
    PAIR_ADV_TYPE
} adv_create_t;

typedef enum {
    IDX_LUNCH,
    IDX_PAIR_ADV,
    IDX_MAX,
} adv_set_t;

STATIC_ASSERT(IDX_MAX <= CFG_GAP_ADV_MAX_INST, "CFG_GAP_ADV_MAX_INST too small!");

typedef struct {
    atm_adv_create_t *create[CFG_GAP_ADV_MAX_INST];
    atm_adv_start_t *start[CFG_GAP_ADV_MAX_INST];
    atm_adv_data_t *adv_data[CFG_GAP_ADV_MAX_INST];
    atm_adv_data_t *scan_data[CFG_GAP_ADV_MAX_INST];
    adv_create_t create_adv_idx;
    // act_idx managed by adv api so we don't know if it's just 0 or 1, need to hash it here
    uint8_t act_idx[CFG_GAP_ADV_MAX_INST];
} app_env_t;