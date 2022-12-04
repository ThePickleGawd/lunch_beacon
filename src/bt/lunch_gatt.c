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
#include "lunch_nvds.h"

ATM_LOG_LOCAL_SETTING("lunch_gatt", V);

/*
 * VARIABLES
 *******************************************************************************
 */

static uint8_t atts_attr_handle[ATTS_ATTR_NUM];

/*
 * DATA PARSER
 *******************************************************************************
 */
static void try_write_student_data(uint8_t const *data, uint8_t len)
{
	if(len > STUDENT_ID_ARR_LEN) {
		ATM_LOG(W, "Cannot write %s to student data, it's too large! (%d > %d)", data, len, STUDENT_ID_ARR_LEN);
		return;
	}

	uint8_t student_id[STUDENT_ID_ARR_LEN] = {0};
	memcpy(student_id, data, len);
	for(int i = len; i < STUDENT_ID_ARR_LEN - 1; i++)
		student_id[i] = 0;

	// TODO: Add some more checks, like ascii characters only?
	nvds_put_student_data(student_id);
}

static void try_write_school_data(uint8_t const *data, uint8_t len)
{
	if(len > SCHOOL_ID_ARR_LEN) {
		ATM_LOG(W, "Cannot write %s to school data, it's too large! (%d > %d)", data, len, SCHOOL_ID_ARR_LEN);
		return;
	}

	uint8_t school_id[SCHOOL_ID_ARR_LEN] = {0};
	memcpy(school_id, data, len);
	for(int i = len; i < SCHOOL_ID_ARR_LEN - 1; i++)
		school_id[i] = 0;

	// TODO: Add some more checks, like ascii characters only?
	nvds_put_student_data(school_id);

}

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

	nvds_lunch_data_t lunch_data = {};
	nvds_get_lunch_data(&lunch_data);
	
	if(att_idx == atts_attr_handle[ATTS_CHAR_RW_SCHOOL_ID]) {
		ATM_LOG(D, "Send read response: %s", lunch_data.school_id);
		ble_atmprfs_gattc_read_cfm(conidx, att_idx, lunch_data.school_id, SCHOOL_ID_ARR_LEN - 1);
	} else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID]) {
		ATM_LOG(D, "Send read response: %s", lunch_data.student_id);
		ble_atmprfs_gattc_read_cfm(conidx, att_idx, lunch_data.student_id, STUDENT_ID_ARR_LEN - 1);
	}

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

	// Try to write data to respective spot
	if(att_idx == atts_attr_handle[ATTS_CHAR_RW_SCHOOL_ID]) {
		try_write_school_data(data, len);
	} else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID]) {
		try_write_student_data(data, len);
	}

	return ATT_ERR_NO_ERROR;
}
/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute confirmation/response (if needed) is sent
 */
static void atts_write_cfm(uint8_t conidx, uint8_t att_idx)
{
	ATM_LOG(D, "%s: idk what to do here yet", __func__);

	// uint8_t status = ble_atmprfs_gattc_send_ntf(conidx, att_idx, (const uint8_t *) "TEST", sizeof("TEST"), NULL);
	// if(status != ATT_ERR_NO_ERROR) {
	// 	ATM_LOG(E, "%s: Error code %d", __func__, status);
	// }
}

ble_atmprfs_cbs_t const atmprfs_cbs = {
	.read_req = atts_read_req,
	.write_req = atts_write_req,
	.write_cfm = atts_write_cfm,
};

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
