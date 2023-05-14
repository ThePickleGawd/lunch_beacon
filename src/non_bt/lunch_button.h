/**
 *******************************************************************************
 *
 * @file lunch_button.h
 *
 * @brief KEY button Header
 *
 * Copyright (C) LunchTrak 2023
 *
 *******************************************************************************
 */

#pragma once


typedef void (*press_event_cb)(void);

/**
 *******************************************************************************
 * @brief Initialize Button
 *
 * @param[in] cb  Callback function for press event
 *******************************************************************************
 */
void lunch_button_init(press_event_cb cb);

/**
 *******************************************************************************
 * @brief Check if we are still pressing button when device is awoken
 *******************************************************************************
 */
void lunch_button_on_wake(void);
