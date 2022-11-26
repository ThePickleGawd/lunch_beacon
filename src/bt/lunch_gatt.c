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
#include "lunch_gatt.h"
#include "atm_debug.h"
#include "atm_log.h"
#include <stdio.h>
#include <string.h>
#include "co_list.h"

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

static uint8_t atts_read_req(uint8_t conidx, uint8_t att_idx)
{

}

static uint8_t atts_write_req(uint8_t conidx, uint8_t att_idx)
{

}

static uint8_t atts_write_cfm(uint8_t conidx, uint8_t att_idx)
{

}

ble_atmprfs_cbs_t const atmprfs_cbs = {
	.read_req = atts_read_req,
	.write_req = atts_write_req,
	.write_cfm = atts_write_cfm,
};


/**
 * @brief Create GATT services and charateristics and register callbacks.
 * @note Called upon app initialization
 */
void atts_create_prf(void)
{
	ATM_LOG(D, "%s: ", __func__);

	// Construct UUID Arrays
	uint8_t svc_lunch_uuid[ATT_UUID_128_LEN] = {SVC_LUNCH_UUID};
	uint8_t char_student_id_uuid[ATT_UUID_128_LEN] = {CHAR_STUDENT_ID_UUID};
	uint8_t char_school_id_uuid[ATT_UUID_128_LEN] = {CHAR_SCHOOL_ID_UUID};

	// Register service and characteristics
	atts_attr_handle[ATTS_SVC_LUNCH] = ble_atmprfs_add_svc(svc_lunch_uuid, 
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs);
	atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID] = ble_atmprfs_add_char(char_school_id_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);
	atts_attr_handle[ATTS_CHAR_RW_STUDENT_ID] = ble_atmprfs_add_char(char_school_id_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);
}
