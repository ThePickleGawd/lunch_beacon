/**
 *******************************************************************************
 *
 * @file lunch_led.c
 *
 * @brief LunchTrak LED Manager
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */

#include "arch.h"
#include <inttypes.h>
#include "led_blink.h"
#include "lunch_led.h"

typedef struct {
    uint16_t hi_dur; // Unit: 10ms
    uint16_t low_dur; // Unit: 10ms
    uint16_t times;
} lunch_led_blink_profile_t;

static lunch_led_blink_profile_t lunch_led_profiles[3] = {
    { .hi_dur = 10, .low_dur = 10, .times = 0, }, // OFF
    { .hi_dur = 10, .low_dur = 10, .times = 0, }, // ACTIVE
    { .hi_dur = 10, .low_dur = 10, .times = 3, }, // PAIRING
};

void lunch_led_blink(LUNCH_LED_STATE state) {
    led_blink(LED_0, 
        lunch_led_profiles[state].hi_dur,
        lunch_led_profiles[state].low_dur,
        lunch_led_profiles[state].times);
}

void lunch_led_off(void) {
    led_off(LED_0);
}