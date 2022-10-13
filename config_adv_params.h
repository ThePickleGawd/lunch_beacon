/**
 *******************************************************************************
 *
 * @file config_adv_params.h
 *
 * @brief advertising configuration
 *
 * Copyright (C) Atmosic 2020
 *
 *******************************************************************************
 */
//***<<< Use Configuration Wizard in Context Menu >>>***

#pragma once

//=============================================================================
//<h> Advertising Timeout Configuration
//<i> Advertising Timeout Configuration: Please enable first check box first
// <c1> Enable User Define Advertising Timeout
// <i> Will use below settings and not to use default or Flash NVDS's settings
//#define ENABLE_USER_ADV_TIMEOUT
// </c>
#ifdef ENABLE_USER_ADV_TIMEOUT

// <c1> Advertising continuously
// <i> Advertising continuously (will ignore ADV0_START_DURATION and CFG_ADV0_START_MAX_ADV_EVENT parameter)
#define ADV_CONTINOUS
// </c>

//<o> Advertising Timeout Duration
//<i> Advertising Timeout Duration (unit: ms)
//<10-655350>
#define ADV0_START_DURATION 30000
#define CFG_ADV0_START_DURATION ((uint16_t)(ADV0_START_DURATION/10))

//<o> Maximum number of advertising events
//<i> Maximum number of advertising events
//<0-255>
#define CFG_ADV0_START_MAX_ADV_EVENT 0

#ifdef ADV_CONTINOUS
#undef CFG_ADV0_START_DURATION
#undef CFG_ADV0_START_MAX_ADV_EVENT
#define CFG_ADV0_START_DURATION 0
#define CFG_ADV0_START_MAX_ADV_EVENT 0
#endif

#endif // end ENABLE_USER_ADV_TIMEOUT
//</h> // end Advertising Timeout Configuration
//=============================================================================



//=============================================================================
//<h> Advertising Parameter
//<i> Advertising Parameter: Please enable first check box first
// <c1> Enable User Define Advertising Parameter
// <i> Enable User Advertising Parameter Setting
//#define ENABLE_USER_ADV_PARAM_SETTING
// </c>

#ifdef ENABLE_USER_ADV_PARAM_SETTING

/// Type of advertising that can be created
// <o> Type of advertising
// <i> Type of advertising that can be created
//   <0=> Legacy advertising
//   <1=> Extended advertising
//   <2=> Periodic advertising
#define CFG_ADV0_CREATE_TYPE 0

/// Type of Advertising discovery mode
// <o> Advertising discovery mode
// <i> Advertising discovery mode
//   <0=> Mode in non-discoverable
//   <1=> Mode in general discoverable
//   <2=> Mode in limited discoverable
//   <3=> Broadcast mode without presence of AD_TYPE_FLAG
#define CFG_ADV0_CREATE_DISCOVERY_MODE 1

//<h> Advertising Property
//<i> Advertising properties configurations

#if (CFG_ADV0_CREATE_TYPE == 0)
// <o> Legacy advertising property
// <i> For legacy advertising
//   <0=> Non connectable non scannable advertising
//   <2=> Non connectable scannable advertising
//   <3=> Undirected connectable advertising
//   //5=> Directed connectable advertising or Low Duty Cycle
//   //13=> Directed connectable with High Duty Cycle
#define CFG_ADV0_CREATE_PROPERTY 0

#if (CFG_ADV0_CREATE_PROPERTY == 0)   //Non connectable non scannable adv.
#define CFG_ADV0_DATA_SCANRSP_ENABLE 0
#elif (CFG_ADV0_CREATE_PROPERTY == 2) //Non connectable scannable adv.
#elif (CFG_ADV0_CREATE_PROPERTY == 3) //Undirected connectable adv.
#endif

#else

// <o> Extended advertising property
// <i> For extended advertising
//   <0=> Non connectable non scannable extended advertising
//   <2=> Non connectable scannable extended advertising
//   //6=> Non connectable scannable directed extended advertising
//   //36=> Non connectable anonymous directed extended advertising
//   <1=> Undirected connectable extended advertising
//   //5=> Directed connectable extended advertising
#define CFG_ADV0_CREATE_PROPERTY 0

#if (CFG_ADV0_CREATE_PROPERTY == 0)   //Non connectable non scannable adv.
#define CFG_ADV0_DATA_SCANRSP_ENABLE 0
#elif (CFG_ADV0_CREATE_PROPERTY == 2) //Non connectable scannable adv.
#define CFG_ADV0_DATA_ADV_ENABLE 0
#elif (CFG_ADV0_CREATE_PROPERTY == 1) //Undirected connectable extended adv.
#define CFG_ADV0_DATA_SCANRSP_ENABLE 0
#endif

#if (CFG_ADV0_CREATE_TYPE == 2)
#undef CFG_ADV0_DATA_SCANRSP_ENABLE
#define CFG_ADV0_DATA_SCANRSP_ENABLE 1
#define ENABLE_USER_ADV_TIMEOUT
#undef CFG_ADV0_START_DURATION
#define CFG_ADV0_START_DURATION 0
#undef CFG_ADV0_START_MAX_ADV_EVENT
#define CFG_ADV0_START_MAX_ADV_EVENT 0
#if (CFG_ADV0_CREATE_PROPERTY!=0)
#error "Periodic advertising uses wrong advertising property. Please check CFG_ADV0_CREATE_TYPE and CFG_ADV0_CREATE_PROPERTY"
#endif
#endif // end (CFG_ADV0_CREATE_TYPE==2)

#endif // end (CFG_ADV0_CREATE_TYPE==0)

//</h>


//<h> Advertising channels enables
// <i> Advertising channels enables
//   <q.0> for channel 37 enable
//   <q.1> for channel 38 enable
//   <q.2> for channel 39 enable
#define CFG_ADV0_CREATE_CHNL_MAP 0x07
//</h>

//<o> Select the Primary PHY
// <i> Select the Primary PHY
//   <1=> LE 1M PHY
//   <3=> LE Coded PHY
#define CFG_ADV0_CREATE_PRIM_PHY 0x01

#if (CFG_ADV0_CREATE_PRIM_PHY == 3) && (CFG_ADV0_CREATE_TYPE == 0)
#error "Coded PHY can't use on Legacy advertising. Please check CFG_ADV0_CREATE_PRIM_PHY and CFG_ADV0_CREATE_TYPE"
#endif

//<o> Min. Advertising interval
//<i> Min. Advertising interval (unit: ms)
//<20-10485759>
#define ADV0_INTERVAL_MIN 250
#define CFG_ADV0_CREATE_INTERVAL_MIN ((uint32_t)ADV0_INTERVAL_MIN*1000/625)

//<o> Max. Advertising interval
//<i> Max. Advertising interval (unit: ms)
//<20-10485759>
#define ADV0_INTERVAL_MAX 250
#define CFG_ADV0_CREATE_INTERVAL_MAX ((uint32_t)ADV0_INTERVAL_MAX*1000/625)


//<o> Maximum power level
// <i> Maximum power level
//   <0xEC=> -20dbm
//   <0xF6=> -10dbm
//   <0xFA=> -6dbm
//   <0xFC=> -4dbm
//   <0xFE=> -2dbm
//   <0x0=> 0dbm
//   <0x02=> 2dbm
//   <0x04=> 4dbm
#define CFG_ADV0_CREATE_MAX_TX_POWER 0x00


//<h> Secondary PHY
//<i> Configuration for secondary phy.

//<o> Adv Skip valid
//<i> Adv Skip valid
//<0-499>
#define CFG_ADV0_CREATE_SEC_MAX_SKIP 0

//<o> Select the Secondary PHY
// <i> Select the Secondary PHY
//   <1=> LE 1M PHY
//   <2=> LE 2M PHY
//   <3=> LE Coded PHY
#define CFG_ADV0_CREATE_SEC_PHY 0x01

//<o> Advertising SID
//<i> Advertising SID
//<0-15>
#define CFG_ADV0_CREATE_SEC_ADV_SID 0

//</h>

//<h> Periodic advertising
//<i> Configuration for Periodic advertising

//<o> Min. Periodic advertising interval
//<i> Min. Periodic advertising interval (unit: us)
//<8750-81918750>
#define ADV0_PERI_INTERVAL_MIN 200000
#define CFG_ADV0_CREATE_PERI_INTERVAL_MIN ((uint16_t)(ADV0_PERI_INTERVAL_MIN/1250))

//<o> Max. Periodic advertising interval
//<i> Max. Periodic advertising interval (unit: us)
//<8750-81918750>
#define ADV0_PERI_INTERVAL_MAX 200000
#define CFG_ADV0_CREATE_PERI_INTERVAL_MAX ((uint16_t)(ADV0_PERI_INTERVAL_MAX/1250))

//</h>
#endif // end ENABLE_USER_ADV_PARAM_SETTING
//</h> // end Advertising Parameter
//=============================================================================



//=============================================================================
//<h> Advertising Data/ScanRsp Payload
//<i> Advertising Data/ScanRsp Payload: Please enable first check box first
// <c1> Enable Define Data/ScanRsp
// <i> Will use below settings and not to use default or Flash NVDS's settings
// #define ENABLE_USER_ADV_DATA_SCANRSP
// </c>

#if defined(CFG_DYN_ADV)
#define BLE_ADV_SIZE 31
#define ATM_ADV_LEN 27
#define CFG_ADV0_DATA_ADV_PAYLOAD 0x03, 0x03, 0x12, 0x18, 0x16, 0x09, \
    'A', 't', 'm', 'o', 's', 'i', 'c', ' ', 'a', 'd', 'v', '.', 'c', 'o', 'u', \
    'n', 't', '=', '0', '0', '0'
#define CFG_ADV0_DATA_SCANRSP_PAYLOAD 0x03, 0x03, 0x12, 0x18, 0x16, 0x09, \
    'A', 't', 'm', 'o', 's', 'i', 'c', ' ', 'a', 'd', 'v', '.', 'c', 'o', 'u', \
    'n', 't', '=', '0', '0', '0'
#elif defined(ENABLE_USER_ADV_DATA_SCANRSP)
#define BLE_ADV_DAT_MANU 0x06, 0xFF, 0x0A, 0x24, 0x01, 0x02, 0x00
#define CFG_ADV0_DATA_ADV_PAYLOAD 0x03, 0x03, 0x12, 0x18, 0x08, 0x09, \
    'A', 't', 'm', 'o', 's', 'i', 'c', BLE_ADV_DAT_MANU
#define CFG_ADV0_DATA_SCANRSP_PAYLOAD 0x0F,0x09,0x41,0x74,0x6D,0x6F,0x73,0x69, \
    0x63,0x20,0x42,0x65,0x61,0x63,0x6F,0x6E
#endif

//</h> // end Advertising Data/ScanRsp Payload
//=============================================================================

//***<<< end of configuration section >>>***
