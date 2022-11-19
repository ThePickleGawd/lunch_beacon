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

// My stuff
#include "lunch_beacon.h"

ATM_LOG_LOCAL_SETTING("Lunch Beacon", D);

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

/*
 * VARIABLES
 *******************************************************************************
 */

static app_env_t app_env;
static pm_lock_id_t adv_lock_hiber;
static uint8_t activity_idx = ATM_INVALID_ACTIDX;

/*
 * GAP CALLBACKS
 *******************************************************************************
 */

/*
 * @brief Callback registered with the GAP layer
 * @note Called after the GAP layer has initialized
 */
static void adv_init_cfm(ble_err_code_t status)
{
    // Register adv state change callbacks
    atm_adv_reg(adv_state_change);

    // Set max transmit power
    atm_ble_set_txpwr_max(CFG_ADV0_CREATE_MAX_TX_POWER);

    atm_asm_move(S_TBL_IDX, OP_CREATE_LUNCH_ADV);
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
    .init_cfm = adv_init_cfm,
    .conn_ind = adv_conn_ind,
    .disc_ind = adv_disc_ind,
    .phy_ind = adv_phy_ind,
};

/*
 * STATIC FUNCTIONS
 *******************************************************************************
 */

static uint8_t act_to_idx(uint8_t act_idx)
{
    for (uint8_t idx = 0; idx < CFG_GAP_ADV_MAX_INST; idx++) {
	if (app_env.act_idx[idx] == act_idx) {
	    return idx;
	}
    }
    ATM_LOG(E, "act_to_idx not find: act_idx=%d", act_idx);
    ASSERT_ERR(0);
    return 0;
}

// BLE create confirmation
static void ble_adv_create_cfm(uint8_t act_idx, ble_err_code_t status)
{
    ATM_LOG(D, "%s", __func__);

    ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
    activity_idx = act_idx;

    uint8_t idx;

    if(app_env.create_adv_idx == LUNCH_ADV_TYPE) idx = IDX_LUNCH;
    if(app_env.create_adv_idx == PAIR_ADV_TYPE) idx = IDX_PAIR_ADV;

    app_env.act_idx[idx] = act_idx;
    app_env.adv_data[idx] = atm_adv_advdata_param_get(idx);
    app_env.scan_data[idx] = atm_adv_scandata_param_get(idx);

    {
        ble_err_code_t ret = atm_adv_set_data_sanity(app_env.create[idx], app_env.adv_data[idx], app_env.scan_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set data sanity failed: %#x", __func__, ret);
            return;
        }
    }

    if(app_env.adv_data[idx]) {
        ble_err_code_t ret = atm_adv_set_adv_data(activity_idx, app_env.adv_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set adv data failed: %#x", __func__, ret);
	        return;
        }
    }

    if(app_env.scan_data[idx]) {
        ble_err_code_t ret = atm_adv_set_scan_data(activity_idx, app_env.scan_data[idx]);
        if(ret != BLE_ERR_NO_ERROR) {
            ATM_LOG(E, "%s: Set scan data failed: %#x", __func__, ret);
	        return;
        }
    }

    if(!app_env.scan_data[idx] && !app_env.adv_data[idx]) {
        ble_err_code_t ret = atm_adv_start(activity_idx, GET_START_ADV(idx));
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
            // Create or start adv after it's created
            ATM_LOG(D, "ATM_ADV_CREATED act_idx=%d", act_idx);
            ble_adv_create_cfm(act_idx, status);
        } break;
        case ATM_ADV_ADVDATA_DONE: {
            // Start adv if we haven't already started
            if(atm_asm_get_current_state(act_idx) == S_ADV_STARTED || GET_SCAN_DATA(act_idx)) break;
            ret = atm_adv_start(activity_idx, GET_START_ADV(act_idx));
        } break;
        case ATM_ADV_SCANDATA_DONE: {
            if(atm_asm_get_current_state(act_idx) == S_ADV_STARTED) break;
            ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
            ret = atm_adv_start(activity_idx, GET_START_ADV(act_idx));
        } break;
        case ATM_ADV_ON: {
            ASSERT_INFO(status == BLE_ERR_NO_ERROR, act_idx, status);
            if(act_to_idx(act_idx) == IDX_LUNCH) {
                // Lunch beacon confirmation (can start now)
                atm_asm_move(S_TBL_IDX, OP_CREATE_LUNCH_CFM);
            } else {
                // Pairing mode confirmation (can start now)
                atm_asm_move(S_TBL_IDX, OP_CREATE_PAIR_CFM);
            }
        } break;
        case ATM_ADV_OFF: {
            atm_asm_move(S_TBL_IDX, OP_ADV_TIMEOUT);
        } break;
        case ATM_ADV_IDLE:
        case ATM_ADV_DELETED:
        default: {
            ATM_LOG(E, "Unhandled state = %d", state);
        } break;
    }

    if(ret != BLE_ERR_NO_ERROR) {
        ATM_LOG(E, "%s: Failed with status: %#x", __func__, ret);
    }
}

/*
 * STATE MACHINES
 *******************************************************************************
 */

/*
 * @brief Fetches advertisment parameters and triggers GAP initialization.
 * Results in a state machine transition from S_INIT -> S_IDLE
 * @note Called upon app initialization
 */
static void ble_init(void)
{
    // Create gatt profile
    ATM_LOG(W, "TODO: Create Gatt Profile (in another file)%s", "");
    //atm_gap_prf_reg(BLE_ATMPRFS_MODULE_NAME, NULL)

    // Start gap
    atm_gap_start(atm_gap_param_get(), &gap_callbacks);
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTING -> S_ADV_STARTED
 * @note Called once the advertisement has been created and started
 */
static void ble_start_on(void)
{
    atm_asm_set_state_op(S_TBL_IDX, S_ADV_STARTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_ADV_STARTED -> S_CONNECTED
 * or once the advertisement has stopped in the S_CONNECTED state
 * @note Called upon a connection establishment
 */
static void ble_connected(void)
{
    atm_asm_set_state_op(S_TBL_IDX, S_CONNECTED, OP_END);
}

/*
 * @brief Triggers a state machine transition from S_CONNECTED -> S_ADV_STOPPED
 * @note Called when the device has been disconnected
 */
static void ble_disconnected(void)
{
    atm_asm_move(S_TBL_IDX, OP_RESTART_ADV);
}

/*
 * @brief Triggers a state machinetransition from S_ADV_STARTED -> S_ADV_STOPPED
 * @note Called when the advertisement has stopped in the S_ADV_STARTED state
 */
static void ble_timeout(void)
{
    atm_pm_unlock(adv_lock_hiber);

    // TODO: this is where I'd set a sw_timer to restart
}

/*
 * @brief Triggers starting of the advertisement. Resulting in a state machine
 * transition from S_ADV_STOPPED -> S_ADV_STARTING
 * @note Called when the advertisement restart timer has expired in the
 * S_ADV_STOPPED state or the device gets disconnected after connection
 */
static void ble_restart_adv(void)
{
    ble_err_code_t ret = atm_adv_start(activity_idx, GET_START_ADV(activity_idx));
    if (ret != BLE_ERR_NO_ERROR) {
	    ATM_LOG(E, "%s: Failed to restart adv with status %#x", __func__, ret);
    }
}

static void ble_create_lunch_adv(void)
{
    ATM_LOG(D, "Creating lunch adv%s","");

    // Fetch params
    app_env.create[IDX_LUNCH] = atm_adv_create_param_get(IDX_LUNCH);
    app_env.start[IDX_LUNCH] = atm_adv_start_param_get(IDX_LUNCH);

    // TODO: Fetch 950 number here from nvds
    ATM_LOG(D, "Fetch 950 number here%s","");
    ATM_LOG(D, "Act idx: %d", app_env.act_idx[IDX_LUNCH]);

    if(app_env.act_idx[IDX_LUNCH] != ATM_INVALID_ACTIDX) {
        atm_adv_start(app_env.act_idx[IDX_LUNCH], app_env.start[IDX_LUNCH]);
    } else {
        app_env.create_adv_idx = LUNCH_ADV_TYPE;
        atm_adv_create(app_env.create[IDX_LUNCH]); // adv_state_change (ATM_ADV_CREATED)
    }
}

static void ble_create_pair_adv(void)
{
    // Fetch params
    app_env.create[IDX_PAIR_ADV] = atm_adv_create_param_get(IDX_PAIR_ADV);
    app_env.start[IDX_PAIR_ADV] = atm_adv_start_param_get(IDX_PAIR_ADV);

    app_env.create_adv_idx = PAIR_ADV_TYPE;
    atm_adv_create(app_env.create[IDX_PAIR_ADV]); // adv_state_change (ATM_ADV_CREATED)
}

static void ble_delete_pair_adv(void)
{
    ATM_LOG(D, "Delete Pairing ADV%s", "");
    atm_adv_delete(app_env.act_idx[IDX_PAIR_ADV]);
}

// Once we create it and call the cfm
// the ASM in atm_adv will callback with ADV_ON State or whatever

// S_OP(state, operation), next_state, handler
// When we are in STATE, and then receive OPERATION, move to NEXT_STATE and call handler
static const state_entry s_tbl[] = {
    // Initialize module
    {S_OP(S_INIT, OP_MODULE_INIT), S_IDLE, ble_init},
    // Create lunch beacon
    {S_OP(S_IDLE, OP_CREATE_LUNCH_ADV), S_STARTING_LUNCH_ADV, ble_create_lunch_adv},
    // Create pairing beacon
    {S_OP(S_IDLE, OP_CREATE_PAIR_ADV), S_STARTING_PAIR_ADV, ble_create_pair_adv},
    // Delete the pairing beacon
    {S_OP(S_STARTING_PAIR_ADV, OP_DELETE_PAIR_ADV), S_IDLE, ble_delete_pair_adv},
    // Start the lunch beacon after receiving confirmation
    {S_OP(S_STARTING_LUNCH_ADV, OP_CREATE_LUNCH_CFM), S_ADV_STARTED, ble_start_on},
    // Start the pairing beacon after receiving confirmation
    {S_OP(S_STARTING_PAIR_ADV, OP_CREATE_PAIR_CFM), S_ADV_STARTED, ble_start_on},
    
    // Handle connections, timeouts, restarts
    {S_OP(S_ADV_STARTED, OP_CONNECTED), S_CONNECTED, ble_connected},
    {S_OP(S_CONNECTED, OP_DISCONNECTED), S_ADV_STOPPED, ble_disconnected},
    {S_OP(S_CONNECTED, OP_ADV_TIMEOUT), S_CONNECTED, ble_connected},
    {S_OP(S_ADV_STARTED, OP_ADV_TIMEOUT), S_ADV_STOPPED, ble_timeout},
    {S_OP(S_ADV_STOPPED, OP_RESTART_ADV), S_STARTING_LUNCH_ADV, ble_restart_adv}
};


/*
 * @brief Starts the advertisement
 * @note Scheduled when the advertisement has stopped in the S_ADV_STARTED state
 */
// static void restart_timer(uint8_t idx, const void *ctx)
// {
//     atm_asm_move(S_TBL_IDX, OP_RESTART_ADV);
// }

static rep_vec_err_t user_appm_init(void)
{
    // Prevent application from hibernating
    adv_lock_hiber = atm_pm_alloc(PM_LOCK_HIBERNATE);
    atm_pm_lock(adv_lock_hiber);

    atm_asm_init_table(S_TBL_IDX, s_tbl, ARRAY_LEN(s_tbl));
    atm_asm_set_state_op(S_TBL_IDX, S_INIT, OP_END);
    atm_asm_move(S_TBL_IDX, OP_MODULE_INIT);

    return RV_DONE;
}

int main(void) 
{
    RV_APPM_INIT_ADD_LAST(user_appm_init);

    ATM_LOG(D, "user_main() done%s", "");
    return 0;
}