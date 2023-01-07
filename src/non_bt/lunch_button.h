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
 * @brief Initial Button
 *
 * @param[in] cb  Callback function for key event (NULL for no callback)
 *******************************************************************************
 */
void lunch_btn_init(press_event_cb cb);
