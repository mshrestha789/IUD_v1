#include "FreeRTOS.h"
#include "Task.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "data_wearhouse.h"
#include "usart1.h"
#include <stdio.h>
#include <string.h>

#define USER_COMMUNICATION_RDY_BIT	( 1 << 4 )
#define UART_TRIGGER_BIT			( 1 << 1 )
#define UART_GET_BIT				( 1 << 2 )
#define UART_TRANSFERRED_BIT		( 1 << 3 )



extern void trigger_led_task(uint32_t l_info);
extern void trigger_user_communication_task(void (*communication_callback)(uint32_t *info), uint32_t len, volatile uint16_t * data1, double * data2, double * data3, double d_i);
void pitch_catch_init(void);
void pitch_catch(__IO uint16_t *ADC3ConvertedValue, uint32_t adc_buff_len, const uint16_t *tone_burst, uint32_t tone_burst_len);
static void task_manager(void *pv);
void communication_listener(uint32_t *info);
extern void filter_lpf(double * raw_data, double * filter_data, uint32_t buff_size);


static EventGroupHandle_t g_task_m_event;


void task_manager_init(void)
{
	g_task_m_event = xEventGroupCreate();
	xTaskCreate(task_manager, "TskMng", 300, NULL, (configMAX_PRIORITIES-2), NULL);
}

static void task_manager(void *pv)
{
			
	trigger_led_task(0);
	vTaskDelay(1000);
	trigger_led_task(2);
	
	trigger_user_communication_task(&communication_listener,0, 0, 0, 0, 0);
	
	xEventGroupWaitBits( g_task_m_event, USER_COMMUNICATION_RDY_BIT, pdTRUE, pdFALSE, portMAX_DELAY );
	
	
	pitch_catch_init();
	memset(ultrasonic_wave_raw,0,sizeof(ultrasonic_wave_raw));
	memset(ultrasonic_wave_lpf,0,sizeof(ultrasonic_wave_lpf));
	
	
	while(1)
	{
		
		EventBits_t uxBits;
		//uxBits = xEventGroupWaitBits( g_task_m_event, UART_TRIGGER_BIT, pdTRUE, pdFALSE, 15000  / portTICK_RATE_MS);
		uxBits = xEventGroupWaitBits( g_task_m_event, UART_TRIGGER_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
		
		pitch_catch(ADC3ConvertedValue, ADC_BUFF_SIZE, tone_burst, DAC_BUFF_SIZE);
		
		uint32_t i;
		for(i = 0; i < ADC_BUFF_SIZE; i++)
		{
			ultrasonic_wave_raw[i] = (double)ADC3ConvertedValue[i] * 3.3/4095;
		}
		filter_lpf(ultrasonic_wave_raw, ultrasonic_wave_lpf, ADC_BUFF_SIZE);
		
		#if 1
		static uint32_t first_time_flag = 0;
		if(first_time_flag < 3)
		{
			memcpy(ultrasonic_wave_intact, ultrasonic_wave_lpf, ADC_BUFF_SIZE);
			first_time_flag += 1;
		}
		
		double sum1 = 0, sum2 = 0;
		for(i = 0; i < ADC_BUFF_SIZE; i++)
		{
			if(i > 70)
			{
				sum1 += ((ultrasonic_wave_lpf[i] - ultrasonic_wave_intact[i]) * (ultrasonic_wave_lpf[i] - ultrasonic_wave_intact[i]));
				sum2 += (ultrasonic_wave_intact[i] * ultrasonic_wave_intact[i]);
		
			}
		}
		
		di = sum1/sum2;
		#endif
		//filter
		//caculate
		if( ( uxBits & UART_TRIGGER_BIT ) != 0 )
		{
			
			trigger_led_task(4);
			trigger_user_communication_task(0, ADC_BUFF_SIZE, &ADC3ConvertedValue[0], &ultrasonic_wave_raw[0], &ultrasonic_wave_lpf[0], di);
			xEventGroupWaitBits( g_task_m_event, UART_TRANSFERRED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
			trigger_led_task(2);
		}
		//process_signal(&adc_data, &processed_data, &di);
		
		
		
		vTaskDelay(1);
	}
}

void communication_listener(uint32_t *info)
{
	switch(*info)
	{
		case 1:			
			xEventGroupSetBits( g_task_m_event, USER_COMMUNICATION_RDY_BIT );			
		break;
		case 2:
			xEventGroupSetBits( g_task_m_event, UART_TRIGGER_BIT );
		break;
		case 3:
			xEventGroupSetBits( g_task_m_event, UART_TRANSFERRED_BIT );
		break;
			
	}
	//vTaskDelay(1);
}
