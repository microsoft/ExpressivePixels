// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_USB.h"

/*********************/
// In nrf_drv_usbd_errata.h, nrf_drv_usbd_errata_type_52840_fp1 change if(.... 0x10 to 0x20)
/*********************/

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst, app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE						0
#define CDC_ACM_COMM_EPIN							NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE						1
#define CDC_ACM_DATA_EPIN							NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT							NRF_DRV_USBD_EPOUT1

/** @brief CDC_ACM class instance */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
	cdc_acm_user_ev_handler,
	CDC_ACM_COMM_INTERFACE,
	CDC_ACM_DATA_INTERFACE,
	CDC_ACM_COMM_EPIN,
	CDC_ACM_DATA_EPIN,
	CDC_ACM_DATA_EPOUT,
	APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

uint8_t												APP_USBD_STRINGS_PRODUCT[APP_USBD_CONFIG_DESC_STRING_SIZE];

static char											m_cdc_data_array[64];
unsigned long										g_usbBytesReceived = 0;
unsigned long										g_cUSBRXReceived = 0;
int													g_USBTXWaiting = 0;

void												*g_USBHostInstance = NULL;
static PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED	g_pfnUSBConnectionStateChanged = NULL;
static PFN_EPXPLATFORM_USB_COMMUNICATIONREADY		g_pfnUSBCommunicationReady = NULL;
static PFN_EPXPLATFORM_USB_POWERSTATECHANGED		g_pfnUSBPowerStateChanged = NULL;
static PFN_EPXPLATFORM_USB_BYTERECEIVED				g_pfnUSBByteReceived = NULL;




/** @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
	app_usbd_cdc_acm_user_event_t event)
{
	app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

	switch (event)
	{
	case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
		{			
			NRF_LOG_INFO("CDC ACM port opened");		
			g_USBTXWaiting = false;

			/*Set up the first transfer*/
			ret_code_t ret = app_usbd_cdc_acm_read_any(p_cdc_acm, m_cdc_data_array, sizeof(m_cdc_data_array));
			(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, true);			
			(*g_pfnUSBCommunicationReady)(g_USBHostInstance);			
			if (ret == NRF_SUCCESS)
				(*g_pfnUSBByteReceived)(g_USBHostInstance, m_cdc_data_array[0]);
			break;
		}

	case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
		NRF_LOG_INFO("CDC ACM port closed");
		g_USBTXWaiting = false;
		(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, false);
		break;

	case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
		g_USBTXWaiting = false;
		break;

	case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
		g_cUSBRXReceived++;
		break;

	default:
		break;
	}
}



static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
	switch (event)
	{
	case APP_USBD_EVT_DRV_SUSPEND:
		break;

	case APP_USBD_EVT_DRV_RESUME:
		break;

	case APP_USBD_EVT_STARTED:
		break;

	case APP_USBD_EVT_STOPPED:
		app_usbd_disable();
		break;

	case APP_USBD_EVT_POWER_DETECTED:
		NRF_LOG_INFO("USB power detected");	
		(*g_pfnUSBPowerStateChanged)(g_USBHostInstance, true);
		if (!nrf_drv_usbd_is_enabled())
			app_usbd_enable();
		break;

	case APP_USBD_EVT_POWER_REMOVED:
		{
			NRF_LOG_INFO("USB power removed");			
			app_usbd_stop();
			
			/*** USB cable disconnect doesn't generate port close disconnection event, so the disconnection needs to be processed here ***/
			(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, false);			
			(*g_pfnUSBPowerStateChanged)(g_USBHostInstance, false);
		}
		break;

	case APP_USBD_EVT_POWER_READY:
		{
			NRF_LOG_INFO("USB ready");
			app_usbd_start();
		}
		break;

	default:
		break;
	}
}



size_t EPXPlatform_USB_Write(uint8_t *p, uint16_t cb)
{
	uint32_t err_code;
	
        DEBUGLOGLN("EPXPlatform_USB_Write %d bytes", cb);
	// Wait until USB has finished its last transmission as the single send buffer needs to be serialized until USB transmission is complete
	g_USBTXWaiting = true;
	err_code = app_usbd_cdc_acm_write(&m_app_cdc_acm, p, cb);
	if (err_code == NRF_SUCCESS)
	{
		// Process USB subsystem until TX completetion event fires
		while(g_USBTXWaiting)
			while(app_usbd_event_queue_process());
		return cb;
	}
	g_USBTXWaiting  = false;		
	NRF_LOG_INFO("EPX_USBChannelWrite::app_usbd_cdc_acm_write ERROR %d", err_code);
	return 0;
}


// USB CODE END



void EPXPlatform_USB_SetDeviceName(char *pszDeviceName)
{
	// Generate the USB description name
	strcpy((char *) APP_USBD_STRINGS_PRODUCT, EPXUSB_NAME_ROOT);
	strcat((char *) APP_USBD_STRINGS_PRODUCT, pszDeviceName);
}



bool EPXPlatform_USB_Initialize(void *pinstance, PFN_EPXPLATFORM_USB_POWERSTATECHANGED pfnUSBPowerStateChanged, PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_USB_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_USB_BYTERECEIVED pfnByteReceived)
{
	g_USBHostInstance = pinstance;
	g_pfnUSBPowerStateChanged = pfnUSBPowerStateChanged;
	g_pfnUSBCommunicationReady = pfnCommunicationReady;
	g_pfnUSBConnectionStateChanged = pfnConnectionStateChanged;
	g_pfnUSBByteReceived = pfnByteReceived;
	
	// Setup USB
	static const app_usbd_config_t usbd_config = {
		.ev_state_proc = usbd_user_ev_handler
	};
		
	app_usbd_serial_num_generate();
	int err_code = app_usbd_init(&usbd_config);
	APP_ERROR_CHECK(err_code);
	app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
	err_code = app_usbd_class_append(class_cdc_acm);
	APP_ERROR_CHECK(err_code);

	return true;
}



bool EPXPlatform_USB_Activate()
{
	// Listen for USB power events
	int err_code = app_usbd_power_events_enable();
	APP_ERROR_CHECK(err_code);
}



void EPXPlatform_USB_Process()
{
	if (g_cUSBRXReceived > 0)
	{
		ret_code_t ret = NRF_SUCCESS;
                int bytesProcessed = 0;
			                
		// Process RXs from USB
		do
		{
			// Get amount of data transfered
			size_t transferred = app_usbd_cdc_acm_rx_size(&m_app_cdc_acm);
		
                        bytesProcessed += transferred;
			uint8_t *pRXBuf = (uint8_t *) m_cdc_data_array;
			g_usbBytesReceived += transferred;
			while (transferred--)	
				(*g_pfnUSBByteReceived)(g_USBHostInstance, *pRXBuf++);			
				
			/* Fetch data until internal buffer is empty */
			ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm, m_cdc_data_array, sizeof(m_cdc_data_array));				
		} while (ret == NRF_SUCCESS) ;
		g_cUSBRXReceived--;
	}
	    
	// Process USB subsystem
	while(app_usbd_event_queue_process());
}

