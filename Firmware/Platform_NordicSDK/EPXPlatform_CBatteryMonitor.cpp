// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "EPXPlatform_CBatteryMonitor.h"

#define VBAT_MV_PER_LSB   (0.87890625F)   // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096
#define VBAT_MV_PER_LSB   (0.87890625F)   // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096
// #define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
// #define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider
#define VBAT_DIVIDER      (0.59820538F)   // 1.2M + 0.806M voltage divider on VBAT = (1.2M / (0.806M + 2M)) 
// #define VBAT_DIVIDER_COMP (1.67166667F)        // Compensation factor for the VBAT divider = 1 / VBAT_DIVIDER
#define VBAT_DIVIDER_COMP (1.70583495F)

#define SAMPLES_IN_BUFFER 1


uint16_t BatteryMonitor::m_milliVolts = 0;
uint8_t BatteryMonitor::m_percentage = 0;

/*
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(2);
static nrf_saadc_value_t     m_buffer[SAMPLES_IN_BUFFER];
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
*/


BatteryMonitor::BatteryMonitor()
{
	m_bInitialized = false;
	m_bSAADCInitialized = false;
}


void BatteryMonitor::Initialize(uint16_t samplingPeriod)
{
	m_samplingPeriod = samplingPeriod;
	m_bInitialized = true;
}



void BatteryMonitor::PowerOn()
{
}



void BatteryMonitor::PowerOff()
{
}



void BatteryMonitor::StopSampling()
{
	/*
	if (m_bInitialized)
	{		
		ret_code_t err_code = nrf_drv_ppi_channel_disable(m_ppi_channel);
		APP_ERROR_CHECK(err_code);
		nrf_drv_timer_disable(&m_timer);
	}
	*/
}



void BatteryMonitor::StartSampling()
{
	/*
	if (m_bInitialized)
	{
		nrf_drv_timer_enable(&m_timer);
		ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);
		APP_ERROR_CHECK(err_code);
	}
	*/
}



void BatteryMonitor::GetInfo(uint16_t *pMV, uint8_t *pPct)
{
	if (pMV != NULL)
		*pMV = m_milliVolts;
	if (pPct != NULL)
		*pPct = m_percentage;
}



void BatteryMonitor::SingleSampleRequest()
{
	ret_code_t err_code;
	nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;     //Set internal reference of fixed 0.6 volts
	channel_config.gain = NRF_SAADC_GAIN1_6;    			     //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channel_config.acq_time = NRF_SAADC_ACQTIME_10US;            //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS. 
	channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;           //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	channel_config.pin_p = NRF_SAADC_INPUT_AIN0;                 //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
	channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;             //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
	channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;     //Disable pullup resistor on the input pin
	channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;     //Disable pulldown resistor on the input pin
	
	nrf_drv_saadc_config_t saadc_config;
	saadc_config.low_power_mode = false;                       //Enable low power mode.
	saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;     //Set SAADC resolution to 12-bit. This will make the SAADC output values from 0 (when input voltage is 0V) to 2^12=2048 (when input voltage is 3.6V for channel gain setting of 1/6).
	saadc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;     // NRF_SAADC_OVERSAMPLE_4X;     //Set oversample to 4x. This will make the SAADC output a single averaged value when the SAMPLE task is triggered 4 times.
	saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;    //Set SAADC interrupt to low priority.
		
	err_code = nrf_drv_saadc_init(&saadc_config, SAADC_Callback);
	APP_ERROR_CHECK(err_code);
		
	err_code = nrf_drv_saadc_channel_init(0, &channel_config);
	APP_ERROR_CHECK(err_code);

	nrf_saadc_value_t adcValue;
	ret_code_t ret_code = nrf_drv_saadc_sample_convert(0, &adcValue);
	if (ret_code == NRF_SUCCESS)
	{			
		m_milliVolts = (uint16_t)((float) adcValue * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP);	 
		m_percentage = VoltToPercent(m_milliVolts);
		NRF_LOG_INFO("MV %d - %dpct", m_milliVolts, m_percentage);
	}
	
	nrf_drv_saadc_abort();
	nrf_drv_saadc_uninit();
	while (nrf_drv_saadc_is_busy()) ;
}
	

void BatteryMonitor::SAADC_Init()
{
	if (!m_bSAADCInitialized)
	{
		ret_code_t err_code;
		nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
		channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;   //Set internal reference of fixed 0.6 volts
		channel_config.gain = NRF_SAADC_GAIN1_6;  			     //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
		channel_config.acq_time = NRF_SAADC_ACQTIME_10US;          //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS. 
		channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;         //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
		channel_config.pin_p = NRF_SAADC_INPUT_AIN0;               //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
		channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;           //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
		channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;   //Disable pullup resistor on the input pin
		channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;   //Disable pulldown resistor on the input pin
	
		nrf_drv_saadc_config_t saadc_config;
		saadc_config.low_power_mode = true;                     //Enable low power mode.
		saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;   //Set SAADC resolution to 12-bit. This will make the SAADC output values from 0 (when input voltage is 0V) to 2^12=2048 (when input voltage is 3.6V for channel gain setting of 1/6).
		saadc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;   // NRF_SAADC_OVERSAMPLE_4X;     //Set oversample to 4x. This will make the SAADC output a single averaged value when the SAMPLE task is triggered 4 times.
		saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;  //Set SAADC interrupt to low priority.
		
		err_code = nrf_drv_saadc_init(&saadc_config, SAADC_Callback);
		APP_ERROR_CHECK(err_code);
		
		err_code = nrf_drv_saadc_channel_init(0, &channel_config);
		APP_ERROR_CHECK(err_code);

		m_bSAADCInitialized = true;
	}
}



void BatteryMonitor::SAADC_UnInit()
{
	/*
	if (m_bSAADCInitialized)
	{
		nrf_drv_saadc_abort();
		nrf_drv_saadc_uninit();
		while (nrf_drv_saadc_is_busy()) ;
		m_bSAADCInitialized = false;
	}
	*/
}


void BatteryMonitor::SAADC_SamplingInit()
{
	/*
	ret_code_t err_code;

	err_code = nrf_drv_ppi_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
	err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, PPI_Timerhandler);
	APP_ERROR_CHECK(err_code);

	uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, m_samplingPeriod);
	nrf_drv_timer_extended_compare(&m_timer,
		NRF_TIMER_CC_CHANNEL0,
		ticks,
		NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
		false);
	nrf_drv_timer_enable(&m_timer);

	uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer, NRF_TIMER_CC_CHANNEL0);
	uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

	// setup ppi channel so that timer compare event is triggering sample task in SAADC 
	err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
		timer_compare_event_addr,
		saadc_sample_task_addr);
	APP_ERROR_CHECK(err_code);
	*/
}



void BatteryMonitor::PPI_Timerhandler(nrf_timer_event_t event_type, void * p_context)
{
}



void BatteryMonitor::SAADC_Callback(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
		{
			// Convert the raw value to compensated mv, taking the resistor-
			// divider into account (providing the actual LIPO voltage)
			// ADC range is 0..3000mV and resolution is 12-bit (0..4095),
			// VBAT voltage divider is 2M + 0.806M, which needs to be added back
			m_milliVolts = (uint16_t) ((float)p_event->data.done.p_buffer[i] * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP);	 
			m_percentage = VoltToPercent(m_milliVolts);
			NRF_LOG_INFO("MV %d - %dpct", m_milliVolts, m_percentage);
		}
		
		ret_code_t err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);
	}
}



/*
100.00	4.2
90		4.13
80		4.06	
70		3.99	
60		3.92	
50		3.85	
40		3.78
30		3.71	
20		3.64	
10		3.57	
0		3.5	*/
uint8_t BatteryMonitor::VoltToPercent(uint16_t mvolts) 
{
	if (mvolts >= 4200)
		return 100;
	else if (mvolts > 4130)
		return 90;
	else if (mvolts > 4060)
		return 80;
	else if (mvolts > 3990)
		return 70;
	else if (mvolts > 3920)
		return 60;
	else if (mvolts > 3850)
		return 50;
	else if (mvolts > 3780)
		return 40;
	else if (mvolts > 3710)
		return 30;
	else if (mvolts > 3640)
		return 20;
	else if (mvolts > 3570)
		return 10;
	return 0;
}


