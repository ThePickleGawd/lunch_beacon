// C Stuff
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// Drivers
#include "arch.h"
#include "nvds.h"
#include "nvds_tag.h"
#include "co_error.h"
#include "flash.h"
#include "ext_flash.h"
#include "atm_gpio.h"
#include "atm_log.h"
#include "atm_asm.h"
#include "atm_pm.h"

ATM_LOG_LOCAL_SETTING("Lunch Beacon", D);

static rep_vec_err_t user_appm_init(void)
{

}

int main(void) 
{
    RV_APPM_INIT_ADD_LAST(user_appm_init);

    ATM_LOG(D, "user_main() done");
    return 0;
}