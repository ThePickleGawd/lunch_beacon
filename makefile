include ../../user/common.mk

BOARD=m2202
DEBUG := 1

DRIVERS := \
	interrupt \
 	timer \
	sw_timer \
	atm_ble \
	atm_pm \
	atm_gpio \
	atm_button \
	atm_vkey \
	sw_event \
	wurx \

LIBRARIES := prf

FRAMEWORK_MODULES := \
	atm_adv \
	atm_asm \
	atm_common \
	atm_debug \
	atm_gap \
	atm_log \
	ble_gap \
	atm_prfs \
	atm_scan \
	ble_atmprfs \
	ble_gap_sec \
	ble_gattc \

UU_TEST := lunch_beacon atm_adv
INCLUDES += .

CFLAGS += \
	-DCFG_NO_GAP_SEC \
	-DCFG_NO_GAP_SCAN \
	-DCFG_NO_GATTC \
	-DPINMAP_$(BOARD)_OVERLAY="pinmap_$(BOARD)_overlay.h" \
	-DCFG_WURX_FROM_FLASH_NVDS \

# Predefined header adv stuff
CFLAGS += \
	-DENABLE_USER_ADV_TIMEOUT \
	-DENABLE_USER_ADV_PARAM_SETTING \
	-DENABLE_USER_ADV_DATA_SCANRSP \
	-DCFG_ADV_DATA_PARAM_CONST=0 \
	-DCFG_GAP_ADV_MAX_INST=2 \
	-DGAP_ADV_PARM_NAME="cfg_adv_params.h" \
	-DGAP_PARM_NAME="cfg_gap_params.h" \

# SRC
SRC_TOP = src
SRC_BT = src/bt
SRC_NON_BT = src/non_bt
INCLUDES += $(SRC_BT) $(SRC_NON_BT) $(SRC_TOP)
C_SRCS += \
	$(SRC_NON_BT)/lunch_button.c \
	$(SRC_NON_BT)/lunch_nvds.c \
	$(SRC_BT)/lunch_gatt.c \

flash_nvds.data := \
	d0-LUNCH_DATA/default \
	01-BD_ADDRESS/beacon_201 \
	11-SLEEP_ENABLE/hib \
	12-EXT_WAKEUP_ENABLE/enable2 \
	b4-PMU_WURX/high_duty_adv \

include $(COMMON_USER_DIR)/framework.mk
