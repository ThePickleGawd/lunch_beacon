/**
 *******************************************************************************
 *
 * @file lunch_beacon.c
 *
 * @brief LunchTrak Beacon Application Source Code
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */

// C Stuff
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// Drivers
#include "arch.h"
#include "nvds.h"
#include "nvds_tag.h"
#include "co_error.h"
#include "co_utils.h"
#include "flash.h"
#include "wurx.h"
#include "ext_flash.h"
#include "atm_gpio.h"
#include "atm_log.h"
#include "atm_asm.h"
#include "atm_gap.h"
#include "atm_pm.h"
#include "atm_gap_param.h"
#include "atm_ble.h"
#include "atm_adv_param.h"
#include "atm_adv.h"
#include "atm_button.h"
#include "at_apb_pseq_regs_core_macro.h"
#include "at_wrpr.h"
#include "base_addr.h"
#include "pmu.h"
#include "ll.h"

// My stuff
#include "lunch_beacon.h"
#include "lunch_button.h"
#include "lunch_gatt.h"
#include "lunch_nvds.h"
#include "lunch_led.h"

ATM_LOG_LOCAL_SETTING("lunch_beacon", V);

/*
 * FUNCTION DECLARATIONS
 *******************************************************************************
 */

static uint8_t act_to_idx(uint8_t act_idx);
static void adv_state_change(atm_adv_state_t state, uint8_t act_idx, ble_err_code_t status);

/*
 * DEFINES
 *******************************************************************************
 */

#define S_TBL_IDX 0

#define GET_CREATE_ADV(act_idx) (app_env.create[act_to_idx(act_idx)])
#define GET_START_ADV(act_idx) (app_env.start[act_to_idx(act_idx)])
#define GET_ADV_DATA(act_idx) (app_env.adv_data[act_to_idx(act_idx)])
#define GET_SCAN_DATA(act_idx) (app_env.scan_data[act_to_idx(act_idx)])

#define ADV_LUNCH_DATA_IDX 8

/*
 * VARIABLES
 *******************************************************************************
 */

static app_env_t app_env;
static pm_lock_id_t lock_hiber;

/*
 * GAP CALLBACKS
 *******************************************************************************
 */

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the GAP layer has initialized
 */
static void gap_init_cfm(ble_err_code_t status)
{
    ATM_LOG(V, "%s", __func__);

    // Register adv state change callbacks
    atm_adv_reg(adv_state_change);

    // Set max transmit power
    atm_ble_set_txpwr_max(CFG_ADV0_CREATE_MAX_TX_POWER);

    // Init act_idx to ATM_INVALID_ACTIDX(0xFF)
    for (uint8_t idx = 0; idx < CFG_GAP_ADV_MAX_INST; idx++){
        app_env.act_idx[idx] = ATM_INVALID_ACTIDX;
    }

    // Create lunch adv if awoken normally by WuRX
    if(atm_asm_get_latest_transition(S_TBL_IDX).operation == OP_MODULE_INIT)
        atm_asm_move(S_TBL_IDX, OP_CREATE_LUNCH_ADV);
    // Create pair adv if awoken by button press
    else if(atm_asm_get_latest_transition(S_TBL_IDX).operation == OP_CREATE_PAIR_ADV)
        atm_asm_move(S_TBL_IDX, OP_CREATE_PAIR_ADV);
} 

/*
 * @brief Callback registered with the GAP layer
 * @note Called after a connection has been established
 */
static void gap_conn_ind(uint8_t conidx, atm_connect_info_t *param)
{
    ATM_LOG(V, "%s", __func__);

    if(app_env.current_adv_idx != PAIR_ADV_TYPE) {
        ATM_LOG(E, "Connection established but it's not from pair adv?");
        return;
    }

    ATM_LOG(D, "TODO: Reject connection if we already have one");

    // Set max transmit power for given connection
    atm_ble_set_con_txpwr(conidx, CFG_ADV1_CREATE_MAX_TX_POWER);

    // Accept connection and update state
    atm_gap_print_conn_param(param);
    atm_gap_connect_accept(conidx);
    // atm_gap_get_link_info(conidx, BLE_GAP_GET_PHY);
    atm_asm_move(S_TBL_IDX, OP_CONNECTED);
}

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the device has been disconnected
 */
static void gap_disc_ind(uint8_t conidx, ble_gap_ind_discon_t const *param)
{
    ATM_LOG(V, "%s", __func__);

    atm_asm_move(S_TBL_IDX, OP_DISCONNECTED);
}

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the PHY mode has been updated
 */
static void gap_phy_ind(uint8_t conidx, ble_gap_le_phy_t const *param)
{
    ATM_LOG(V, "%s", __func__);

    ATM_LOG(D, "%s: conidx=%d rx_phy=%d tx_phy=%d", __func__, conidx,
	param->rx_phy, param->tx_phy);
}

// GAP callbacks
static const atm_gap_cbs_t gap_callbacks = {
    .init_cfm = gap_init_cfm,
    .conn_ind = gap_conn_ind,
    .disc_ind = gap_disc_ind,
    .phy_ind = gap_phy_ind,
};

/*
 * WURX
 *******************************************************************************
 */

/**
 * @brief Enable WURX in prevent hibernation vector.
 * @note Called before the system enters the hibernation mode.
 * @param prevent Pointer to prevent control.
 * @param pseq_dur Pointer to hibernation duration.
 * @param ble_dur BLE sleep duration.
 */
__FAST static rep_vec_err_t
wurx_adv_prevent_hib(bool *prevent, int32_t *pseq_dur, int32_t ble_dur)
{
    if (!boot_was_cold()) {
        ATM_LOG(D, "Enabling WuRX");
	    wurx_enable();
    }
    return RV_NEXT;
}


/*
 * STATIC FUNCTIONS
 *******************************************************************************
 */

static void asm_state_change_cb(ASM_S last_s, ASM_O op, ASM_S next_s)
{
    ATM_LOG(V, "ASM State Change from %d to %d, with OP Code %d", last_s, next_s, op);
}

static void button_press_cb(void)
{
    atm_asm_move(S_TBL_IDX, OP_CREATE_PAIR_ADV);
}

static uint8_t act_to_idx(uint8_t act_idx)
{
    for (uint8_t idx = 0; idx < CFG_GAP_ADV_MAX_INST; idx++) {
	if (app_env.act_idx[idx] == act_idx) {
	    return idx;
	}
    }
    ATM_LOG(E, "act_to_idx could not find: act_idx=%d", act_idx);
    ASSERT_ERR(0);
    return 0;
}

/**
 * @brief Load adv and scan parameters and set them into GAP layer.
 * @note Called when the advertisement activity is created.
 */
static void ble_adv_create_cfm(uint8_t act_idx, ble_err_code_t status)
{
    ATM_LOG(V, "%s", __func__);

    ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);

    uint8_t idx;

    if(app_env.current_adv_idx == LUNCH_ADV_TYPE) idx = IDX_LUNCH;
    if(app_env.current_adv_idx == PAIR_ADV_TYPE) idx = IDX_PAIR_ADV;

    app_env.act_idx[idx] = act_idx;
    app_env.adv_data[idx] = atm_adv_advdata_param_get(idx);
    app_env.scan_data[idx] = atm_adv_scandata_param_get(idx);

    if (idx == IDX_LUNCH) {
        // Fetch lunch data for adv params
        nvds_lunch_data_t lunch_data = {0};
        uint8_t err = nvds_get_lunch_data(&lunch_data);
        if(err == NVDS_OK) {
            if(*lunch_data.school_id == 0 || *lunch_data.student_id == 0) {
                ATM_LOG(D, "Lunch Data not set yet, don't start adv");
                atm_asm_set_state_op(S_TBL_IDX, S_IDLE, OP_END);
                return;
            }

            memcpy((app_env.adv_data[idx]->data + ADV_LUNCH_DATA_IDX), (uint8_t *) &lunch_data, sizeof(nvds_lunch_data_t));
            ATM_LOG(D, "Found Lunch Data - School ID: %s - Student ID: %s", 
                lunch_data.school_id,
                lunch_data.student_id);
            ATM_LOG(D, "NEW ADV DATA: %s%s", (char *) app_env.adv_data[idx]->data, (char *) (app_env.adv_data[idx]->data + ADV_LUNCH_DATA_IDX+6));
        } else {
            ATM_LOG(E, "%s - Could not fetch lunch data. Err = %d", __func__, err);
            return;
        }
    }

    {
        ble_err_code_t ret = atm_adv_set_data_sanity(app_env.create[idx], app_env.adv_data[idx], app_env.scan_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set data sanity failed: %#x", __func__, ret);
            return;
        }
    }

    if(app_env.adv_data[idx]) {
        ble_err_code_t ret = atm_adv_set_adv_data(act_idx, app_env.adv_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set adv data failed: %#x", __func__, ret);
	        return;
        }
    }

    if(app_env.scan_data[idx]) {
        ble_err_code_t ret = atm_adv_set_scan_data(act_idx, app_env.scan_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set scan data failed: %#x", __func__, ret);
	        return;
        }
    }

    if(!app_env.scan_data[idx] && !app_env.adv_data[idx]) {
        ble_err_code_t ret = atm_adv_start(act_idx, app_env.start[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Failed to start adv with status %#x", __func__, ret);
	        return;
        }
    }
}

/*
 * @brief Callback registered with the atm_adv module
 * @note Called upon a state change in the advertising state machine
 */
static void adv_state_change(atm_adv_state_t state, uint8_t act_idx, ble_err_code_t status)
{
    ATM_LOG(V, "%s: act_idx=%d adv_state=%d", __func__, act_idx, state);

    ble_err_code_t ret = BLE_ERR_NO_ERROR;

    switch (state) {
        case ATM_ADV_CREATING:
        case ATM_ADV_ADVDATA_SETTING:
        case ATM_ADV_SCANDATA_SETTING:
        case ATM_ADV_STARTING:
        case ATM_ADV_STOPPING:
        case ATM_ADV_DELETING: {
        } break;
        case ATM_ADV_CREATED: {
            ATM_LOG(D, "ATM_ADV_CREATED act_idx=%d", act_idx);
            ble_adv_create_cfm(act_idx, status);
        } break;
        case ATM_ADV_ADVDATA_DONE: {
            // Start adv if adv is off and theres no scan data
            if(atm_adv_get_state(act_idx) != ATM_ADV_OFF || GET_SCAN_DATA(act_idx)) break;
            ret = atm_adv_start(act_idx, GET_START_ADV(act_idx));
        } break;
        case ATM_ADV_SCANDATA_DONE: {
            if(atm_asm_get_current_state(S_TBL_IDX) == S_ADV_STARTED) break;
            ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
            ret = atm_adv_start(act_idx, GET_START_ADV(act_idx));
        } break;
        case ATM_ADV_ON: {
            ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
            if(act_to_idx(act_idx) == IDX_LUNCH) {
                // Lunch beacon confirmation (can start now)
                atm_asm_move(S_TBL_IDX, OP_CREATE_LUNCH_CFM);

                // Blink LED to confirm
                // lunch_led_blink(LUNCH_LED_ACTIVE);
            } else {
                // Pairing mode confirmation (can start now)
                atm_asm_move(S_TBL_IDX, OP_CREATE_PAIR_CFM);

                // Blink LED to confirm
                lunch_led_blink(LUNCH_LED_PAIRING);
            }
        } break;
        case ATM_ADV_OFF: {
            if(atm_asm_get_current_state(S_TBL_IDX) != S_STARTING_PAIR_ADV)
                atm_asm_move(S_TBL_IDX, OP_ADV_TIMEOUT);
        } break;
        case ATM_ADV_DELETED: {
            // If we deleted the pair adv, go into regular beacon mode
            if(app_env.current_adv_idx == PAIR_ADV_TYPE) {
                atm_asm_move(S_TBL_IDX, OP_CREATE_LUNCH_ADV);
            }
        } break;
        case ATM_ADV_IDLE:
        default: {
            ATM_LOG(E, "Unhandled state = %d", state);
        } break;
    }

    if(ret != BLE_ERR_NO_ERROR) {
        ATM_LOG(E, "%s: Failed with status: %#x", __func__, ret);
    }
}

/*
 * STATE MACHINE
 *******************************************************************************
 */

/*
 * @brief Fetches advertisment parameters and triggers GAP initialization.
 * Results in a state machine transition from S_INIT -> S_IDLE
 * @note Called upon app initialization
 */
static void lunch_s_init(void)
{
    ATM_LOG(V, "%s", __func__);

    // Create gatt profile
    lunch_atts_create_prf();
    atm_gap_prf_reg(BLE_ATMPRFS_MODULE_NAME, NULL); 

    // Assign Random Static ADDR
    // atm_gap_gen_rand_addr(BLE_GAP_STATIC_ADDR);

    // Start gap
    atm_gap_start(atm_gap_param_get(), &gap_callbacks);
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTING -> S_ADV_STARTED
 * @note Called once the advertisement has been created and started
 */
static void lunch_s_start_on(void)
{
    ATM_LOG(V, "%s", __func__);
    atm_asm_set_state_op(S_TBL_IDX, S_ADV_STARTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTED -> S_CONNECTED
 * or once the advertisement has stopped in the S_CONNECTED state
 * @note Called upon a connection establishment
 */
static void lunch_s_connected(void)
{
    ATM_LOG(V, "%s", __func__);
    atm_asm_set_state_op(S_TBL_IDX, S_CONNECTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_CONNECTED -> S_ADV_STOPPED
 * @note Called when the device has been disconnected
 */
static void lunch_s_disconnected(void)
{
    ATM_LOG(V, "%s", __func__);
    
    // LED off indicator
    // lunch_led_blink(LUNCH_LED_OFF);

    atm_asm_move(S_TBL_IDX, OP_SLEEP);
}

/*
 * @brief Triggers a state machinetransition from S_ADV_STARTED -> S_ADV_STOPPED
 * @note Called when the advertisement has stopped in the S_ADV_STARTED state
 */
static void lunch_s_timeout(void)
{
    ATM_LOG(V, "%s", __func__);

    // LED off indicator
    // lunch_led_blink(LUNCH_LED_OFF);

    if(app_env.current_adv_idx == LUNCH_ADV_TYPE) {
        atm_asm_move(S_TBL_IDX, OP_SLEEP);
    } else {
        atm_asm_move(S_TBL_IDX, OP_DELETE_PAIR_ADV);
    }
}

static void lunch_s_create_lunch_adv(void)
{
    ATM_LOG(V, "%s", __func__);

    // Fetch params
    app_env.create[IDX_LUNCH] = atm_adv_create_param_get(IDX_LUNCH);
    app_env.start[IDX_LUNCH] = atm_adv_start_param_get(IDX_LUNCH);
    app_env.current_adv_idx = LUNCH_ADV_TYPE;

    if(app_env.act_idx[IDX_LUNCH] != ATM_INVALID_ACTIDX) {
        atm_adv_start(app_env.act_idx[IDX_LUNCH], app_env.start[IDX_LUNCH]);
    } else {
        atm_adv_create(app_env.create[IDX_LUNCH]); // adv_state_change (ATM_ADV_CREATED)
    }
}

static void lunch_s_create_pair_adv(void)
{
    ATM_LOG(V, "%s", __func__);
    
    // Fetch params
    app_env.create[IDX_PAIR_ADV] = atm_adv_create_param_get(IDX_PAIR_ADV);
    app_env.start[IDX_PAIR_ADV] = atm_adv_start_param_get(IDX_PAIR_ADV);
    app_env.current_adv_idx = PAIR_ADV_TYPE;

    if(app_env.act_idx[IDX_PAIR_ADV] != ATM_INVALID_ACTIDX) {
        atm_adv_start(app_env.act_idx[IDX_PAIR_ADV], app_env.start[IDX_PAIR_ADV]);
    } else {
        atm_adv_create(app_env.create[IDX_PAIR_ADV]); // adv_state_change (ATM_ADV_CREATED)
    }
}

static void lunch_s_delete_pair_adv(void)
{
    ATM_LOG(V, "%s", __func__);

    atm_adv_delete(app_env.act_idx[IDX_PAIR_ADV]);
    app_env.act_idx[IDX_PAIR_ADV] = ATM_INVALID_ACTIDX;

    atm_pm_unlock(lock_hiber);
}

static void lunch_s_stop_adv_and_pair(void)
{
    ATM_LOG(V, "%s", __func__);

    if(app_env.current_adv_idx == LUNCH_ADV_TYPE) {
        atm_adv_stop(app_env.act_idx[IDX_LUNCH]);
        atm_asm_move(S_TBL_IDX, OP_CREATE_PAIR_ADV);
    } else {
        // Don't go to idle mode
        atm_asm_set_state_op(S_TBL_IDX, S_ADV_STARTED, OP_END);
    }
}

static void lunch_s_sleep(void)
{
    lunch_led_off();
    atm_pm_unlock(lock_hiber);
}

static const state_entry s_tbl[] = {
    // Initialize module
    {S_OP(S_INIT, OP_MODULE_INIT), S_IDLE, lunch_s_init},
    // Create lunch beacon
    {S_OP(S_IDLE, OP_CREATE_LUNCH_ADV), S_STARTING_LUNCH_ADV, lunch_s_create_lunch_adv},
    // Create pairing beacon
    {S_OP(S_INIT, OP_CREATE_PAIR_ADV), S_IDLE, lunch_s_init},
    {S_OP(S_IDLE, OP_CREATE_PAIR_ADV), S_STARTING_PAIR_ADV, lunch_s_create_pair_adv},
    // Start the lunch beacon after receiving confirmation
    {S_OP(S_STARTING_LUNCH_ADV, OP_CREATE_LUNCH_CFM), S_ADV_STARTED, lunch_s_start_on},
    // Start the pairing beacon after receiving confirmation
    {S_OP(S_STARTING_PAIR_ADV, OP_CREATE_PAIR_CFM), S_ADV_STARTED, lunch_s_start_on},
    // Stop adv and move to go into pairing mode
    {S_OP(S_ADV_STARTED, OP_CREATE_PAIR_ADV), S_IDLE, lunch_s_stop_adv_and_pair},
    
    // Handle connections, timeouts, restarts
    {S_OP(S_ADV_STARTED, OP_CONNECTED), S_CONNECTED, lunch_s_connected},
    {S_OP(S_CONNECTED, OP_DISCONNECTED), S_ADV_STOPPED, lunch_s_disconnected},
    {S_OP(S_CONNECTED, OP_ADV_TIMEOUT), S_CONNECTED, lunch_s_connected},
    {S_OP(S_ADV_STARTED, OP_ADV_TIMEOUT), S_ADV_STOPPED, lunch_s_timeout},
    {S_OP(S_ADV_STOPPED, OP_DELETE_PAIR_ADV), S_IDLE, lunch_s_delete_pair_adv},
    {S_OP(S_ADV_STOPPED, OP_SLEEP), S_IDLE, lunch_s_sleep}
};

__FAST static rep_vec_err_t
enter_hib(bool *sleep, int32_t duration, uint32_t int_set)
{
    ATM_LOG(D, "Entering Hibernation Mode");
    return RV_NEXT;
}

static rep_vec_err_t user_appm_init(void)
{
    // Initialize state machine
    atm_asm_init_table(S_TBL_IDX, s_tbl, ARRAY_LEN(s_tbl));
    atm_asm_reg_state_change_cb(S_TBL_IDX, asm_state_change_cb);
    atm_asm_set_state_op(S_TBL_IDX, S_INIT, OP_END);

    // Setup WuRX
    RV_PLF_PREVENT_HIBERNATION_ADD_LAST(wurx_adv_prevent_hib);
    RV_PLF_HIBERNATE_ADD(enter_hib);

    lock_hiber = atm_pm_alloc(PM_LOCK_HIBERNATE);

    // Check if woken by WuRX or button
    if (!boot_was_cold()) {
        ATM_LOG(D, "WuRX Boot");

        // Check if we are pressing button
        lunch_button_on_wake();

        wurx_disable();
        atm_pm_lock(lock_hiber);
        
        // Move state machine
        atm_asm_move(S_TBL_IDX, OP_MODULE_INIT);
    } else {
        ATM_LOG(D, "Cold Boot");

        atm_pm_unlock(lock_hiber);
    }


    return RV_DONE;
}

int main(void) 
{
    // Initialize button
    lunch_button_init(button_press_cb);

    RV_APPM_INIT_ADD_LAST(user_appm_init);

    ATM_LOG(D, "user_main() done%s", "");
    return 0;
}