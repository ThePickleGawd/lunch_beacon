/**
 *******************************************************************************
 *
 * @file lunch_led.h
 *
 * @brief LunchTrak LED Manager
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */

typedef enum {
    LUNCH_LED_OFF,
    LUNCH_LED_ACTIVE,
    LUNCH_LED_PAIRING,
} LUNCH_LED_STATE;

/**
 * @brief Turns on led in specified state
 * 
 * @param state 
 */
void lunch_led_blink(LUNCH_LED_STATE state);

/**
 * @brief Turns off led
 * 
 */
void lunch_led_off(void);
