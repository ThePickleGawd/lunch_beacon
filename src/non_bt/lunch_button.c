/**
 *******************************************************************************
 *
 * @file lunch_button.c
 *
 * @brief KEY button handler
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */
#include <inttypes.h>
#include "atm_log.h"
#include "atm_button.h"
#include "atm_vkey.h"
#include "atm_utils_c.h"
#include "atm_pm.h"
#include "atm_gpio.h"
#include "sw_timer.h"
#include "interrupt.h"
#include "pinmux.h"

// My stuff
#include "lunch_beacon.h"
#include "lunch_button.h"

ATM_LOG_LOCAL_SETTING("lunch_button", V);

// Button Configuration
#define BTN_PRESS_MIN_TIME_CS 200
#define BTN_CHECK_PRESS_INTERVAL_CS 20
#define MAX_PRESS_CHECKS (BTN_PRESS_MIN_TIME_CS / BTN_CHECK_PRESS_INTERVAL_CS)
#ifndef PIN_BUTTON1_IO
#define PIN_BUTTON1_IO 10
#endif

static press_event_cb event_cb;
static sw_timer_id_t check_press_tid;

int check_press_counter = 0;

static void check_press(sw_timer_id_t timer_id, const void *ctx)
{
    sw_timer_clear(check_press_tid);

    check_press_counter++;

    if(check_press_counter >= MAX_PRESS_CHECKS) {
        event_cb(); 
        check_press_counter = 0;
    } else {
        if(atm_gpio_read_gpio(PIN_BUTTON1_IO) == 1) {
            // Still pressing, keep checking
            sw_timer_set(check_press_tid, BTN_CHECK_PRESS_INTERVAL_CS);
        } else {
            // Stopped pressing, abort
            check_press_counter = 0;
        }
    }
}

__FAST static void interrupt_hdlr(uint32_t mask)
{
    ATM_LOG(D, "Button Clicked");

    atm_gpio_set_int_disable(PIN_BUTTON1_IO);
    atm_gpio_clear_int_status(PIN_BUTTON1_IO);

    atm_gpio_int_set_rising(PIN_BUTTON1_IO);
    atm_gpio_set_int_enable(PIN_BUTTON1_IO);

    sw_timer_set(check_press_tid, BTN_CHECK_PRESS_INTERVAL_CS);
}

void lunch_button_init(press_event_cb cb)
{
    // Register timer and callback
	event_cb = cb;
    check_press_tid = sw_timer_alloc(check_press, NULL);

    // Setup GPIO for button
    atm_gpio_setup(PIN_BUTTON1_IO);
    atm_gpio_set_input(PIN_BUTTON1_IO);

    interrupt_install_gpio(PIN_BUTTON1_IO, 3, interrupt_hdlr);    

    atm_gpio_int_set_rising(PIN_BUTTON1_IO);
    atm_gpio_set_int_enable(PIN_BUTTON1_IO);
}

void lunch_button_on_wake(void) {
    // We could be awaken by a press, so check here
    sw_timer_set(check_press_tid, BTN_CHECK_PRESS_INTERVAL_CS);
}

