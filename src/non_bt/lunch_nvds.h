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

#include <inttypes.h>
#include "arch.h"

#define NVDS_TAG_LUNCH_DATA 0xD0
#define SCHOOL_ID_ARR_LEN 6
#define STUDENT_ID_ARR_LEN 10

/**
 * @brief NVDS Lunch Data type
 * @note The last byte for each array will always be 0 so that we can treat it like a string
 */
typedef struct {
    uint8_t school_id[SCHOOL_ID_ARR_LEN];
    uint8_t student_id[STUDENT_ID_ARR_LEN];
} __PACKED nvds_lunch_data_t;

/**
 * @brief Get lunch data from nvds tag
 * @returns NVDS_OK on success
*/
uint8_t nvds_get_lunch_data(nvds_lunch_data_t *out);

/**
 * @brief Put lunch data into nvds
 * @returns NVDS_OK on success
*/
uint8_t nvds_put_lunch_data(nvds_lunch_data_t *data);

/**
 * @brief Modify only school data and put into nvds
 * @returns NVDS_OK on success
*/
uint8_t nvds_put_school_data(uint8_t const *school_data);

/**
 * @brief Modify only student data and put into nvds
 * @returns NVDS_OK on success
*/
uint8_t nvds_put_student_data(uint8_t const *student_data);

/**
 * @brief Print nvds lunch data
 */
void nvds_print_lunch_data(void);
