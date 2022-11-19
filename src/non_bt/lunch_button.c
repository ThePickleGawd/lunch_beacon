/**
 *******************************************************************************
 *
 * @file lunch_button.c
 *
 * @brief KEY button handler
 *
 * Copyright (C) Atmosic 2022
 *
 *******************************************************************************
 */
#include <inttypes.h>
#include "atm_log.h"
#include "atm_button.h"
#include "atm_vkey.h"
#include "atm_utils_c.h"
#include "lunch_button.h"
#include "atm_pm.h"
#include "pinmux.h"

ATM_LOG_LOCAL_SETTING("lunch_button", V);

// Button Configuration
#define BTN_TAP_HOLD_TIME_MS 0
#define BTN_TAP_HOLD_MAX_TIME_MS 200
#define BTN_LONG_PRESS_MIN_TIME_MS 2000

static key_event_cb event_cb;
static pm_lock_id_t vk_lock_hiber;

static void btn_tap_handler(atm_vk_ck_evt_t const *evt, void const *ctx)
{
    ATM_LOG(V, "%s: key %d tap %d ms", __func__, evt->top.vkey, evt->time_ms);
    if (event_cb) {
	event_cb(evt->top.vkey, LUNCH_BTN_TAP);
    }
}

static bool btn_press_handler(atm_vk_hd_evt_t const *evt, void const *ctx)
{
    ATM_LOG(V, "%s: key_mask %#" PRIx32 ", press %d ms", __func__,
	evt->held.mask[0], evt->time_ms);
    for (uint8_t vkey = 0; vkey < KEY_NUM; vkey++) {
	if ((evt->held.mask[0] & (1 << vkey)) && event_cb) {
	    event_cb(vkey, LUNCH_BTN_PRESS);
	}
    }

    return false;
}

static void hold_key_status_ind(bool pressed, void const *ctx)
{
    if (pressed) {
	atm_pm_lock(vk_lock_hiber);
    } else {
	atm_pm_unlock(vk_lock_hiber);
    }
}

static void button_cb(bool is_press, uint8_t idx, void const *ctx)
{
    struct vkll_ctx_s *handle = CONTEXT_VOID_P(ctx);
    atm_vkey_feed(handle, idx, is_press);
}

void lunch_btn_init(key_event_cb cb)
{
    static atm_vk_reg_t const vkeys[] = {
	VKEY_CLICK(btn_tap_handler, LUNCH_BTN1, BTN_TAP_HOLD_TIME_MS,
	    BTN_TAP_HOLD_MAX_TIME_MS),
        VKEY_HOLD(btn_press_handler, hold_key_status_ind,
	    BTN_LONG_PRESS_MIN_TIME_MS, (1 << ATTS_BTN1))
    };

    static uint8_t const key_gpios[KEY_NUM] = {
	PIN_BUTTON1_IO
    };
    atm_button_set_gpio(KEY_NUM, key_gpios);
    atm_button_reg(button_cb,
	atm_vkey_add_table(vkeys, ARRAY_LEN(vkeys), NULL));
    if (cb) {
	event_cb = cb;
    }
    vk_lock_hiber = atm_pm_alloc(PM_LOCK_HIBERNATE);
    ATM_LOG(V, "Button init!!");
}

