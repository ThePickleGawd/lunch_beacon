/**
 *******************************************************************************
 *
 * @file atts_gatt.c
 *
 * @brief BLE ATT Server Example Source
 *
 * Copyright (C) Atmosic 2020-2021
 *
 *******************************************************************************
 */
#include "arch.h"
#include "atm_gap.h"
#include "atts_gatt.h"
#include "atm_debug.h"
#include "atm_log.h"
#include <stdio.h>
#include <string.h>
#include "co_list.h"

#ifdef GAP_PARM_NAME
#include STR(GAP_PARM_NAME)
#endif // GAP_PARM_NAME
#include "BLE_att_server.h"

ATM_LOG_LOCAL_SETTING("atts_gatt", V);


#define ATTS_RW_SEC_PROPERTY \
    (BLE_ATT_READ_NO_SECURITY | BLE_ATT_WRITE_REQ_NO_SECURITY)

#define ATTS_NT_SEC_PROPERTY \
    (BLE_ATT_NTF_NO_SECURITY | BLE_ATT_READ_NO_SECURITY)

#define ATTS_IND_SEC_PROPERTY \
    (BLE_ATT_IND_NO_SECURITY | BLE_ATT_READ_NO_SECURITY)

#define ATTS_USER_DESC_SIZE 16
#define ATTS_ECHO_BUF_SIZE 16
#define NTF_CNT_STR_OFFSET 10
#define NTF_CNT_STR_LEN 7
#define NTF_IND_STR_LEN (NTF_CNT_STR_OFFSET + NTF_CNT_STR_LEN)
#define ATM_ATT_STR {'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T'}

static uint8_t atts_attr_handle[ATTS_ATTR_NUM];
static uint16_t nt_cccd; // BLE_ATT_CCCD_STOP_NTF;
static uint16_t ind_cccd = BLE_ATT_CCCD_STOP_IND;
static uint16_t ntf_count;

static uint8_t atts_user_desc[ATTS_USER_DESC_SIZE] = {"Atmosic"};
static uint8_t atts_user_dfd_desc[ATTS_USER_DESC_SIZE] = {"User Defined"};
static uint16_t echo_nt_cccd; // BLE_ATT_CCCD_STOP_NTF;
static uint8_t echo_rw_buff[ATTS_ECHO_BUF_SIZE];

#define STR_OPEN_DOOR "open door"
#define STR_CLOSE_DOOR "close door"
#ifdef CFG_ENABLE_SVC_CHG
#define STR_UNLOCK_SERVICE "unlock service"
#endif

STATIC_ASSERT(sizeof(STR_OPEN_DOOR) <= ATTS_ECHO_BUF_SIZE,
    "ATTS_ECHO_BUF_SIZE too small for STR_OPEN_DOOR!");

STATIC_ASSERT(sizeof(STR_CLOSE_DOOR) <= ATTS_ECHO_BUF_SIZE,
    "ATTS_ECHO_BUF_SIZE too small for STR_CLOSE_DOOR!");

typedef struct {
    bool valid;
    uint8_t conidx;
} att_server_status_t;

static att_server_status_t atts_status[ATT_SERVER_LINK_MAX];

/**
 * @brief Get new status entry for attribute server
 */
static uint8_t atts_gatt_get_empty_status(void)
{
    for (uint8_t link_idx = 0; link_idx < ATT_SERVER_LINK_MAX; link_idx++) {
	if (!atts_status[link_idx].valid) {
	    return link_idx;
	}
    }
    return ATT_SERVER_LINK_MAX;
}

/**
 * @brief Get status entry index by connection index
 */
static uint8_t atts_gatt_get_index_by_conidx(uint8_t conidx)
{
    for (uint8_t link_idx = 0; link_idx < ATT_SERVER_LINK_MAX; link_idx++) {
	if (atts_status[link_idx].valid &&
	    (atts_status[link_idx].conidx == conidx)) {
	    return link_idx;
	}
    }
    return ATT_SERVER_LINK_MAX;
}

/**
 * @brief Update att server connect status
 */
void atts_gatt_update_status(uint8_t conidx)
{
    uint8_t link_idx = atts_gatt_get_empty_status();

    if (link_idx == ATT_SERVER_LINK_MAX) {
	ATM_LOG(E, "%s: conidx:%d, link_idx:%d", __func__, conidx, link_idx);
	return;
    }

    atts_status[link_idx].conidx = conidx;
    atts_status[link_idx].valid = true;
}

/**
 * @brief Clear att server connect status
 */
void atts_gatt_clean_status(uint8_t conidx)
{
    uint8_t sts_idx = atts_gatt_get_index_by_conidx(conidx);

    if (sts_idx == ATT_SERVER_LINK_MAX) {
	ATM_LOG(E, "%s: conidx:%d, link_idx:%d", __func__, conidx, sts_idx);
	return;
    }

    memset(&atts_status[sts_idx], 0, sizeof(att_server_status_t));
}

/**
 * @brief Get current connected link amount as client role
 */
uint8_t atts_gatt_connected_link_amount(void)
{
    uint8_t conected_num = 0;

    for (uint8_t link_idx = 0; link_idx < ATT_SERVER_LINK_MAX; link_idx++) {
	if (atts_status[link_idx].valid) {
	    conected_num++;
	}
    }
    return conected_num;
}

typedef struct {
    co_list_hdr_t hdr;
    bool is_ntf;
    uint8_t conidx;
    uint8_t att_idx;
    uint16_t length;
    uint8_t data;
} att_ni_send_req_t;

static co_list_t atts_ble_ni_send_list;

/**
 * @brief Callback passed in ble_atmprfs_gattc_send_ntf or
 * ble_atmprfs_gattc_send_ind.
 * @note Called when the GATT notification or indication is sent.
 */
static void atts_ble_ni_sent(uint8_t conidx,
    ble_gattc_cmp_evt_ex_t const *parm, void const *ctx)
{
    if (parm->operation == GATTC_NOTIFY) {
	ATM_LOG(D, "%s: GATTC_NOTIFY, conidx:%d", __func__, conidx);
    } else {
	ATM_LOG(D, "%s: GATTC_INDICATE, conidx:%d", __func__, conidx);
    }

    att_ni_send_req_t *req = (att_ni_send_req_t *)co_list_pick(
	&atts_ble_ni_send_list);

    if (!req) {
	return;
    }

    ATM_LOG(D, "Send next ind/ntf-ConIdx(%d), att_idx(%#x)", req->conidx,
	req->att_idx);
    if (req->is_ntf) {
	ble_atmprfs_gattc_send_ntf(req->conidx, req->att_idx, &req->data,
	    req->length, atts_ble_ni_sent);
    } else {
	ble_atmprfs_gattc_send_ind(req->conidx, req->att_idx, &req->data,
	    req->length, atts_ble_ni_sent);
    }
    co_list_extract(&atts_ble_ni_send_list, &req->hdr);
    free(req);
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when read attribute request is received in ATTS_SVC_2,
 * ATTS_SVC_3 or ATTS_SVC_ECHO.
 */
static uint8_t atts_read_req(uint8_t conidx, uint8_t att_idx)
{
    ATM_LOG(D, "%s: att_idx (%d)", __func__, att_idx);
    uint8_t def[] = {'d','e','f','a','u','l','t'};
    uint8_t *cfm = NULL;
    uint16_t cfm_size = 0;

    if (att_idx == atts_attr_handle[ATTS_CHAR_RW_3]) {
	cfm_size = sizeof(def);
	cfm = def;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_IND_CCCD_2]) {
	cfm_size = sizeof(ind_cccd);
	cfm = (uint8_t *)&ind_cccd;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_IND_2]) {
	cfm_size = sizeof(ind_cccd);
	cfm = (uint8_t *)&ind_cccd;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_CUDD_3]) {
	cfm_size = sizeof(atts_user_desc);
	cfm = atts_user_desc;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_4]) {
	cfm_size = sizeof(def);
	cfm = def;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_UDFD_4]) {
	cfm_size = sizeof(atts_user_dfd_desc);
	cfm = atts_user_dfd_desc;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_NT_CCCD_ECHO]) {
	cfm_size = sizeof(echo_nt_cccd);
	cfm = (uint8_t *)&echo_nt_cccd;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_ECHO]) {
	cfm_size = sizeof(echo_rw_buff);
	cfm = echo_rw_buff;
    } else {
	ASSERT_INFO(0, conidx, att_idx);
    }

    ble_atmprfs_gattc_read_cfm(conidx, att_idx, cfm, cfm_size);

    return ATT_ERR_NO_ERROR;
}

/**
 * @brief Send notification/indication to all connected link
 */
static void atts_send_ntf_ind_all_links(bool is_ntf, uint8_t att_idx,
    uint8_t const *data, uint16_t length)
{
    for (uint8_t link_idx = 0; link_idx < ATT_SERVER_LINK_MAX; link_idx++) {
	if (!atts_status[link_idx].valid) {
	    continue;
	}

	uint8_t conidx = atts_status[link_idx].conidx;
	uint8_t status;
	if (is_ntf) {
	    status = ble_atmprfs_gattc_send_ntf(conidx, att_idx, data, length,
		atts_ble_ni_sent);
	} else {
	    status = ble_atmprfs_gattc_send_ind(conidx, att_idx, data, length,
		atts_ble_ni_sent);
	}

	if (status != ATT_ERR_INSUFF_RESOURCE) {
	    continue;
	}

	att_ni_send_req_t *send_reg = malloc(sizeof(att_ni_send_req_t) +
	    length - 1);

	ASSERT_ERR(send_reg);

	send_reg->is_ntf = is_ntf;
	send_reg->conidx = conidx;
	send_reg->att_idx = att_idx;
	send_reg->length = length;
	memcpy(&send_reg->data, data, length);
	co_list_push_back(&atts_ble_ni_send_list, &send_reg->hdr);
    }
}

/**
 * @brief Send notification to ATTS_CHAR_NT_1 with the following payload:
 * "Atmosic ATT(XXXX)", XXXX means ntf_count in hex format.
 * @note Called when ATTS_CHAR_NT_CCCD_1 notification is enabled.
 */
static void atts_ble_nt_send(void)
{
    uint8_t tmp[NTF_IND_STR_LEN] = ATM_ATT_STR;

    snprintf((char *)&tmp[NTF_CNT_STR_OFFSET], NTF_CNT_STR_LEN, "(%04X)",
	ntf_count++);

    atts_send_ntf_ind_all_links(true, atts_attr_handle[ATTS_CHAR_NT_1], tmp,
	sizeof(tmp));
}

/**
 * @brief Send indication to ATTS_CHAR_IND_2 with the following payload:
 * "Atmosic ATT(XXXX)", XXXX means ntf_count in hex format.
 * @note Called when ATTS_CHAR_IND_CCCD_2 indication is enabled.
 */
static void atts_ble_ind_send(void)
{
    uint8_t tmp[NTF_IND_STR_LEN] = ATM_ATT_STR;

    snprintf((char *)&tmp[NTF_CNT_STR_OFFSET], NTF_CNT_STR_LEN, "(%04X)",
	ntf_count++);
    atts_send_ntf_ind_all_links(false, atts_attr_handle[ATTS_CHAR_IND_2], tmp,
	sizeof(tmp));
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute request is received in ATTS_SVC_2,
 * ATTS_SVC_3 or ATTS_SVC_ECHO.
 */
static uint8_t atts_write_req(uint8_t conidx, uint8_t att_idx,
    uint8_t const *data, uint16_t len)
{
    ATM_LOG(D, "%s: conidx(%d) att_idx (%d)", __func__, conidx, att_idx);
    if ((att_idx == atts_attr_handle[ATTS_CHAR_RW_3]) || (att_idx ==
	atts_attr_handle[ATTS_CHAR_RW_4])) {
	ATM_LOG(D, "%s: size (%d)", __func__, len);
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_IND_CCCD_2]) {
	if (len != sizeof(ind_cccd)) {
	    return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}
	memcpy(&ind_cccd, data, len);
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_CUDD_3]) {
	if (len != sizeof(atts_user_desc)) {
	    return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}
	memcpy(atts_user_desc, data, len);
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_UDFD_4]) {
#ifndef CFG_ENABLE_SVC_CHG
	if (len != sizeof(atts_user_dfd_desc)) {
	    return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}
	memcpy(atts_user_dfd_desc, data, len);
#else
	uint16_t wr_len = (len > sizeof(atts_user_dfd_desc)) ?
	    sizeof(atts_user_dfd_desc) : len;
	memcpy(atts_user_dfd_desc, data, wr_len);
	memset(atts_user_dfd_desc + wr_len, 0,
	     sizeof(atts_user_dfd_desc) - wr_len);
	if (!memcmp(atts_user_dfd_desc, STR_UNLOCK_SERVICE,
	     sizeof(STR_UNLOCK_SERVICE))) {
	    ATM_LOG(D, " - Get STR_UNLOCK_SERVICE");
	    atm_debug_header_color(ATM_BG_GREEN, "UNLOCK",
		"ECHO service unlocked");
	    ble_atmprfs_svc_visibility_set(atts_attr_handle[ATTS_SVC_ECHO],
		true);
	    ble_gattc_svc_changed(conidx, ble_atmprfs_get_att_handle(
		atts_attr_handle[ATTS_SVC_ECHO]), ble_atmprfs_get_att_handle(
		atts_attr_handle[ATTS_CHAR_NT_CCCD_ECHO]), NULL);
	}
#endif
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_RW_ECHO]) {
	uint16_t wr_len = (len > ATTS_ECHO_BUF_SIZE) ? ATTS_ECHO_BUF_SIZE : len;
	memcpy(echo_rw_buff, data, wr_len);
	memset(echo_rw_buff + wr_len, 0, ATTS_ECHO_BUF_SIZE - wr_len);
	// handle echo write command
	if (!memcmp(echo_rw_buff, STR_OPEN_DOOR, sizeof(STR_OPEN_DOOR))) {
	    ATM_LOG(D, " - Get STR_OPEN_DOOR");
	    atm_debug_header_color(ATM_BG_GREEN, "OPEN", "Open Door");
	    ATM_LOG(D, "[==========]");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "|          |");
	    ATM_LOG(D, "[==========]");
	} else if (!memcmp(echo_rw_buff, STR_CLOSE_DOOR, sizeof(STR_CLOSE_DOOR))) {
	    ATM_LOG(D, " - Get STR_CLOSE_DOOR");
	    atm_debug_header_color(ATM_BG_BLUE, "CLOSE", "Close Door");
	    ATM_LOG(D, "[==========]");
	    ATM_LOG(D, "|x        x|");
	    ATM_LOG(D, "| x      x |");
	    ATM_LOG(D, "|  x    x  |");
	    ATM_LOG(D, "|   x  x   |");
	    ATM_LOG(D, "|    xx    |");
	    ATM_LOG(D, "|    xx    |");
	    ATM_LOG(D, "|   x  x   |");
	    ATM_LOG(D, "|  x    x  |");
	    ATM_LOG(D, "| x      x |");
	    ATM_LOG(D, "|x        x|");
	    ATM_LOG(D, "[==========]");
	} else {
	    ATM_LOG(D, " - Get Unknown command");
		echo_rw_buff[15] = '\0';
		ATM_LOG(D, "%s", (char *)echo_rw_buff);
	}
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_NT_CCCD_ECHO]) {
	if (len != sizeof(echo_nt_cccd)) {
	    return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}
	memcpy(&echo_nt_cccd, data, len);
    } else {
	ASSERT_INFO(0, conidx, att_idx);
    }
    return ATT_ERR_NO_ERROR;
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when read attribute request is received in ATTS_SVC_1.
 */
static uint8_t atts_read_req_1(uint8_t conidx, uint8_t att_idx)
{
    ATM_LOG(D, "%s: att_idx (%d)", __func__, att_idx);
    uint8_t def[] = {'d','e','f','a','u','l','t','-','1'};
    uint8_t *cfm = NULL;
    uint16_t cfm_size = 0;

    if (att_idx == atts_attr_handle[ATTS_CHAR_RW_1]) {
	cfm_size = sizeof(def);
	cfm = def;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_NT_CCCD_1]) {
	cfm_size = sizeof(nt_cccd);
	cfm = (uint8_t *)&nt_cccd;
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_NT_1]) {
	cfm_size = sizeof(nt_cccd);
	cfm = (uint8_t *)&nt_cccd;
    } else {
	ASSERT_INFO(0, conidx, att_idx);
    }

    ble_atmprfs_gattc_read_cfm(conidx, att_idx, cfm, cfm_size);

    return ATT_ERR_NO_ERROR;
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute request is received in ATTS_SVC_1.
 */
static uint8_t atts_write_req_1(uint8_t conidx, uint8_t att_idx,
    uint8_t const *data, uint16_t len)
{
    ATM_LOG(D, "%s: att_idx (%d)", __func__, att_idx);

    if (att_idx == atts_attr_handle[ATTS_CHAR_RW_1]) {
	ATM_LOG(D, "%s: size (%d)", __func__, len);
    } else if (att_idx == atts_attr_handle[ATTS_CHAR_NT_CCCD_1]) {
	if (len != sizeof(nt_cccd)) {
	    return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}
	memcpy(&nt_cccd, data, len);
    } else {
	ASSERT_INFO(0, conidx, att_idx);
    }

    return ATT_ERR_NO_ERROR;
}

/**
 * @brief Callback registered with the ble_atmprfs module.
 * @note Called when write attribute confirmation/response (if needed) is sent
 */
static void atts_write_cfm(uint8_t conidx, uint8_t att_idx)
{
    ATM_LOG(D, "%s: att_idx (%d)", __func__, att_idx);
    if (att_idx == atts_attr_handle[ATTS_CHAR_NT_CCCD_1]) {
	if (nt_cccd & BLE_ATT_CCCD_NTF) {
	    atts_ble_nt_send();
	}
	return;
    }

    if (att_idx == atts_attr_handle[ATTS_CHAR_IND_CCCD_2]) {
	if (ind_cccd & BLE_ATT_CCCD_IND) {
	    atts_ble_ind_send();
	}
	return;
    }

    if (att_idx == atts_attr_handle[ATTS_CHAR_RW_ECHO]) {
	if (echo_nt_cccd & BLE_ATT_CCCD_NTF) {
	    ATM_LOG(D, " - Notify is on: echo write-command via notify");
	    atts_send_ntf_ind_all_links(true,
		atts_attr_handle[ATTS_CHAR_NT_ECHO], echo_rw_buff,
		ATTS_ECHO_BUF_SIZE);

	} else {
	    ATM_LOG(D, " - Notify is off");
	}
    }
}

/**
 * @brief ble_atmprfs callbacks for ATTS_SVC_1
 */
ble_atmprfs_cbs_t const atmprfs_cbs_1 = {
    .read_req = atts_read_req_1,
    .write_req = atts_write_req_1,
    .write_cfm = atts_write_cfm,
};

/**
 * @brief ble_atmprfs callbacks for ATTS_SVC_2
 */
ble_atmprfs_cbs_t const atmprfs_cbs_2 = {
    .read_req = atts_read_req,
    .write_req = atts_write_req,
    .write_cfm = atts_write_cfm,
};

/**
 * @brief ble_atmprfs callbacks for ATTS_SVC_3
 */
ble_atmprfs_cbs_t const atmprfs_cbs_3 = {
    .read_req = atts_read_req,
    .write_req = atts_write_req,
};

/**
 * @brief ble_atmprfs callbacks for ATTS_SVC_ECHO
 */
ble_atmprfs_cbs_t const atmprfs_cbs_echo = {
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

    uint8_t atms_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'S', 'e', 'r', 'v',
	 0x00, 0x01};

    uint8_t rw_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'C', 'h', 'a', 'r',
	 'R', 'W'};

    uint8_t nt_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'C', 'h', 'a', 'r',
	 'N', 'T'};

    uint8_t atms_uuid2[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'S', 'e', 'r', 'v',
	 0x00, 0x02};

    uint8_t ind_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'C', 'h', 'a', 'r',
	 'I', 'D'};

    uint8_t atms_uuid3[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'S', 'e', 'r', 'v',
	 0x00, 0x03};

    uint8_t sv3_rw1_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'S', 'V', '3', 'R',
	 'W', '1'};

    uint8_t sv3_rw2_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', 'A', 'T', 'T', 'S', 'V', '3', 'R',
	 'W', '2'};

    uint8_t echo_sv_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', ' ', 'E', 'c', 'h', 'o', 'S', 'V',
	 0x00, 0x04};

    uint8_t echo_rw_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', ' ', 'E', 'c', 'h', 'o', 'C', 'H',
	 'R', 'W'};

    uint8_t echo_nt_uuid[ATT_UUID_128_LEN] =
	{'A', 't', 'm', 'o', 's', 'i', 'c', ' ', 'E', 'c', 'h', 'o', 'C', 'H',
	 'N', 'T'};

    atts_attr_handle[ATTS_SVC_1] = ble_atmprfs_add_svc(atms_uuid,
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs_1);

    atts_attr_handle[ATTS_CHAR_RW_1] = ble_atmprfs_add_char(rw_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);

    atts_attr_handle[ATTS_CHAR_NT_1] = ble_atmprfs_add_char(nt_uuid,
	ATTS_NT_SEC_PROPERTY, ATTS_DATA_SIZE);

    atts_attr_handle[ATTS_CHAR_NT_CCCD_1] = ble_atmprfs_add_client_char_cfg();

    atts_attr_handle[ATTS_SVC_2] = ble_atmprfs_add_svc(atms_uuid2,
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs_2);

    atts_attr_handle[ATTS_CHAR_IND_2] = ble_atmprfs_add_char(ind_uuid,
	ATTS_IND_SEC_PROPERTY, ATTS_DATA_SIZE);

    atts_attr_handle[ATTS_CHAR_IND_CCCD_2] = ble_atmprfs_add_client_char_cfg();

    atts_attr_handle[ATTS_SVC_3] = ble_atmprfs_add_svc(atms_uuid3,
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs_3);

    atts_attr_handle[ATTS_CHAR_RW_3] = ble_atmprfs_add_char(sv3_rw1_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);

    atts_attr_handle[ATTS_CHAR_RW_CUDD_3] = ble_atmprfs_add_char_user_desc(
	ATTS_RW_SEC_PROPERTY, ATTS_USER_DESC_SIZE);

    atts_attr_handle[ATTS_CHAR_RW_4] = ble_atmprfs_add_char(sv3_rw2_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_DATA_SIZE);

    atts_attr_handle[ATTS_CHAR_RW_UDFD_4] = ble_atmprfs_add_desc_user_defined(
	atms_uuid, ATTS_RW_SEC_PROPERTY, ATTS_USER_DESC_SIZE);

    ATM_LOG(D, "%s: svc1 (%d), RW1 (%d), NT1 (%d) NT_CCCD1 (%d)", __func__,
	atts_attr_handle[ATTS_SVC_1], atts_attr_handle[ATTS_CHAR_RW_1],
	atts_attr_handle[ATTS_CHAR_NT_1], atts_attr_handle[ATTS_CHAR_NT_CCCD_1]);

    ATM_LOG(D, "%s: svc2 (%d), IND2 (%d) NT_CCCD2 (%d)", __func__,
	atts_attr_handle[ATTS_SVC_2], atts_attr_handle[ATTS_CHAR_IND_2],
	atts_attr_handle[ATTS_CHAR_IND_CCCD_2]);

    ATM_LOG(D, "%s: svc3 (%d), RW3 (%d), CUDD3 (%d)", __func__,
	atts_attr_handle[ATTS_SVC_3], atts_attr_handle[ATTS_CHAR_RW_3],
	atts_attr_handle[ATTS_CHAR_RW_CUDD_3]);

    // Atmosic Echo
    atts_attr_handle[ATTS_SVC_ECHO] = ble_atmprfs_add_svc(echo_sv_uuid,
	ATTS_SVC_SEC_PROPERTY, &atmprfs_cbs_echo);
    atts_attr_handle[ATTS_CHAR_RW_ECHO] = ble_atmprfs_add_char(echo_rw_uuid,
	ATTS_RW_SEC_PROPERTY, ATTS_ECHO_BUF_SIZE);
    atts_attr_handle[ATTS_CHAR_NT_ECHO] = ble_atmprfs_add_char(echo_nt_uuid,
	BLE_ATT_NTF_NO_SECURITY, ATTS_ECHO_BUF_SIZE);
    atts_attr_handle[ATTS_CHAR_NT_CCCD_ECHO] =
	ble_atmprfs_add_client_char_cfg();

    ATM_LOG(D, "%s: SVC_ECHO(%u), RW_ECHO(%u), NT_ECHO(%u), NT_CCCD_ECHO(%u)",
	__func__, atts_attr_handle[ATTS_SVC_ECHO],
	atts_attr_handle[ATTS_CHAR_RW_ECHO], atts_attr_handle[ATTS_CHAR_NT_ECHO],
	atts_attr_handle[ATTS_CHAR_NT_CCCD_ECHO]);

    co_list_init(&atts_ble_ni_send_list);

}

void atts_init_done(void)
{
#ifdef CFG_ENABLE_SVC_CHG
    // Hide echo service by default
    ble_atmprfs_svc_visibility_set(atts_attr_handle[ATTS_SVC_ECHO], false);
#endif
}
