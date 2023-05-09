/**
 *******************************************************************************
 *
 * @file lunch_nvds.c
 *
 * @brief NVDS tag access functions
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */

#include "lunch_nvds.h"
#include "nvds.h"
#include "nvds_tag.h"
#include <inttypes.h>
#include "atm_log.h"
#include "co_utils.h"

ATM_LOG_LOCAL_SETTING("lunch_nvds", V);

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

    nvds_print_lunch_data();

    return err;
}

uint8_t nvds_put_school_data(uint8_t const *school_data)
{
    // Get current lunch data
    nvds_lunch_data_t data = {0};
    nvds_get_lunch_data(&data);

    // Modify school id
    memcpy(data.school_id, school_data, SCHOOL_ID_ARR_LEN);
    data.school_id[SCHOOL_ID_ARR_LEN - 1] = 0;

    // Write back to nvds
    return nvds_put_lunch_data(&data);
}

uint8_t nvds_put_student_data(uint8_t const *student_data)
{
    // Get current lunch data
    nvds_lunch_data_t data = {0};
    nvds_get_lunch_data(&data);

    // Modify student id
    memcpy(data.student_id, student_data, STUDENT_ID_ARR_LEN - 1);
    data.student_id[STUDENT_ID_ARR_LEN - 1] = 0;

    // Write back to nvds
    return nvds_put_lunch_data(&data);
}

void nvds_print_lunch_data(void)
{
    nvds_lunch_data_t data = {0};
    nvds_get_lunch_data(&data);

    ATM_LOG(D, "=============================");
    ATM_LOG(D, "| School Data %s", data.school_id);
    ATM_LOG(D, "| Student Data %s", data.student_id);
    ATM_LOG(D, "==============================");
}
