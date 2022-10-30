/**
 *******************************************************************************
 *
 * @file gap_param.h
 *
 * @brief Header file - ATM bluetooth framework GAP parameters
 *
 * Copyright (C) Atmosic 2020-2022
 *
 *******************************************************************************
 */
#include "atm_gap.h"
#include "atm_gap_param_internal.h"

/**
 *******************************************************************************
 * @brief Get persistent GAP start parameter.
 * @return Pointer of GAP start parameter
 *******************************************************************************
 */
atm_gap_param_t __ATM_GAP_PARAM_CONST *get_gap_params(void);
atm_gap_param_t __ATM_GAP_PARAM_CONST *get_gap_params(void)
{
    // Device name
    static uint8_t __ATM_GAP_PARAM_CONST dname[CFG_GAP_DNAME_MAX_LEN] =
	CFG_GAP_DEV_NAME;

    static ble_gap_set_dev_config_t __ATM_GAP_PARAM_CONST default_dev_conf = {
	.role = CFG_GAP_ROLE,
	.pairing_mode = CFG_GAP_PAIRING_MODE,
	.sugg_max_tx_octets = CFG_GAP_MAX_TX_OCTETS,
	.sugg_max_tx_time = CFG_GAP_MAX_TX_TIME,
	.max_mtu = CFG_GAP_MAX_LL_MTU,
	.max_mps = CFG_GAP_MAX_LL_MPS,
	.privacy_cfg = CFG_GAP_PRIVACY_CFG,
	.renew_dur = CFG_RENEW_DURATION,
	.att_cfg = CFG_GAP_ATT_CFG,
	.tx_path_comp = 800, // (80 dBm)
    .rx_path_comp = 800 // (80 dBm)
    };

    static atm_gap_param_t __ATM_GAP_PARAM_CONST default_bt_init_param = {
	.dev_name = dname,
	.dev_name_len = sizeof(CFG_GAP_DEV_NAME),
	.dev_name_max = CFG_GAP_DNAME_MAX_LEN,
	.app_irk = CFG_GAP_APP_IRK,
	.appearance = CFG_GAP_APPEARANCE,
	.periph_pref_params = {
	    .con_intv_min = CFG_GAP_CONN_INT_MIN,
	    .con_intv_max = CFG_GAP_CONN_INT_MAX,
	    .periph_latency = CFG_GAP_PERIPH_LATENCY,
	    .con_timeout = CFG_GAP_CONN_TIMEOUT,
	},
	.dev_config = &default_dev_conf,
	.addr = {
	    .addr[0] = CFG_GAP_OWN_STATIC_RANDOM_ADDR0,
	    .addr[1] = CFG_GAP_OWN_STATIC_RANDOM_ADDR1,
	    .addr[2] = CFG_GAP_OWN_STATIC_RANDOM_ADDR2,
	    .addr[3] = CFG_GAP_OWN_STATIC_RANDOM_ADDR3,
	    .addr[4] = CFG_GAP_OWN_STATIC_RANDOM_ADDR4,
	    .addr[5] = CFG_GAP_OWN_STATIC_RANDOM_ADDR5,
	},
    };
    return &default_bt_init_param;
}