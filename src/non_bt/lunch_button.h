/**
 *******************************************************************************
 *
 * @file lunch_button.h
 *
 * @brief KEY button Header
 *
 * Copyright (C) Atmosic 2022
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
 * @brief Check if button press was the cause of wakeup
 *******************************************************************************
 */
void lunch_button_check_on_boot(void);
