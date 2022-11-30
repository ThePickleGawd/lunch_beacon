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

#include <inttypes.h>

#define NVDS_TAG_LUNCH_DATA 0xD0

typedef struct {
    uint8_t school_id[6];
    uint8_t student_id[10];
} nvds_lunch_data_t;

/**
 * @brief Get lunch data from nvds tag
 * @returns NVDS_OK on success
*/
uint8_t nvds_get_lunch_data(nvds_lunch_data_t *out);

uint8_t nvds_put_lunch_data(nvds_lunch_data_t *data);