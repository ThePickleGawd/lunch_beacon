include ../../user/common.mk

BOARD=m2202
DEBUG := 1

DRIVERS := interrupt timer sw_timer atm_ble atm_pm
LIBRARIES := prf

FRAMEWORK_MODULES := \
	atm_adv \
	atm_asm \
	atm_common \
	atm_debug \
	atm_gap \
	atm_log \
	ble_gap \

UU_TEST := lunch_beacon atm_adv
INCLUDES += .

CFLAGS += \
	-DNO_GAP_SEC \
	-DNO_ATM_SCAN \
	-DNO_BLE_GATTC \
	-DGAP_ADV_PARM_NAME="config_adv_params.h" \
	-Wno-unknown-pragmas \

# CFG_DYN_ADV Stuff
CFG_NVDS_ADV :=

include reference_beacons.mk
flash_nvds.data := $(reference_beacon_$(REF_BCN)) $(if $(AUTO_RESTART),09-APP_BLE_RSTRT_DUR/500ms)

include $(COMMON_USER_DIR)/framework.mk
