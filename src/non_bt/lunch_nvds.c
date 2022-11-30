/**
 *******************************************************************************
 *
 * @file lunch_nvds.c
 *
 * @brief NVDS tag access functions
 *
 * Copyright (C) Atmosic 2022
 *
 *******************************************************************************
 */

#include "lunch_nvds.h"
#include "nvds.h"
#include "nvds_tag.h"
#include <inttypes.h>
#include "atm_log.h"

ATM_LOG_LOCAL_SETTING("lunch_nvds", D);

uint8_t nvds_get_lunch_data(nvds_lunch_data_t *out)
{
    nvds_tag_len_t len = sizeof(nvds_lunch_data_t);
    uint8_t err = nvds_get(NVDS_TAG_LUNCH_DATA, &len, (uint8_t *) out);
    if(err != NVDS_OK) ATM_LOG(E, "%s - err = %d", __func__, err);

    return err;
}

uint8_t nvds_put_lunch_data(nvds_lunch_data_t *data)
{
    nvds_tag_len_t len = sizeof(nvds_lunch_data_t);
    uint8_t err = nvds_put(NVDS_TAG_LUNCH_DATA, len, (uint8_t *) data);
    if(err != NVDS_OK) ATM_LOG(E, "%s - err = %d", __func__, err);

    return err;
}