/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


#define	GREEN_LED		LED4
#define RED_LED			LED3
#define BLUE_LED		LED5
#define ORANGE_LED		LED6


#define USED_LED		GREEN_LED
static uint32_t g_status;


void trigger_led_task(uint32_t l_info);
static void led_status_task(void *pv);
static TaskHandle_t g_led_task_handle;

void trigger_led_task(uint32_t l_info)
{
	if(g_led_task_handle == 0)
	{
		xTaskCreate(led_status_task, (const char*)"LED S T", configMINIMAL_STACK_SIZE, NULL, 1, &g_led_task_handle);		
	}
	else
	{
		g_status = l_info;
		if(l_info)
		{
			
			vTaskResume(g_led_task_handle);			
		}
		else
		{
			STM_EVAL_LEDOff(USED_LED);
			vTaskSuspend(g_led_task_handle);
		}
	}
	
}

static void led_status_task(void *pv)
{
	
	STM_EVAL_LEDInit(GREEN_LED);
	STM_EVAL_LEDInit(RED_LED);
	STM_EVAL_LEDInit(BLUE_LED);
	STM_EVAL_LEDInit(ORANGE_LED);

	g_status = 0;
	vTaskSuspend(NULL);
	do
	{
		STM_EVAL_LEDOn(USED_LED);
		vTaskDelay(500);
		STM_EVAL_LEDOn(USED_LED);
		vTaskDelay(1000);
		for(uint32_t i = 0; i < (g_status+1); i++)
		{
			STM_EVAL_LEDToggle(USED_LED);
			vTaskDelay(100);
		}
	}
	while(1);
}
