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

#define PIN_BUTTON1_IO 10

/// KEY action define
typedef enum {
    LUNCH_BTN_TAP,
    LUNCH_BTN_PRESS,
} lunch_button_action_t;

/// Button define
typedef enum {
    LUNCH_BTN1,
    KEY_NUM
} lunch_button_idx_t;

typedef void (*key_event_cb)(uint8_t idx, lunch_button_action_t key_action);

/**
 *******************************************************************************
 * @brief Initial Button
 *
 * @param[in] cb  Callback function for key enevt (NULL for no callback)
 *******************************************************************************
 */
void atts_btn_init(key_event_cb cb);
