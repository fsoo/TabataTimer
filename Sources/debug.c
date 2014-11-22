/*
 * debug.c
 *
 *  Created on: May 6, 2014
 *      Author: fsoo
 */


#include "debug.h"

unsigned char debug_strbuf[DEBUG_STRBUF_SIZE];
const char* heapstr = "heapsize: ";
const char* stackstr = " hw: ";



#ifdef DEBUG_TASK_ON

extern SoundTable kSoundTable;

portTASK_FUNCTION(DebugTask, pvParameters)
{
	// wait until sound table is initialized
	DEBUG_printf("Waiting for sound table initialization\r\n");
			
	while(kSoundTable.numsounds == 0)
	{
		vTaskDelay(1000/portTICK_PERIOD_MS);
		
	}
	DEBUG_printf("Starting DebugTask\r\n");
	for(;;)
	{
		uint16 i;
		
		for(i=0;i<kSoundTable.numsounds;i++)
		{
			DEBUG_printf("Adding sound %d: %s to queue\r\n", i, kSoundTable.strings[i]);
			AddSoundToQueue(i,0,0,1);
		}	
		while(GetSoundQueueLength() >0)
		{
			vTaskDelay(100/portTICK_PERIOD_MS);
		}
		
	}		
	
}
#endif
