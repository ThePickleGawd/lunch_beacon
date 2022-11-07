#define GPIO_MODE_0 1

static void setup(void) 
{
    // Setup the dip-switches
    atm_gpio_setup(GPIO_MODE_0);
    atm_gpio_set_input(GPIO_MODE_0);
    atm_gpio_set_pullup(GPIO_MODE_0);

    // Check dip-switch to see which application should run
    bool mode0 = atm_gpio_read_gpio(GPIO_MODE_0);

    // Remove pull up and clear input to avoid power drain
    atm_gpio_clear_pullup(GPIO_MODE_0);
    atm_gpio_clear_input(GPIO_MODE_0);

    // Start program based on mode
    //if(mode0) RV_APPM_INIT_ADD_LAST(init_sensor);
}

int main(void) {
    setup();

    return 0;
}