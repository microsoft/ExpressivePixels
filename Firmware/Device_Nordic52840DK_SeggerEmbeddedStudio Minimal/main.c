#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_log.h"
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



/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}




/**@brief Application main function. */
int main(void)
{
    // Initialize.
    uint32_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);	
    log_init();		
            
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

