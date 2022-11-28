/**
 *******************************************************************************
 *
 * @file lunch_gatt.c
 *
 * @brief BLE ATT Server Example Source
 *
 * Copyright (C) Atmosic 2020-2021
 *
 *******************************************************************************
 */
#include "arch.h"
#include "atm_gap.h"
#include "atm_debug.h"
#include "atm_log.h"
#include <stdio.h>
#include <string.h>
#include "co_list.h"
#include "rwble_hl_error.h"

#ifdef GAP_PARM_NAME
#include STR(GAP_PARM_NAME)
#endif

#include "lunch_gatt.h"

ATM_LOG_LOCAL_SETTING("lunch_gatt", V);

/*
 * VARIABLES
 *******************************************************************************
 */

static uint8_t atts_attr_handle[ATTS_ATTR_NUM];

/*
 * SERVICE CALLBACKS
 *******************************************************************************
 */

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when read attribute request is received
 */
static uint8_t atts_read_req(uint8_t conidx, uint8_t att_idx)
{
    ATM_LOG(D, "%s: att_idx (%d)", __func__, att_idx);
	ATM_LOG(D, "TODO: check which characteristic with atts_attr_handle[]");

	const uint8_t *data = (const uint8_t *)"test read";
	ble_atmprfs_gattc_read_cfm(conidx, att_idx, data, sizeof(data));

	return ATT_ERR_NO_ERROR;
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute request is received
 */
static uint8_t atts_write_req(uint8_t conidx, uint8_t att_idx, uint8_t const *data, uint16_t len)
{
	ATM_LOG(D, "%s: conidx(%d) att_idx (%d)", __func__, conidx, att_idx);
	ATM_LOG(D, "Write request: %s", data);

	return ATT_ERR_NO_ERROR;
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute confirmation/response (if needed) is sent
 */
static void atts_write_cfm(uint8_t conidx, uint8_t att_idx)
{
	ATM_LOG(D, "%s: idk what to do here yet", __func__);
}

ble_atmprfs_cbs_t const atmprfs_cbs = {
	.read_req = atts_read_req,
	.write_req = atts_write_req,
	.write_cfm = atts_write_cfm,
};

/*
 * STATIC FUNCTIONS
 *******************************************************************************
 */


/*
 * GLOBAL FUNCTIONS
 *******************************************************************************
 */

/**
 * @brief Create GATT services and charateristics and register callbacks.
 * @note Called upon app initialization
 */
void lunch_atts_create_prf(void)
{
	ATM_LOG(D, "%s: ", __func__);

	// Construct UUID Arrays
	uint8_t svc_lunch_uuid[ATT_UUID_128_LEN] = {SVC_LUNCH_UUID};
	uint8_t char_student_id_uuid[ATT_UUID_128_LEN] = {CHAR_STUDENT_ID_UUID};
	uint8_t char_school_id_uuid[ATT_UUID_128_LEN] = {CHAR_SCHOOL_ID_UUID};

	// Register lunch service and it's characteristics
	atts_attr_handle[ATTS_SVC_LUNCH] = ble_atmprfs_add_svc(svc_lunch_uuid, 
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs);
	atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID] = ble_atmprfs_add_char(char_student_id_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);
	atts_attr_handle[ATTS_CHAR_RW_SCHOOL_ID] = ble_atmprfs_add_char(char_school_id_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);
	atts_attr_handle[ATTS_CHAR_CCCD] = ble_atmprfs_add_client_char_cfg();

	ATM_LOG(D, "%s: SVC (%d), RW_STUDENT_ID (%d), RW_SCHOOL_ID (%d) CCCD (%d)", __func__,
	atts_attr_handle[ATTS_SVC_LUNCH], atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID],
	atts_attr_handle[ATTS_CHAR_RW_SCHOOL_ID], atts_attr_handle[ATTS_CHAR_CCCD]);
}
