/**
 *******************************************************************************
 *
 * @file BLE_adv.c
 *
 * @brief BLE Advertising Application
 *
 * Copyright (C) Atmosic 2020-2022
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "arch.h"
#include "atm_ble.h"
#include "atm_gap.h"
#include "atm_adv.h"
#include "atm_adv_param.h"
#include "atm_gap_param.h"
#include "nvds.h"
#include "nvds_tag.h"
#include "BLE_adv.h"
#include "atm_asm.h"
#include "atm_pm.h"
#include "sw_timer.h"
#include "co_utils.h"
#include "gap_params.h"
#ifdef AUTO_TEST
#include "uart_stdout.h"
#endif

#define S_TBL_IDX 0

#ifndef CFG_ADV0_CREATE_MAX_TX_POWER
#define CFG_ADV0_CREATE_MAX_TX_POWER 0
#endif

ATM_LOG_LOCAL_SETTING("BLE_adv", D);

static uint8_t activity_idx = ATM_INVALID_ADVIDX;
static uint32_t restart_time_csec; // centi-seconds
static sw_timer_id_t tid_restart;

#if !defined(ENABLE_USER_ADV_PARAM_SETTING) && defined(CFG_NVDS_ADV)
static atm_adv_create_t adv_create;
#endif
static __ATM_ADV_CREATE_PARAM_CONST atm_adv_create_t *create;

#if !defined(ENABLE_USER_ADV_DATA_SCANRSP) && defined(CFG_NVDS_ADV)
static atm_adv_data_nvds_t adv_nvds_data;
static atm_adv_data_nvds_t scan_nvds_data;
#endif
static __ATM_ADV_DATA_PARAM_CONST atm_adv_data_t *adv_data;
static __ATM_ADV_DATA_PARAM_CONST atm_adv_data_t *scan_data;

#if !defined(ENABLE_USER_ADV_TIMEOUT) && defined(CFG_NVDS_ADV)
static atm_adv_start_t adv_start;
#endif
static __ATM_ADV_START_PARAM_CONST atm_adv_start_t *start;

#ifdef CFG_DYN_ADV
#define ATM_CNT_IDX 24
#define USER_DISPLAY_DIGIT 3
#define ITVL_UPDATE_ADV_PAYLOAD_CS 1000
static sw_timer_id_t tid_update_adv;
static uint8_t adv_cnt = 0;

static void update_adv_content(void)
{
    char data_tmp[USER_DISPLAY_DIGIT + 1];
    snprintf(data_tmp, USER_DISPLAY_DIGIT + 1, "%03d", adv_cnt);

    DEBUG_TRACE("%s: adv count (%d)", __func__, adv_cnt);
    adv_cnt++;

    if (adv_data) {
	adv_data->len = ATM_ADV_LEN;
	memcpy(&(adv_data->data[ATM_CNT_IDX]), data_tmp,
	    USER_DISPLAY_DIGIT);
	ble_err_code_t ret = atm_adv_set_adv_data(activity_idx, adv_data);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Set adv data failed: %#x", __func__, ret);
	    return;
	}
    }

    if (scan_data) {
	scan_data->len = ATM_ADV_LEN;
	memcpy(&(scan_data->data[ATM_CNT_IDX]), data_tmp,
	    USER_DISPLAY_DIGIT);
	ble_err_code_t ret = atm_adv_set_scan_data(activity_idx, scan_data);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Set scan data failed: %#x", __func__, ret);
	    return;
	}
    }
}

static void update_adv_timer(uint8_t idx, void const *ctx)
{
    if (activity_idx == ATM_INVALID_ADVIDX) {
	return;
    }

    if (atm_asm_get_current_state(activity_idx) != S_ADV_STARTED) {
	adv_cnt = 0;
	sw_timer_clear(tid_update_adv);
	return;
    }

    sw_timer_set(tid_update_adv, ITVL_UPDATE_ADV_PAYLOAD_CS);
    update_adv_content();
}
#endif // CFG_DYN_ADV

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the GAP layer has initialized
 */
static void adv_init_cfm(ble_err_code_t status)
{
    // Set max transmit power.
    atm_ble_set_txpwr_max(CFG_ADV0_CREATE_MAX_TX_POWER);

    atm_asm_move(S_TBL_IDX, OP_START_ADV);
}

/*
 * @brief Callback registered with the GAP layer
 * @note Called after a connection has been established
 */
static void adv_conn_ind(uint8_t conidx, atm_connect_info_t *param)
{
    // Set max transmit power for given connection
    atm_ble_set_con_txpwr(conidx, CFG_ADV0_CREATE_MAX_TX_POWER);

    atm_gap_print_conn_param(param);
    atm_gap_connect_accept(conidx);
    atm_gap_get_link_info(conidx, BLE_GAP_GET_PHY);
    atm_asm_move(S_TBL_IDX, OP_CONNECTED);
}

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the device has been disconnected
 */
static void adv_disc_ind(uint8_t conidx, ble_gap_ind_discon_t const *param)
{
    atm_asm_move(S_TBL_IDX, OP_DISCONNECTED);
}

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the PHY mode has been updated
 */
static void adv_phy_ind(uint8_t conidx, ble_gap_le_phy_t const *param)
{
    ATM_LOG(D, "%s: conidx=%d rx_phy=%d tx_phy=%d", __func__, conidx,
	param->rx_phy, param->tx_phy);
}

// GAP callbacks
static const atm_gap_cbs_t gap_callbacks = {
    .conn_ind = adv_conn_ind,
    .disc_ind = adv_disc_ind,
    .init_cfm = adv_init_cfm,
    .phy_ind = adv_phy_ind,
};

static void ble_adv_create_cfm(uint8_t act_idx, ble_err_code_t status)
{
    ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
    activity_idx = act_idx;

    {
	ble_err_code_t ret = atm_adv_set_data_sanity(create, adv_data, scan_data);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Set data sanity failed: %#x", __func__, ret);
	    return;
	}
    }

    if (adv_data) {
	ble_err_code_t ret = atm_adv_set_adv_data(activity_idx, adv_data);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Set adv data failed: %#x", __func__, ret);
	    return;
	}
    }

    if (scan_data) {
	ble_err_code_t ret = atm_adv_set_scan_data(activity_idx, scan_data);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Set scan data failed: %#x", __func__, ret);
	    return;
	}
    }

    if (!scan_data && !adv_data) {
	ble_err_code_t ret = atm_adv_start(activity_idx, start);
	if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Failed to start adv with status %#x", __func__,
		ret);
	    return;
	}
    }
}

/*
 * @brief Callback registered with the atm_adv module
 * @note Called upon a state change in the advertising state machine
 */
static void adv_state_change(atm_adv_state_t state, uint8_t act_idx,
    ble_err_code_t status)
{
    ble_err_code_t ret = BLE_ERR_NO_ERROR;

    ATM_LOG(D, "adv_state = %d", state);
    switch (state) {
	case ATM_ADV_CREATING:
	case ATM_ADV_ADVDATA_SETTING:
	case ATM_ADV_SCANDATA_SETTING:
	case ATM_ADV_STARTING:
	case ATM_ADV_STOPPING:
	case ATM_ADV_DELETING: {
	} break;
	case ATM_ADV_CREATED: {
	    ble_adv_create_cfm(act_idx, status);
	} break;
	case ATM_ADV_ADVDATA_DONE: {
	    if (atm_asm_get_current_state(act_idx) != S_ADV_STARTED) {
		if (!scan_data) {
		    ret = atm_adv_start(activity_idx, start);
		}
	    }
	} break;
	case ATM_ADV_SCANDATA_DONE: {
	    if (atm_asm_get_current_state(act_idx) != S_ADV_STARTED) {
		ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
		ret = atm_adv_start(activity_idx, start);
	    }
	} break;
	case ATM_ADV_ON: {
	    ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
	    atm_asm_move(S_TBL_IDX, OP_START_ADV_CFM);
	} break;
	case ATM_ADV_OFF: {
	    atm_asm_move(S_TBL_IDX, OP_ADV_TIMEOUT);
#ifdef AUTO_TEST
	    if (!restart_time_csec) {
		UartEndSimulation();
	    }
#endif
	} break;
	case ATM_ADV_IDLE:
	case ATM_ADV_DELETED:
	default: {
	    ATM_LOG(E, "Unhandled state = %d", state);
	} break;
    }

    if (ret != BLE_ERR_NO_ERROR) {
	ATM_LOG(E, "%s: Failed with status: %#x", __func__, ret);
    }
}

/*
 * @brief Fetches advertisment parameters and triggers GAP initialization.
 * Results in a state machine transition from S_INIT -> S_IDLE
 * @note Called upon app initialization
 */
static void ble_adv_init(void)
{
    atm_gap_start(atm_gap_param_get(), &gap_callbacks);

#if !defined(ENABLE_USER_ADV_PARAM_SETTING) && defined(CFG_NVDS_ADV)
    if (atm_adv_create_param_nvds(false, &adv_create)) {
	create = &adv_create;
    } else {
	ATM_LOG(E, "%s: NVDS tag for create adv param not found.", __func__);
	ASSERT_ERR(0);
    }
#else
    create = atm_adv_create_param_get(0);
#endif

#if !defined(ENABLE_USER_ADV_DATA_SCANRSP) && defined(CFG_NVDS_ADV)
    if (atm_adv_advdata_param_nvds(&adv_nvds_data)) {
	adv_data = atm_adv_convert_nvds_data_type(&adv_nvds_data);
    } else {
	ATM_LOG(V, "%s: NVDS tag for set adv data param not found.", __func__);
    }

    if (atm_adv_scandata_param_nvds(&scan_nvds_data)) {
	scan_data = atm_adv_convert_nvds_data_type(&scan_nvds_data);
    } else {
	ATM_LOG(V, "%s: NVDS tag for set scan resp param not found.", __func__);
    }
#else
    adv_data = atm_adv_advdata_param_get(0);
    scan_data = atm_adv_scandata_param_get(0);
#endif

#if !defined(ENABLE_USER_ADV_TIMEOUT) && defined(CFG_NVDS_ADV)
    if (atm_adv_start_param_nvds(&adv_start)) {
	start = &adv_start;
    } else {
	ATM_LOG(D, "%s: NVDS tag for adv timeout param not found. Using default"
	    , __func__);
	start = atm_adv_start_param_get(0);
    }
#else
    start = atm_adv_start_param_get(0);
#endif
}

/*
 * @brief Triggers creation and start of the advertisement. Results in a state
 * machine transition from S_IDLE -> S_ADV_STARTING
 * @note Called upon GAP initialization
 */
static void ble_adv_start_adv(void)
{
    ble_err_code_t ret = atm_adv_timeout_param_print(create, start);
    if (ret != BLE_ERR_NO_ERROR) {
	ATM_LOG(E, "%s: Failed to print adv timeout param %d", __func__, ret);
	return;
    }
    atm_adv_reg(adv_state_change);
    ret = atm_adv_create(create);
    if (ret != BLE_ERR_NO_ERROR) {
	ATM_LOG(E, "%s: Failed to create adv %#x", __func__, ret);
    }
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTING ->
 * S_ADV_STARTED
 * @note Called once the advertisement has been created and started
 */
static void ble_adv_start_on(void)
{
#ifdef CFG_DYN_ADV
    update_adv_content();
    sw_timer_set(tid_update_adv, ITVL_UPDATE_ADV_PAYLOAD_CS);
#endif // CFG_DYN_ADV
    atm_asm_set_state_op(S_TBL_IDX, S_ADV_STARTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTED -> S_CONNECTED
 * or once the advertisement has stopped in the S_CONNECTED state
 * @note Called upon a connection establishment
 */
static void ble_adv_connected(void)
{
    atm_asm_set_state_op(S_TBL_IDX, S_CONNECTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_CONNECTED -> S_ADV_STOPPED
 * @note Called when the device has been disconnected
 */
static void ble_adv_disconnected(void)
{
    atm_asm_move(S_TBL_IDX, OP_RESTART_ADV);
}

static pm_lock_id_t adv_lock_hiber;

/*
 * @brief Starts a timer to restart advertisement. Results in a state machine
 * transition from S_ADV_STARTED -> S_ADV_STOPPED
 * @note Called when the advertisement has stopped in the S_ADV_STARTED state
 */
static void ble_adv_timeout(void)
{
    atm_pm_unlock(adv_lock_hiber);
    if (restart_time_csec) {
	sw_timer_set(tid_restart, restart_time_csec);
    }
}

/*
 * @brief Triggers starting of the advertisement. Resulting in a state machine
 * transition from S_ADV_STOPPED -> S_ADV_STARTING
 * @note Called when the advertisement restart timer has expired in the
 * S_ADV_STOPPED state or the device gets disconnected after connection
 */
static void ble_adv_restart_adv(void)
{
    ble_err_code_t ret = atm_adv_start(activity_idx, start);
    if (ret != BLE_ERR_NO_ERROR) {
	ATM_LOG(E, "%s: Failed to restart adv with status %#x", __func__, ret);
    }
}

/*
 * @brief Starts the advertisement
 * @note Scheduled when the advertisement has stopped in the S_ADV_STARTED state
 */
static void restart_timer(uint8_t idx, const void *ctx)
{
    atm_asm_move(S_TBL_IDX, OP_RESTART_ADV);
}

static const state_entry s_tbl[] = {
    {S_OP(S_INIT, OP_MODULE_INIT), S_IDLE, ble_adv_init},
    {S_OP(S_IDLE, OP_START_ADV), S_ADV_STARTING, ble_adv_start_adv},
    {S_OP(S_ADV_STARTING, OP_START_ADV_CFM), S_ADV_STARTED, ble_adv_start_on},
    {S_OP(S_ADV_STARTED, OP_CONNECTED), S_CONNECTED, ble_adv_connected},
    {S_OP(S_CONNECTED, OP_DISCONNECTED), S_ADV_STOPPED, ble_adv_disconnected},
    {S_OP(S_CONNECTED, OP_ADV_TIMEOUT), S_CONNECTED, ble_adv_connected},
    {S_OP(S_ADV_STARTED, OP_ADV_TIMEOUT), S_ADV_STOPPED, ble_adv_timeout},
    {S_OP(S_ADV_STOPPED, OP_RESTART_ADV), S_ADV_STARTING, ble_adv_restart_adv}
};

/*
 * @brief Initialize the app data structures and start its state machine
 * @note Called after the platform drivers have initialized
 */
static rep_vec_err_t user_appm_init(void)
{
    // Read restart duration from NVDS
    nvds_tag_len_t restart_dur_size = sizeof(restart_time_csec);
    if (nvds_get(NVDS_TAG_APP_BLE_RSTRT_DUR, &restart_dur_size,
	(uint8_t *)&restart_time_csec)) {
    }

    if (restart_time_csec) {
	tid_restart = sw_timer_alloc(restart_timer, NULL);
    }

#ifdef CFG_DYN_ADV
    tid_update_adv = sw_timer_alloc(update_adv_timer, NULL);
#endif // CFG_DYN_ADV

    adv_lock_hiber = atm_pm_alloc(PM_LOCK_HIBERNATE);
    atm_pm_lock(adv_lock_hiber);
    atm_pm_set_hib_restart_time(restart_time_csec);

    atm_asm_init_table(S_TBL_IDX, s_tbl, ARRAY_LEN(s_tbl));
    atm_asm_set_state_op(S_TBL_IDX, S_INIT, OP_END);
    atm_asm_move(S_TBL_IDX, OP_MODULE_INIT);

    // Don't use the app in the ROM
    return RV_DONE;
}

/*
 * user_main()
 * Driver initialization and rep_vec additions
 */
int main(void)
{
    /*
     * Vector replacement
     */
    RV_APPM_INIT_ADD_LAST(user_appm_init);

    ATM_LOG(D, "user_main() done");
    return 0;
}
