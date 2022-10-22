include ../../user/common.mk

ifndef RUN_IN_RAM
CFG_NVDS_ADV := 1
endif
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

ifdef CFG_DYN_ADV
CFG_NVDS_ADV :=
CFLAGS += \
	-DCFG_DYN_ADV \
	-DCFG_ADV_CREATE_PARAM_CONST=0 \
	-DCFG_ADV_DATA_PARAM_CONST=0
endif

ifdef CFG_NVDS_ADV
CFLAGS += -DCFG_NVDS_ADV
endif

ifdef CFG_PREDEFINED_ADV
CFLAGS += \
	-DENABLE_USER_ADV_TIMEOUT \
	-DENABLE_USER_ADV_PARAM_SETTING \
	-DENABLE_USER_ADV_DATA_SCANRSP \
	-DCFG_ADV_DATA_PARAM_CONST=0
endif

include reference_beacons.mk
flash_nvds.data := $(reference_beacon_$(REF_BCN)) $(if $(AUTO_RESTART),09-APP_BLE_RSTRT_DUR/500ms)

include $(COMMON_USER_DIR)/framework.mk
