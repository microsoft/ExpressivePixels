#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_pwr_mgmt.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_log.h"
#include "nrf_drv_clock.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "EPXPlatform_Runtime.h"

#define DEAD_BEEF                       0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

void NRF52_EPXApp_Initialize();
void NRF52_EPXApp_Process();



/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}



void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
	__disable_irq();
	
	
	NRF_LOG_ERROR("app_error_handler %d %d", error_code, line_num);
	NRF_LOG_FINAL_FLUSH();
#ifndef DEBUG
	NRF_LOG_WARNING("System reset");
	NVIC_SystemReset();
#else
	// The following variable helps Keil keep the call stack visible, in addition, it can be set to
    // 0 in the debugger to continue executing code after the error check.
    volatile bool loop = true;
	while (loop) ;	
#endif // DEBUG
}



/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
	switch (event)
	{
	case NRF_PWR_MGMT_EVT_PREPARE_DFU:
		NRF_LOG_INFO("Power management wants to reset to DFU mode.");
		// YOUR_JOB: Get ready to reset into DFU mode
		//
		// If you aren't finished with any ongoing tasks, return "false" to
		// signal to the system that reset is impossible at this stage.
		//
		// Here is an example using a variable to delay resetting the device.
		//
		// if (!m_ready_for_reset)
		// {
		//      return false;
		// }
		// else
		//{
		//
		//    // Device ready to enter
		//    uint32_t err_code;
		//    err_code = sd_softdevice_disable();
		//    APP_ERROR_CHECK(err_code);
		//    err_code = app_timer_stop_all();
		//    APP_ERROR_CHECK(err_code);
		//}
		break;

	default:
		// YOUR_JOB: Implement any of the other events available from the power management module:
		//      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
		//      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
		//      -NRF_PWR_MGMT_EVT_PREPARE_RESET
		return true;
	}

	NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
	return true;
}


/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);



/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}



void VDDSet3V3(void)
{
	if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_3V3) 
	{
		// Save away bootloader
		uint32_t bootloaderAddr = NRF_UICR->NRFFW[0];
		
		// Enable Erase mode
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos;  //0x02; 
		while(NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
		        
		// Erase the UICR registers
		NRF_NVMC->ERASEUICR = NVMC_ERASEUICR_ERASEUICR_Erase << NVMC_ERASEUICR_ERASEUICR_Pos;  //0x00000001;
		while(NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
		
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;     //write enable
		while(NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
		NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V3;                         //configurate REGOUT0
		NRF_UICR->NRFFW[0] = bootloaderAddr;		
		
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
		NVIC_SystemReset();                                                // Reset device
	} 
}


/*********************************************************************************/
/* Centralize heap functions so they can be swapped for tracing/tracking purposes
/*********************************************************************************/
void *tmalloc(const char *pszFile, int line, size_t siz)
{
  return malloc(siz);
}



void tfree(void *buf)
{
  free(buf);
}



void tmallocstats()
{
}



void tmallocdump()
{
}




/**@brief Application main function. */
int main(void)
{
	// Initialize.
	uint32_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
	
	log_init();		
        nrf_drv_clock_init();
	nrf_pwr_mgmt_init();
	
	// Ensure VDD LDO regulator is set to 3.3 volts to power component rail
	VDDSet3V3();
	
	EPXPlatform_Runtime_Initialize();
	NRF52_EPXApp_Initialize();
		
    // Enter main loop.
    for (;;)
    {
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());	    
	    	    
	    // Process main app
	    EPXPlatform_Runtime_Process();
	    NRF52_EPXApp_Process();
     }
}

