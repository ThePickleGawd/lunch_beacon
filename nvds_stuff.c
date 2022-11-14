// Drivers and lib stuff
#include <stdio.h>
#include <string.h>
#include "arch.h"
#include "atm_gpio.h"
#include "nvds.h"
#include "nvds_tag.h"
#include "atm_log.h"
#include "flash.h"
#include "co_error.h"
#include "ext_flash.h"
#include <inttypes.h>

// My stuff
#include "BLE_adv.h"
#include "BLE_att_server.h"

#define GPIO_MODE_0 9
#define GPIO_BUTTON 10
#define NVDS_TAG_STUDENT_DATA 0xD0

ATM_LOG_LOCAL_SETTING("Lunch Beacon", D);

/*
 *   0x10000000  ┌────────────────────┐ 0x0
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░UFLASH░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               ├────────────────────┤FLASH_RECORD_OFFSET
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░Records░░░░░░│(USER_SIZE)
 *               │░░░░░░░░░░░░░░░░░░░░│
 *  __NVDS_Flash ├────────────────────┤FLASH_NVDS_OFFSET
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░NVDS░░░░░░░░│ (NVDS_FLASH_SIZE)
 *               │░░░░░░░░░░░░░░░░░░░░│
 *__NVDS_FlashEnd├────────────────────┤
 *               │░░░░░░░░░░░░░░░░░░░░│
 *               │░░░░░░░░MPR░░░░░░░░░│
 *               │░░░░░░░░░░░░░░░░░░░░│
 *...............└────────────────────┘FLASH_BANK0_SIZE

 *
 */

typedef struct {
    uint8_t student_id[10];
    uint8_t school_id[6];
} nvds_student_data_t;

static uint32_t flash_sector_idx = 0;

static uint8_t nvds_get_student_data(nvds_student_data_t *data)
{
    nvds_tag_len_t len = sizeof(nvds_student_data_t);
    uint8_t err = nvds_get(NVDS_TAG_STUDENT_DATA, &len, (uint8_t *) data);
    if(err != NVDS_OK) ATM_LOG(E, "%s - err = %d", __func__, err);
 
    ATM_LOG(D, "Student ID is %u%u%u%u%u%u%u%u%u%u", 
        data->student_id[0],
        data->student_id[1],
        data->student_id[2],
        data->student_id[3],
        data->student_id[4],
        data->student_id[5],
        data->student_id[6],
        data->student_id[7],
        data->student_id[8],
        data->student_id[9]
    );

    return err;
}

static uint8_t nvds_put_student_data(nvds_student_data_t const *data)
{
    nvds_tag_len_t len = sizeof(nvds_student_data_t);
    uint8_t err = nvds_put(NVDS_TAG_STUDENT_DATA, len, (uint8_t const *) data);
    if (err != NVDS_OK) {
	ATM_LOG(E, "%s - err = %d", __func__, err);
    }

    ATM_LOG(D, "Student ID is %u%u%u%u%u%u%u%u%u%u", 
        data->student_id[0],
        data->student_id[1],
        data->student_id[2],
        data->student_id[3],
        data->student_id[4],
        data->student_id[5],
        data->student_id[6],
        data->student_id[7],
        data->student_id[8],
        data->student_id[9]
    );

    return err;
}

int mode0 = 0;

static void write_nvds(void) {
    nvds_student_data_t data = {
        .student_id = {9,5,0,3,0,4,8,6},
        .school_id = "GUNN"
    };

    nvds_put_student_data(&data);
}

static void read_nvds(void) {
    nvds_student_data_t out;
    nvds_get_student_data(&out);

    ATM_LOG(D, "Reading first num %d", out.student_id[0]);
}

static rep_vec_err_t test_nvds(void)
{
    if(mode0) write_nvds();
    else read_nvds();

    //flash_record_write((uint8_t *) &data, FLASH_SECTOR_SIZE);
    //flash_record_read(0, (uint8_t *) &out2, FLASH_SECTOR_SIZE);

    //ATM_LOG(D, "NVDS first number is %d", out.student_id[0]);
    //ATM_LOG(D, "Flash reading gives me first number %d", out2.student_id[0]);

    return RV_DONE;
}

static void setup(void) 
{
    // Setup the dip-switches
    atm_gpio_setup(GPIO_MODE_0);
    atm_gpio_set_input(GPIO_MODE_0);
    atm_gpio_set_pullup(GPIO_MODE_0);

    // Check dip-switch to see which application should run
    mode0 = atm_gpio_read_gpio(GPIO_MODE_0);

    // Remove pull up and clear input to avoid power drain
    atm_gpio_clear_pullup(GPIO_MODE_0);
    atm_gpio_clear_input(GPIO_MODE_0);

    // Start program based on mode
}

int main(void) {
    setup();
    //RV_APPM_INIT_ADD_LAST(test_nvds);


    return 0;
}