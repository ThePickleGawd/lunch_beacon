/**
 *******************************************************************************
 *
 * @file BLE_att_server.c
 *
 * @brief Generic ATT server example
 *
 * Copyright (C) Atmosic 2020-2022
 *
 *******************************************************************************
 */
#include "arch.h"
#include "atm_gap.h"
#include "atm_adv.h"
#include "atm_adv_param.h"
#include "atm_gap_param.h"
#include "BLE_att_server.h"
#include "atts_gatt.h"
#include "atm_asm.h"
#include "atm_pm.h"
#include "co_utils.h"
#include "rep_vec.h"

#define S_TBL_IDX 0

/**
 * @brief Application state enumeration.
 */
typedef enum {
    S_INIT,
    S_IDLE,
    S_ADV_CREATED,
    S_ADV_STARTED,
    S_CONNECTED,
} app_state_t;

/**
 * @brief Application state machine operation enumeration.
 */
typedef enum {
    OP_MODULE_INIT,
    OP_CREATE_ADV,
    OP_ADV_DATA,
    OP_SCANRSP_DATA,
    OP_START_ADV,
    OP_START_ADV_CFM,
    OP_CONNECTED,
    OP_DISCONNECTED,
    OP_END = 0xFF
} app_op_t;

ATM_LOG_LOCAL_SETTING("BLE_att_server", D);

static uint8_t adv_idx;
static pm_lock_id_t atts_lock_hib;
static atm_adv_data_nvds_t atts_adv_data, atts_scan_data;

STATIC_ASSERT(ATT_SERVER_LINK_MAX <= BLE_CONNECTION_MAX,
    "ATT_SERVER_LINK_MAX over BLE_CONNECTION_MAX");

/**
 * @brief Load adv and scan parameters from NVDS and set them into GAP layer.
 * @note Called when the advertisement activity is created.
 */
static void adv_create_cfm(uint8_t idx, ble_err_code_t status)
{
    ATM_LOG(D, "%s: status: %#x", __func__, status);

    adv_idx = idx;
    atm_adv_advdata_param_nvds(&atts_adv_data);
    atm_adv_scandata_param_nvds(&atts_scan_data);
    atm_adv_set_adv_data(idx, atm_adv_convert_nvds_data_type(&atts_adv_data));
    atm_adv_set_scan_data(idx, atm_adv_convert_nvds_data_type(&atts_scan_data));
}

/**
 * @brief Callback registered with the atm_adv module. Application
 * reacts for the following events: advertisement activity is created, started
 * and stopped.
 * @note Called when the advertisement state machine is changed.
 */
static void adv_evt_hdlr(atm_adv_state_t state, uint8_t idx,
    ble_err_code_t status)
{
    switch (state) {
	case ATM_ADV_CREATED: {
	    adv_create_cfm(idx, status);
	} break;

	case ATM_ADV_ADVDATA_DONE: {
	    ATM_LOG(D, "ATM_ADV_DATA_DONE");
	    atm_asm_move(S_TBL_IDX, OP_START_ADV_CFM);
	} break;

	case ATM_ADV_ON: {
	    ATM_LOG(D, "ATM_ADV_ON: Wait for connection");
	} break;

	case ATM_ADV_OFF: {
	    ATM_LOG(D, "ATM_ADV_OFF");
	    if (status == BLE_GAP_ERR_TIMEOUT) {
		ATM_LOG(D, "ADV_STOP_TIMEOUT");
		if (!atts_gatt_connected_link_amount()) {
		    atm_pm_unlock(atts_lock_hib);
		} else {
		    atm_asm_set_state_op(S_TBL_IDX, S_CONNECTED, OP_END);
		}
		return;
	    }
	    if (atts_gatt_connected_link_amount() < ATT_SERVER_LINK_MAX) {
		atm_asm_set_state_op(S_TBL_IDX, S_ADV_CREATED, OP_END);
		atm_asm_move(S_TBL_IDX, OP_START_ADV_CFM);
	    }
	} break;

	case ATM_ADV_CREATING:
	case ATM_ADV_ADVDATA_SETTING:
	case ATM_ADV_SCANDATA_SETTING:
	case ATM_ADV_STARTING:
	case ATM_ADV_STOPPING:
	case ATM_ADV_DELETING: {
	    ATM_LOG(D, "error adv_state = %d", state);
	    ASSERT_ERR(0);
	} break;

	case ATM_ADV_IDLE:
	case ATM_ADV_DELETED:
	case ATM_ADV_SCANDATA_DONE:
	default: {
	    ATM_LOG(D, "unhandle adv_state = %d", state);
	} break;
    }
}

/**
 * @brief Callback registered with the atm_gap module. Triggers the disconnect
 * operation for the application state machine transition: S_CONNECTED ->
 * S_ADV_CREATED
 * @note Called after the device has been disconnected.
 */
static void gap_disc_ind(uint8_t conidx, ble_gap_ind_discon_t const *param)
{
    ATM_LOG(D, "%s: reason: %#x", __func__, param->reason);

    atts_gatt_clean_status(conidx);

    if (atm_asm_get_current_state(S_TBL_IDX) == S_CONNECTED) {
	atm_asm_move(S_TBL_IDX, OP_DISCONNECTED);
    }
}

/**
 * @brief Callback registered with the atm_gap module. Triggers the connected
 * operation for the application state machine transition: S_ADV_STARTED ->
 * S_CONNECTED
 * @note Called after the device has been connected.
 */
static void gap_conn_ind(uint8_t conidx, atm_connect_info_t *param)
{
    ATM_LOG(D, "%s: ", __func__);
    atts_gatt_update_status(conidx);

    atm_asm_move(S_TBL_IDX, OP_CONNECTED);
    atm_gap_connect_accept(conidx);
}

/**
 * @brief Callback registered with the atm_gap module. Triggers the creating
 * advertisement operation for the application state machine transition:
 * S_IDLE -> S_ADV_CREATED
 * @note Called after the GAP has been initialized.
 */
static void gap_init_cfm(ble_err_code_t status)
{
    ATM_LOG(D, "%s: status: %#x", __func__, status);

    atts_init_done();
    atm_asm_move(S_TBL_IDX, OP_CREATE_ADV);
}

/**
 * @brief atm_gap callback functions for @ref atm_gap_start
 */
atm_gap_cbs_t const gap_cbfn = {
    .init_cfm = gap_init_cfm,
    .conn_ind = gap_conn_ind,
    .disc_ind = gap_disc_ind,
};

/**
 * @brief Fetches advertisment parameters and triggers GAP initialization.
 * Results in a state machine transition from S_INIT -> S_IDLE
 * @note Called upon app initialization
 */
static void app_init(void)
{
    ATM_LOG(D, "%s: con_max:%d, att_server:%d", __func__, BLE_CONNECTION_MAX,
	ATT_SERVER_LINK_MAX);

    // lock hibernate
    atts_lock_hib = atm_pm_alloc(PM_LOCK_HIBERNATE);
    atm_pm_lock(atts_lock_hib);

    // Register profile
    atts_create_prf();
    atm_gap_prf_reg(BLE_ATMPRFS_MODULE_NAME, NULL);

    // GAP initialize
    atm_gap_start(atm_gap_param_get(), &gap_cbfn);
}

/**
 * @brief Triggers creation and start of the advertisement. Results in a state
 * machine transition from S_IDLE -> S_ADV_CREATED
 * @note Called upon GAP initialization
 */
static void app_create_adv(void)
{
    ATM_LOG(D, "%s: ", __func__);

    atm_adv_create_t adv_create;
    atm_adv_create_param_nvds(false, &adv_create);

    ATM_LOG(D, "%s: adv_param.prop (%x)", __func__, adv_create.adv_param.prop);

    atm_adv_reg(adv_evt_hdlr);

    ble_err_code_t ret = atm_adv_create(&adv_create);
    if (ret != BLE_ERR_NO_ERROR) {
	ATM_LOG(D, "%s: atm_adv_create (%#x) fail!", __func__, ret);
    }
}

/**
 * @brief Start advertisement with the payload data in NVDS.
 */
static void app_start_adv(void)
{
    ATM_LOG(D, "%s:", __func__);
    atm_adv_start_t start;
    atm_adv_start_param_nvds(&start);
    atm_adv_start(adv_idx, &start);
    atm_asm_set_state_op(S_TBL_IDX, S_ADV_STARTED, OP_END);
}

/**
 * @brief Triggers a state machine transition from S_ADV_STARTED -> S_CONNECTED
 * or once the advertisement has stopped in the S_CONNECTED state
 * @note Called upon a connection establishment
 */
static void app_connected(void)
{
    ATM_LOG(D, "%s:", __func__);

    atm_asm_set_state_op(S_TBL_IDX, S_CONNECTED, OP_END);
}

/**
 * @brief Triggers a serie of state machine transition from S_CONNECTED ->
 * S_ADV_CREATED -> S_ADV_STARTED
 * @note Called when the device has been disconnected
 */
static void app_disconnected(void)
{
    ATM_LOG(D, "%s:", __func__);
    atm_asm_move(S_TBL_IDX, OP_START_ADV_CFM);
}

/**
 * @brief Application state machine table
 */
state_entry const s_tbl[] = {
    {S_OP(S_INIT, OP_MODULE_INIT), S_IDLE, app_init},
    {S_OP(S_IDLE, OP_CREATE_ADV), S_ADV_CREATED, app_create_adv},
    {S_OP(S_ADV_CREATED, OP_START_ADV_CFM), S_ADV_STARTED, app_start_adv},
    {S_OP(S_ADV_STARTED, OP_CONNECTED), S_CONNECTED, app_connected},
    {S_OP(S_CONNECTED, OP_DISCONNECTED), S_ADV_CREATED, app_disconnected},
};

/**
 * @brief Initialize the app data structures and start its state machine
 * @note Called after the platform drivers have initialized
 */
static rep_vec_err_t user_appm_init(void)
{
    bool init_app = true;
    ATM_LOG(D, "%s:", __func__);

    // Init. application state table
    atm_asm_init_table(S_TBL_IDX, s_tbl, ARRAY_LEN(s_tbl));

    if (init_app) {
	atm_asm_move(S_TBL_IDX, OP_MODULE_INIT);
    }
    return (RV_DONE);
}

/**
 * @brief Application user main(). Driver initialization and rep_vec additions
 */
int main(void)
{
    // Vector replacement
    RV_APPM_INIT_ADD_LAST(&user_appm_init);
    return 0;
}

