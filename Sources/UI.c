/*
 * UI.c
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#include "UI.h"
#include "Interval.h"


PE_ISR(PortDInterrupt)
{
	taskENTER_CRITICAL();
	BaseType_t xHigherPriorityTaskWoken;
	if( xButtonSemaphore != 0 )
	{
		xSemaphoreGiveFromISR( xButtonSemaphore, &xHigherPriorityTaskWoken );
	}
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
//	PORT_PDD_ClearInterruptFlags(PTD_PORT_DEVICE,0);
	PORTD_ISFR = 0xffffffff;
	taskEXIT_CRITICAL();
}


 
portTASK_FUNCTION_PROTO(ButtonTask, pvParameters)
{
	static uint8 old_AB = 0;  //lookup table index
	static int8 encval = 0;   //encoder value  
	static const int8 enc_states [] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table

	
	ButtonMessage b;
	static unsigned char startstop_prev_button_val;
	static unsigned char startstop_button_val;
	TickType_t startstop_last_tick;
	TickType_t startstop_current_tick;

	
	static unsigned char mode_prev_button_val;
	static unsigned char mode_button_val;
	TickType_t mode_last_tick;
	TickType_t mode_current_tick;
	
	static unsigned char A_prev_button_val;
	static unsigned char A_button_val;
	TickType_t A_last_tick;
	TickType_t A_current_tick;
	
	static unsigned char B_prev_button_val;
	static unsigned char B_button_val;
	TickType_t B_last_tick;
	TickType_t B_current_tick;
		
	
	
	//DEBUG_printf("Starting ButtonTask\n");
	startstop_last_tick = xTaskGetTickCount();
	mode_last_tick = xTaskGetTickCount();
		
	
	/* Ensure the semaphore is created before it gets used. */
	vSemaphoreCreateBinary( xButtonSemaphore );
//	xSemaphoreTake( xButtonSemaphore, ( TickType_t ) 100 ); // clear the first semaphore, usually junk!
	DEBUG_printStack("Button");	
	
	for( ;; )
	{
		/* Block on the semaphore to wait for an interrupt event.  The semaphore
		is 'given' from vButtonISRHandler() below.  Using portMAX_DELAY as the
		block time will cause the task to block indefinitely provided
		INCLUDE_vTaskSuspend is set to 1 in FreeRTOSConfig.h. */
		xSemaphoreTake( xButtonSemaphore, ( TickType_t ) portMAX_DELAY );
	//	vTaskDelay( butDEBOUNCE_DELAY );
		
	//	xSemaphoreTake( xButtonSemaphore, 0 );
		startstop_button_val = StartStopButton_GetVal(StartStopButton_DeviceData); // get portd status
		mode_button_val = ModeButton_GetVal(ModeButton_DeviceData); // get portd status
		A_button_val = EncoderA_GetVal(EncoderA_DeviceData); // get portd status
		B_button_val = EncoderB_GetVal(EncoderB_DeviceData); // get portd status
	
		// keep track of encoder state
		old_AB <<=2;  //remember previous state
	    old_AB = old_AB |= (A_button_val + 2 * B_button_val);
	    encval += (uint16)enc_states[( old_AB & 0x0f )];

	    if( encval > 3 ) {  //four steps forward - send a message that this happened
	    	encval =0;
			b.buttonID = ENCODER_STEP;
			b.buttonValue = 1; // one step forward
			b.dt = 0;
			if(UIQueue  != 0)
				xQueueSendToBack(UIQueue,( void * ) &b, (TickType_t) 0 );	
		}
	    else if( encval < -3 ) {  //four steps forward - send a message that this happened
	    	encval =0;
			b.buttonID = ENCODER_STEP;
			b.buttonValue = -1; // 1 step reverse rotation
			b.dt = 0;
			if(UIQueue  != 0)
				xQueueSendToBack(UIQueue,( void * ) &b, (TickType_t) 0 );
			
		}	
		
		if(startstop_button_val != startstop_prev_button_val)  // transition
		{
			startstop_current_tick = xTaskGetTickCount();
			
			b.buttonID = STARTSTOP_BUTTON;
			b.buttonValue = startstop_button_val;
			b.dt = startstop_current_tick-startstop_last_tick;
			if(UIQueue  != 0)
			{
				xQueueSendToBack(UIQueue,( void * ) &b, (TickType_t) 0 );
		//		DEBUG_printf("transition: button %d, status %d, dt %lu\n", b.buttonID, b.buttonValue, b.dt);
		//		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		//		DEBUG_printf("ButtonTask highwater mark 1: %lu\r\n", uxHighWaterMark);
		//		DEBUG_printStack("Button2");
	
			}		
			startstop_last_tick = startstop_current_tick;
			startstop_prev_button_val = startstop_button_val;	
		}
		
		if(mode_button_val != mode_prev_button_val)  // transition
		{
			mode_current_tick = xTaskGetTickCount();
			
			b.buttonID = MODE_BUTTON;
			b.buttonValue = mode_button_val;
			b.dt = mode_current_tick-mode_last_tick;
			if(UIQueue  != 0)
			{
				xQueueSendToBack(UIQueue,( void * ) &b, (TickType_t) 0 );
			}		
			mode_last_tick = mode_current_tick;
			mode_prev_button_val = mode_button_val;	
		}
		
	}
}

portTASK_FUNCTION_PROTO(UITask, pvParameters)
{
	xSemaphoreHandle complete_semaphore;
	complete_semaphore = xSemaphoreCreateBinary();
	//vTaskSuspend( interval_task_handle );
	DEBUG_printf("Starting UITask\n");
			
	UIQueue = xQueueCreate( 4, sizeof(ButtonMessage) );
			
	ButtonMessage b;
	DEBUG_printStack("UITask");	

	for(;;)
	{
		if( UIQueue != 0 )
		{
		// Receive a message on the created queue.  Block indefinitely if message not immediately available
			if( xQueueReceive( UIQueue, & b, ( TickType_t ) portMAX_DELAY ) )
			{
				DEBUG_printf("button %d, status %d dt %lu\n", b.buttonID, b.buttonValue, b.dt);
				DEBUG_printStack("UITask1");	

				if(b.buttonID == STARTSTOP_BUTTON && b.buttonValue == STARTSTOP_BUTTON_UP)	
				{
					if(b.dt < butLONG_DURATION)
					{
						Interval_Toggle();
						DEBUG_printf("short off!\n");
					}	
					else
						DEBUG_printf("long off!\n");
				}
				else if(b.buttonID==ENCODER_STEP)		
				{
					if(b.buttonValue == 1)
					{
						// increment volume
						DecreaseVolume(complete_semaphore);
			//			xSemaphoreTake(complete_semaphore, portMAX_DELAY);
					}
					else
					{
						// decrease volume
						IncreaseVolume(complete_semaphore);
				//		xSemaphoreTake(complete_semaphore, portMAX_DELAY);
					}
					// if interval task not running, safe to draw to screen directly, otherwise let it refresh
					if(!Interval_Running())
					{
				//	getDisplayMutex(portMAX_DELAY);
						Draw_Master_Display();
					
				//	releaseDisplayMutex();
						Display_Refresh();
					}
				}
				else // see if user holds for a long time
				{
					if(xQueuePeek( UIQueue, & b, ( TickType_t ) butLONG_DURATION )==pdFALSE)
					{
						Interval_Reset();
						DEBUG_printf("long hold!\n");
					}					
				}
				
				
					
					
					
				
			}
		}
		
	}
}



#define VOL_DELAY 1
void changeVolume(int steps)
{
	int i;
	// U/D volume control for MCP4011/2/3/4

	//VolumeCS_PutVal(VolumeCS_DeviceData, 1); // start with pin high

	if(steps > 0)
	{
	//	VolumeUD_PutVal(VolumeUD_DeviceData, 1); // for increment, start with pin high
	//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
	//	VolumeCS_PutVal(VolumeCS_DeviceData, 0); // drop CS
					
		for(i=0; i < steps; i++)
		{
		//	VolumeUD_PutVal(VolumeUD_DeviceData,0);
		//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
		//	VolumeUD_PutVal(VolumeUD_DeviceData,1);
		//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
			
		}
	//	VolumeCS_PutVal(VolumeCS_DeviceData,1);
	}
	else if (steps < 0)
	{
	//	VolumeUD_PutVal(VolumeUD_DeviceData, 0); // for increment, start with pin low
	//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
	//	VolumeCS_PutVal(VolumeCS_DeviceData, 0); // drop CS
					
		for(i=0; i < (-steps); i++)
		{
		//	VolumeUD_PutVal(VolumeUD_DeviceData,0);
		//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
		//	VolumeUD_PutVal(VolumeUD_DeviceData,1);
		//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
			
		}
	//	VolumeUD_PutVal(VolumeUD_DeviceData,0);
	//	vTaskDelay(VOL_DELAY/portTICK_PERIOD_MS);
	//	VolumeCS_PutVal(VolumeCS_DeviceData,1);
	}
	
}
