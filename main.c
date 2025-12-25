#include "stm32f4_discovery.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"


extern void task_manager_init(void);

int main(void)
{
	SystemCoreClockUpdate();
	task_manager_init();
	vTaskStartScheduler();
  
}
