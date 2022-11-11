// Drivers and lib stuff
#include <stdio.h>
#include <string.h>
#include "arch.h"
#include "atm_gpio.h"
#include "spi_flash.h"

// My stuff
#include "BLE_adv.h"
#include "BLE_att_server.h"

#define GPIO_MODE_0 9

static void setup(void) 
{
    // Setup the dip-switches
    atm_gpio_setup(GPIO_MODE_0);
    atm_gpio_set_input(GPIO_MODE_0);
    atm_gpio_set_pullup(GPIO_MODE_0);

    // Check dip-switch to see which application should run
    int mode0 = atm_gpio_read_gpio(GPIO_MODE_0);

    // Remove pull up and clear input to avoid power drain
    atm_gpio_clear_pullup(GPIO_MODE_0);
    atm_gpio_clear_input(GPIO_MODE_0);

    // Start program based on mode
    //if(mode0) setup_BLE_adv();
    //else setup_BLE_att_server();


}

int main(void) {
    setup();

    return 0;
}