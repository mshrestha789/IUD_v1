/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include <stdio.h>
#include <string.h> 
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"


typedef struct {
	volatile uint16_t * data1;
	double * data2;
	double * data3;
	double damage_index;
	uint32_t len;
}S_POST_INFO;

typedef void (*void_fnc_ptr)(uint32_t *info);
static void_fnc_ptr fire_communication_info;

static TaskHandle_t g_comm_task_handle;


static void communication_task(void *pv);

S_POST_INFO  g_post_info;

void trigger_user_communication_task(void (*communication_callback)(uint32_t *info), uint32_t len, volatile uint16_t * data1, double * data2, double * data3, double d_i)
{	
	if(g_comm_task_handle == 0)
	{
		//const size_t xMessageBufferSizeBytes = 1;
		fire_communication_info = communication_callback;
		xTaskCreate(communication_task, "ComTask", 300, NULL, configMAX_PRIORITIES, &g_comm_task_handle);
		
	}
	else
	{
		g_post_info.data1 = data1;
		g_post_info.data2 = data2;
		g_post_info.data3 = data3;
		g_post_info.len = len;
		g_post_info.damage_index = d_i;
		xTaskNotifyGive(g_comm_task_handle);
	}
}

static void communication_task(void *pv)
{
	uint32_t info;
	char rx_buffer[50];     // Buffer for incoming commands
	uint8_t rx_index = 0;   // Current buffer index

	usart1_init();
	
	info = 1;
	fire_communication_info(&info);
	
	// Clear buffer initially
	memset(rx_buffer, 0, sizeof(rx_buffer));

	do
	{
		// ---------------------------------------------------------
		// Command Parser Loop
		// ---------------------------------------------------------
		do
		{
			int16_t ch = usart1_get_char();
			
			// Check if we received a valid character (not 0)
			if(ch != 0) 
			{
				// Check for Line Endings (Process Command)
				if(ch == '\r' || ch == '\n')
				{
					rx_buffer[rx_index] = '\0'; // Null terminate string
					
					// 1. Check Device ID (Must start with "001")
					if(strncmp(rx_buffer, "001", 3) == 0)
					{
						// 2. Check Command (Must contain "pitch")
						if(strstr(rx_buffer, "pitch") != NULL)
						{
							info = 2;
							fire_communication_info(&info);
							rx_index = 0; // Reset buffer
							break;        // Break inner loop to start measurement
						}
					}
					
					// If format didn't match, just reset buffer and keep listening
					rx_index = 0;
					memset(rx_buffer, 0, sizeof(rx_buffer));
				}
				// Standard character processing
				else
				{
					if(rx_index < (sizeof(rx_buffer) - 1))
					{
						rx_buffer[rx_index++] = (char)ch;
					}
					else
					{
						// Buffer overflow protection: reset if too long
						rx_index = 0;
					}
				}
			}
			
			vTaskDelay(1); // Yield to other tasks
			
		} while(1);

		// ---------------------------------------------------------
		// Wait for processing to finish (Notification from Task Manager)
		// ---------------------------------------------------------
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		S_POST_INFO * p = &g_post_info;
		
		#if 0
		for(uint32_t i = 0; i < p->len; i++)
		{
			char s_adc_value[30];
			sprintf(s_adc_value, "%d, %f, %f, %f\r\n", p->data1[i], p->data2[i], p->data3[i], p->damage_index);
			usart1_send_string((int8_t *)s_adc_value);
			vTaskDelay(1);
		}
		#endif
		
		// Send Result
		char temp_buf[32]; // Increased size slightly for safety
		sprintf(temp_buf, "%f\r\n", p->damage_index);
		usart1_send_string((int8_t *)temp_buf);
		
		info = 3;
		fire_communication_info(&info);
		vTaskDelay(1);
	}
	while(1);
}