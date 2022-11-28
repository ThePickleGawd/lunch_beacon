
REF_BCN := blank

ifdef SUPPLY_BD_ADDR
supply_bd_addr := 01-BD_ADDRESS/beacon_201
endif

reference_beacon_blank := \
	01-BD_ADDRESS/beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# Lunch Beacon
#
# Currently, it is based on simple_beacon
# It will advertise forever, allowing us to see it on scanner app
#
reference_beacon_lunch_beacon := \
	01-BD_ADDRESS/beacon_201 \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_s_100ms \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \
	0b-APP_BLE_ADV_DATA/lunch_beacon \
	0c-APP_BLE_SCAN_RESP_DATA/lunch_beacon \
	05-APP_BLE_ACT_STRT_CMD/180s_adv \
	
#
# power_profile
#
# Send one scannable legacy Eddystone beacon each second until 30 are sent.
# Between beacons, use the retain w/ voltage drop power mode.
# Then hibernate for 30 seconds, and repeat.
#
# This is a good example for measuring the device's low power performance.
#
reference_beacon_power_profile := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_s_1s \
	0b-APP_BLE_ADV_DATA/beacon_201 \
	0c-APP_BLE_SCAN_RESP_DATA/beacon_201 \
	05-APP_BLE_ACT_STRT_CMD/30_adv \
	09-APP_BLE_RSTRT_DUR/30s \
	11-SLEEP_ENABLE/hib \
	12-EXT_WAKEUP_ENABLE/enable2 \

#
# extended_adv
#
# Send one extended Eddystone beacon each half second.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_extended_adv := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/ext_nc_ns_500ms \
	0b-APP_BLE_ADV_DATA/ext_big_beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# extended_con
#
# Send one extended connectable Eddystone beacon each half second.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_extended_con := \
	$(supply_bd_addr) \
	02-DEVICE_NAME/beacon_201 \
	06-APP_BLE_ACT_CRT_CMD/ext_c_ns_500ms \
	0b-APP_BLE_ADV_DATA/ext_big_beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# extended_per
#
# Send one extended Eddystone beacon each half second.
# Also send 0c-APP_BLE_SCAN_RESP_DATA as periodic advertising.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_extended_per := \
	01-BD_ADDRESS/beacon_201 \
	06-APP_BLE_ACT_CRT_CMD/ext_per_500ms \
	0b-APP_BLE_ADV_DATA/ext_beacon_201 \
	0c-APP_BLE_SCAN_RESP_DATA/ext_beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# extended_scn
#
# Send one extended scannable advertisement each half second.
# The extended scan response will contain an Eddystone beacon.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_extended_scn := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/ext_nc_s_500ms \
	0c-APP_BLE_SCAN_RESP_DATA/ext_beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# simple_beacon
#
# Sends scannable beacon forever without hibernation.
# This will allow mobile applications on iOS and Android to always see
# the device when scanning.
#
reference_beacon_simple_beacon := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_s_100ms \
	0b-APP_BLE_ADV_DATA/simple_beacon \
	0c-APP_BLE_SCAN_RESP_DATA/simple_beacon \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# simple_harv_beacon
#
# Beacon example when using Harvesting.
# Refer to "ATM32x1 Energy Harvesting Application Note" for details.
#
# Sends scannable beacon forever without hibernation.
# This will allow mobile applications on iOS and Android to always see
# the device when scanning.
#
reference_beacon_simple_harv_beacon := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_s_1s \
	0b-APP_BLE_ADV_DATA/simple_beacon \
	0c-APP_BLE_SCAN_RESP_DATA/simple_harv_beacon \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# 2M_secondary
#
# Send one extended Eddystone beacon each second via the 2M phy.
# Between beacons, use the hibernate power mode.
#
reference_beacon_2M_secondary := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/ext_2M_nc_ns_1s \
	0b-APP_BLE_ADV_DATA/ext_huge_beacon_201 \
	05-APP_BLE_ACT_STRT_CMD/1_adv \
	09-APP_BLE_RSTRT_DUR/1s \
	11-SLEEP_ENABLE/hib \
	12-EXT_WAKEUP_ENABLE/enable2 \

#
# LR_coded
#
# Send one extended Eddystone beacon each second via the long range 125k phy.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_LR_coded := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/ext_LR_nc_ns_1s \
	0b-APP_BLE_ADV_DATA/ext_beacon_201 \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# LR500_coded
#
# Send one extended Eddystone beacon each second via the long range 500k phy.
# Between beacons, use the retain w/ voltage drop power mode.
#
reference_beacon_LR500_coded := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/ext_LR_nc_ns_1s \
	0b-APP_BLE_ADV_DATA/ext_beacon_201 \
	85-LE_CODED_PHY_500/500k \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# iBeacon
#
# Send one legacy iBeacon each 100ms.
# Between beacons, use the retain w/ voltage drop power mode.
# NOTE that the Signal Power has been set to 0xff in iBeacon.tds so that
# it can be measured and then overwritten in place.
#
reference_beacon_iBeacon := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_ns_100ms \
	0b-APP_BLE_ADV_DATA/iBeacon \
	11-SLEEP_ENABLE/ret_drop \
	12-EXT_WAKEUP_ENABLE/enable \

#
# WURX
#
# Send one legacy Eddystone beacon, then wait in the hibernate power mode
# for an event from the wake-up receiver.
# Send one scannable legacy Eddystone beacon each 300ms until 10 are sent.
# Between beacons, use the retain w/ voltage drop power mode.
# Repeat.
#
reference_beacon_WURX := \
	$(supply_bd_addr) \
	06-APP_BLE_ACT_CRT_CMD/legacy_nc_ns_30ms \
	0b-APP_BLE_ADV_DATA/beacon_201 \
	05-APP_BLE_ACT_STRT_CMD/1_adv \
	b4-PMU_WURX/wurx_demo \
	21-APP_WURX_BLE_ACT_CRT_CMD/legacy_nc_s_300ms \
	22-APP_WURX_BLE_ADV_DATA/beacon_201 \
	23-APP_WURX_BLE_SCAN_RESP_DATA/beacon_201 \
	20-APP_WURX_BLE_ACT_STRT_CMD/10_adv \
	11-SLEEP_ENABLE/hib \
	12-EXT_WAKEUP_ENABLE/enable2 \