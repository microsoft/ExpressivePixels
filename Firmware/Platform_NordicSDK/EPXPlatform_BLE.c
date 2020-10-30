// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_dfu.h"
#include "nrf_delay.h"
#include "nrf_power.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
//#include "nrf_bootloader_info.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//#include "EPXApp.h"
#include "EPXPlatform_BLE.h"
#include "EPXPlatform_Runtime.h"
#include "EPXVariant.h"

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                320 //64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      0                                         /**< The advertising timeout (in units of seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */


BLE_NUS_DEF(g_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT); /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(g_gatt); /**< GATT module instance. */
BLE_ADVERTISING_DEF(g_advertising); /**< Advertising module instance. */

static bool g_bAdvertising = false;
static bool									g_ble_gatts_hvntx_complete = false;
static ble_advertising_init_t				g_bleAdvertisingInit;
static ble_advdata_manuf_data_t				g_manuf_specific_data;

static uint16_t								g_conn_handle          = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static uint16_t								g_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t							g_adv_uuids[] = /**< Universally unique service identifier. */
														{
															{ BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE }
														};


NRF_BLE_SCAN_DEF(m_scan); /**< Scanning module instance. */

static uint8_t								g_beaconData = 0;
static uint32_t								g_lastBeaconReceived = 0;
static uint8_t								g_emptyBeaconHostAddr[BLE_GAP_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static BEACONACTIVATIONITEM					**g_ppBeaconActivationEntries = NULL;


static uint8_t								*g_manufacturerPayload = NULL;
static uint8_t								g_cbManufacturerPayloadLength = 0;

static char									*g_pszDEFAULT_BLE_NAME = NULL;
char										g_szDeviceName[BLEMAX_DEVICENAME] = "";
char										g_szRealizedDeviceName[BLEMAX_DEVICENAME] = "";
void										*g_hostInstance = NULL;
PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED	g_pfnConnectionStateChanged = NULL;
PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY		g_pfnCommunicationReady = NULL;
PFN_EPXPLATFORM_BLE_BYTERECEIVED			g_pfnByteReceived = NULL;
PFN_EPXPLATFORM_BLE_BEACONRECEIVED			g_pfnBLEBeaconReceived = NULL;



void gap_params_init(void)
{
	char					szDeviceName[BLEMAX_DEVICENAME];
	uint32_t                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
		
	// Set the advertising name
	strncpy(szDeviceName, (char *)(g_szDeviceName[0] != 0x00 ? g_szDeviceName : g_pszDEFAULT_BLE_NAME), BLEMAX_DEVICENAME);		
		
	// If the device hasn't been previously named then append the last two bytes of MAC address for somewhat uniqueness
	if(g_szDeviceName[0] == 0x00)
	{
		ble_gap_addr_t	gapAddr;
		char			szHex[5];
		
		sd_ble_gap_addr_get(&gapAddr);
		sprintf(&szDeviceName[strlen(szDeviceName)], " %X%X", gapAddr.addr[BLE_GAP_ADDR_LEN - 1], gapAddr.addr[BLE_GAP_ADDR_LEN - 2]);
	}
	szDeviceName[BLEMAX_DEVICENAME - 1] = 0x00;
	strcpy(g_szRealizedDeviceName, szDeviceName);
				
	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) szDeviceName, strlen(szDeviceName));
	APP_ERROR_CHECK(err_code);
	NRF_LOG_DEBUG("\tBLE Device Name %s", szDeviceName);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));
	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency     = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}

	
	
	
/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
 /**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
	//NRF_LOG_INFO("nus_data_handler %d", p_evt->type);		
	if(p_evt->type == BLE_NUS_EVT_COMM_STARTED)
	{
		DEBUGLOGLN("BLENUS Communication READY");
		if (g_pfnCommunicationReady != NULL)			
			(*g_pfnCommunicationReady)(g_hostInstance);
	}		
	else if(p_evt->type == BLE_NUS_EVT_RX_DATA)
	{
		//NRF_LOG_DEBUG("Received %d bytes data from BLE NUS", p_evt->params.rx_data.length);
		//NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
		for(uint32_t i = 0 ; i < p_evt->params.rx_data.length ; i++)
			(*g_pfnByteReceived)(g_hostInstance, p_evt->params.rx_data.p_data[i]);			
	}
}

	

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
	uint32_t       err_code;
	ble_nus_init_t nus_init;

	memset(&nus_init, 0, sizeof(nus_init));

	nus_init.data_handler = nus_data_handler;

	err_code = ble_nus_init(&g_nus, &nus_init);
	APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
	uint32_t err_code;

	DEBUGLOGLN("on_conn_params_evt %d", p_evt->evt_type);
	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
	{
		err_code = sd_ble_gap_disconnect(g_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
		APP_ERROR_CHECK(err_code);
	}
}



/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
	APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
	uint32_t               err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params                  = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	cp_init.disconnect_on_fail             = false;
	cp_init.evt_handler                    = on_conn_params_evt;
	cp_init.error_handler                  = conn_params_error_handler;

	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
}


	
	
/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
	uint32_t err_code;

	switch (ble_adv_evt)
	{
	case BLE_ADV_EVT_FAST:
		break;
	case BLE_ADV_EVT_IDLE:
		//sleep_mode_enter();
		break;
	default:
		break;
	}
}



/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
	uint32_t err_code;
	ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

	// DEBUGLOGLN("ble_evt_handler %d", p_ble_evt->header.evt_id);
	switch (p_ble_evt->header.evt_id)
	{
	case BLE_GAP_EVT_CONNECTED:
		NRF_LOG_INFO("BLE Connected %d ms", p_ble_evt->evt.gap_evt.params.connected.conn_params.min_conn_interval * 1.25);
		g_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		(*g_pfnConnectionStateChanged)(g_hostInstance, true);
		break;

	case BLE_GAP_EVT_DISCONNECTED:
		NRF_LOG_INFO("BLE Disconnected");
		g_conn_handle = BLE_CONN_HANDLE_INVALID;
		(*g_pfnConnectionStateChanged)(g_hostInstance, false);
		break;

	case BLE_GAP_EVT_ADV_REPORT:
		if (g_ppBeaconActivationEntries != NULL)
		{
			BEACONACTIVATIONITEM *pItem = *g_ppBeaconActivationEntries;
				
			while (pItem != NULL)
			{
				// If the MAC address hasn't been seend and cached yet, try to resolve by name
				if(memcmp(pItem->beaconHostAddr, g_emptyBeaconHostAddr, sizeof(g_emptyBeaconHostAddr)) == 0)
				{
					char szScanDeviceName[32];
					uint16_t offset = 0;
					uint16_t nameLen = ble_advdata_search(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, &offset, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);
						
					if (nameLen > 0)
					{							
						memcpy(szScanDeviceName, &p_gap_evt->params.adv_report.data.p_data[offset], nameLen);
						szScanDeviceName[nameLen] = 0x00;							
						if (stricmp(pItem->szBeaconHostName, szScanDeviceName) == 0)						
							memcpy(pItem->beaconHostAddr, p_gap_evt->params.adv_report.peer_addr.addr, sizeof(pItem->beaconHostAddr));
					}
				}
			
				// See if MAC addresses match
				if(memcmp(p_gap_evt->params.adv_report.peer_addr.addr, pItem->beaconHostAddr, sizeof(pItem->beaconHostAddr)) == 0)			
				{					
					uint8_t *mfd_data = ble_advdata_parse(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
					if (mfd_data != NULL)
					{
						uint8_t advBeaconData = (uint8_t) *(mfd_data + sizeof(uint16_t));

						g_lastBeaconReceived = millis();
						if (g_beaconData != advBeaconData)
						{
							NRF_LOG_INFO("MANUDATA 0x%02x", advBeaconData);
							g_beaconData = advBeaconData;
							if (g_pfnBLEBeaconReceived != NULL)
								(*g_pfnBLEBeaconReceived)(g_hostInstance, pItem->szBeaconHostName, g_beaconData);
						}
					}					
				}					
				pItem = pItem->pNext;
			}
		}
		break;

#ifndef S140
	case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
		{
			NRF_LOG_DEBUG("PHY update request.");
			ble_gap_phys_t const phys =
			{
				.rx_phys = BLE_GAP_PHY_AUTO,
				.tx_phys = BLE_GAP_PHY_AUTO,
			};
			err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
			APP_ERROR_CHECK(err_code);
		} break;
#endif

	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		// Pairing not supported
		err_code = sd_ble_gap_sec_params_reply(g_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
		APP_ERROR_CHECK(err_code);
		break;
		
#if !defined (S112)
	case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
		{
			ble_gap_data_length_params_t dl_params;

			// Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
			memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
			err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
			APP_ERROR_CHECK(err_code);
		} break;
#endif //!defined (S112)
		
	case BLE_GATTS_EVT_SYS_ATTR_MISSING:
		// No system attributes have been stored.
		err_code = sd_ble_gatts_sys_attr_set(g_conn_handle, NULL, 0, 0);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTC_EVT_TIMEOUT:
		// Disconnect on GATT Client timeout event.
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
			BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_TIMEOUT:
		// Disconnect on GATT Server timeout event.
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
			BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
		g_ble_gatts_hvntx_complete = true;
		break;
			
	case BLE_EVT_USER_MEM_REQUEST:
		err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_WRITE:
		break;
		
	case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		{
			ble_gatts_evt_rw_authorize_request_t  req;
			ble_gatts_rw_authorize_reply_params_t auth_reply;

			req = p_ble_evt->evt.gatts_evt.params.authorize_request;

			if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
			{
				if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
				    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
				    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
				{
					if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
					{
						auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
					}
					else
					{
						auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
					}
					auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
					err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
						&auth_reply);
					APP_ERROR_CHECK(err_code);
				}
			}
		} break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

		default :
		    // No implementation needed.
		    break;
	}
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Register a handler for BLE events.
	NRF_SDH_BLE_OBSERVER(g_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}



/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
	if ((g_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
	{
		g_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
		NRF_LOG_INFO("Data len is set to 0x%X(%d)", g_ble_nus_max_data_len, g_ble_nus_max_data_len);
	}
	NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
		p_gatt->att_mtu_desired_central,
		p_gatt->att_mtu_desired_periph);
}



/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
	ret_code_t err_code;

	err_code = nrf_ble_gatt_init(&g_gatt, gatt_evt_handler);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_gatt_att_mtu_periph_set(&g_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);    //64);
	APP_ERROR_CHECK(err_code);
}
	
	
	
/**@brief Function for handling Scaning events.
*
* @param[in]   p_scan_evt   Scanning event.
*/
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	ret_code_t err_code;

	switch (p_scan_evt->scan_evt_id)
	{
	case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
		err_code = p_scan_evt->params.connecting_err.err_code;
		APP_ERROR_CHECK(err_code);
		break;
	default:
		break;
	}
}


	
static void scan_init(void)
{
	ret_code_t          err_code;
	nrf_ble_scan_init_t init_scan;
		
	memset(&init_scan, 0, sizeof(init_scan));

	//init_scan.connect_if_match = true;
	init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

	err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
	APP_ERROR_CHECK(err_code);
}
	

	
/**@brief Function to start scanning.
 */
static void scan_start(void)
{
	nrf_ble_scan_start(&m_scan);
}

	
static void scan_stop(void)
{
	nrf_ble_scan_stop();
}



uint32_t EPXPlatform_BLE_GetLastBeaconReceived()
{
	return g_lastBeaconReceived;
}


void EPXPlatform_BLE_SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries)
{
	g_ppBeaconActivationEntries = ppBeaconActivationEntries;
}



void EPXPlatform_BLE_SetBeaconActivation(bool on)
{
#ifndef DISABLE_BLE_ADVERTISING
	if (on)
	{
		scan_start();
		g_lastBeaconReceived = millis();
	}
	else
	{
		scan_stop();
		g_lastBeaconReceived = 0;
		delay(250);  // Wait for all processing to stop
	}
#endif
}

	
	
static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
	memset(p_config, 0, sizeof(ble_adv_modes_config_t));

	p_config->ble_adv_fast_enabled  = true;
	p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
	p_config->ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
}

	
	
/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
	uint32_t               err_code;

	memset(&g_bleAdvertisingInit, 0, sizeof(g_bleAdvertisingInit));

	g_bleAdvertisingInit.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
	g_bleAdvertisingInit.advdata.include_appearance = true;
	g_bleAdvertisingInit.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
		
	g_manuf_specific_data.data.p_data        = (uint8_t *) g_manufacturerPayload;
	g_manuf_specific_data.data.size          = g_cbManufacturerPayloadLength;
	g_bleAdvertisingInit.srdata.p_manuf_specific_data = &g_manuf_specific_data;
	g_bleAdvertisingInit.srdata.uuids_complete.uuid_cnt = sizeof(g_adv_uuids) / sizeof(g_adv_uuids[0]);
	g_bleAdvertisingInit.srdata.uuids_complete.p_uuids  = g_adv_uuids;
		
	g_bleAdvertisingInit.config.ble_adv_fast_enabled  = true;
	g_bleAdvertisingInit.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
	g_bleAdvertisingInit.config.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

	g_bleAdvertisingInit.evt_handler = on_adv_evt;

	sd_ble_gap_appearance_set(1990);
	err_code = ble_advertising_init(&g_advertising, &g_bleAdvertisingInit);
	APP_ERROR_CHECK(err_code);

	ble_advertising_conn_cfg_tag_set(&g_advertising, APP_BLE_CONN_CFG_TAG);
}


	
static void advertising_Update()
{
    if(g_bAdvertising)
    {
	ret_code_t err_code = ble_advdata_encode(&g_bleAdvertisingInit.advdata, g_advertising.enc_advdata, &g_advertising.adv_data.adv_data.len);	
	err_code = ble_advdata_encode(&g_bleAdvertisingInit.srdata, g_advertising.enc_scan_rsp_data, &g_advertising.adv_data.scan_rsp_data.len);
    }
}
	
	
	
static void disconnect(uint16_t conn_handle, void * p_context)
{
	UNUSED_PARAMETER(p_context);

	ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
	if (err_code != NRF_SUCCESS)
	{
		NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
	}
	else
	{
		NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
	}
}



void EPXPlatform_BLE_SetDeviceName(char *pszDeviceName)
{
	strcpy(g_szDeviceName, pszDeviceName);
}



char *EPXPlatform_BLE_GetRealizedDeviceName()
{
	return g_szRealizedDeviceName;
}



void EPXPlatform_BLE_Initialize(void *pinstance, char *pszDEFAULT_BLE_NAME, PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_BLE_BYTERECEIVED pfnByteReceived)
{
	g_hostInstance = pinstance;
	g_pszDEFAULT_BLE_NAME = pszDEFAULT_BLE_NAME;
	g_pfnConnectionStateChanged = pfnConnectionStateChanged;
	g_pfnCommunicationReady = pfnCommunicationReady;
	g_pfnByteReceived = pfnByteReceived;
	
	ble_stack_init();
	gap_params_init();
	gatt_init();
	services_init();
	advertising_init();
	conn_params_init();
}



void EPXPlatform_BLE_Start()
{
	scan_init();
			
#ifndef DISABLE_BLE_ADVERTISING	
	uint32_t err_code = ble_advertising_start(&g_advertising, BLE_ADV_MODE_FAST);
	APP_ERROR_CHECK(err_code);
        g_bAdvertising = true;
	NRF_LOG_INFO("\tAdvertising started");
#endif				
}



void EPXPlatform_BLE_Disconnect()
{
	if (g_conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		disconnect(g_conn_handle, NULL);
		g_conn_handle = BLE_CONN_HANDLE_INVALID;
	}
}



void EPXPlatform_BLE_SetBeaconReceivedHandler(PFN_EPXPLATFORM_BLE_BEACONRECEIVED pfnBLEBeaconReceived)
{
	g_pfnBLEBeaconReceived = pfnBLEBeaconReceived;
}



void EPXPlatform_BLE_SetManufacturerPayload(uint8_t *p, uint8_t cb)
{
	g_manufacturerPayload = p;
	g_cbManufacturerPayloadLength = cb;
}



void EPXPlatform_BLE_AdvertizingUpdate()
{
	advertising_Update();
}



size_t EPXPlatform_BLE_SendBytes(void *pvPayload, uint16_t cb)
{
	uint8_t *pPayload = (uint8_t *)pvPayload;
	size_t requestBytesToWrite = cb, bytesWritten = 0;

	ble_gatts_hvx_params_t hvx_params;
	uint16_t sendLength;
	uint32_t err_code;
		
	//NRF_LOG_DEBUG("BLESendData %d bytes", cb);			
	memset(&hvx_params, 0, sizeof(hvx_params));

	hvx_params.handle = g_nus.tx_handles.value_handle;
	hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
	while (cb > 0)
	{
		sendLength = epxmin(cb, g_ble_nus_max_data_len);
		//NRF_LOG_DEBUG("\tsending %d bytes", sendLength);
			
		hvx_params.p_data = pPayload;
		hvx_params.p_len = &sendLength;
		g_ble_gatts_hvntx_complete = false;
		err_code = sd_ble_gatts_hvx(g_conn_handle, &hvx_params);
		if (err_code != NRF_SUCCESS)
		{	
			NRF_LOG_DEBUG("BLESendData sd_ble_gatts_hvx ERROR %d", err_code);	
			if (err_code == NRF_ERROR_RESOURCES)
			{
				int ms = 0;
				while (!g_ble_gatts_hvntx_complete)
				{
					nrf_delay_ms(10);   				
					ms += 10;
				}
				NRF_LOG_DEBUG("BLESendData sd_ble_gatts_hvx RETRY, waited %d ms", ms);	
				continue;
			}						
			return err_code;
		}
		
		cb -= sendLength;
		bytesWritten += sendLength;
		pPayload += sendLength;
	}	
	//NRF_LOG_DEBUG("BLESendData sd_ble_gatts_hvx sent %d bytes of %d", bytesWritten, requestBytesToWrite);		
	return bytesWritten;
}

