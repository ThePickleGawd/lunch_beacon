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
	sw_event \

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

SRC_TOP = src
SRC_BT = src/bt

# application: BT part
C_SRCS += $(SRC_BT)/atts_gatt.c
INCLUDES += $(SRC_BT)

# application: top
C_SRCS += $(SRC_TOP)/BLE_adv.c $(SRC_TOP)/BLE_att_server.c
INCLUDES += $(SRC_TOP)

UU_TEST := lunch_beacon atm_adv
INCLUDES += .

CFLAGS += \
	-DCFG_NO_GAP_SEC \
	-DCFG_NO_GAP_SCAN \
	-DCFG_NO_GATTC \
	-Wno-error=unused-function

# att server stuff
CFLAGS += \
	-DGAP_PARM_NAME="param_gap.h" \
	-DGCOV_PREVENT_RET \
	-DCFG_GAP_MAX_LINKS=5 \

# Predefined header adv stuff
CFLAGS += \
	-DENABLE_USER_ADV_TIMEOUT \
	-DENABLE_USER_ADV_PARAM_SETTING \
	-DENABLE_USER_ADV_DATA_SCANRSP \
	-DCFG_ADV_DATA_PARAM_CONST=0

include reference_beacons.mk
flash_nvds.data := $(reference_beacon_$(REF_BCN)) $(if $(AUTO_RESTART),09-APP_BLE_RSTRT_DUR/500ms)

include $(COMMON_USER_DIR)/framework.mk
