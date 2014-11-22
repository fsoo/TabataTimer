/*
 * TestCode.c
 *
 *  Created on: Jun 10, 2014
 *      Author: fsoo
 */
#include "TestCode.h"


//const uint32 _buzzer_numsamples = 128;
//const uint16 _buzzer[] __attribute__ ((aligned (512))) = {2048, 2148, 2249, 2348, 2447, 2545, 2642, 2738, 2831, 2923, 3013, \
		3100, 3185, 3267, 3347, 3423, 3495, 3565, 3630, 3692, 3750, 3804, \
		3853, 3898, 3939, 3975, 4007, 4034, 4056, 4073, 4085, 4093, 4095, \
		4093, 4085, 4073, 4056, 4034, 4007, 3975, 3939, 3898, 3853, 3804, \
		3750, 3692, 3630, 3565, 3495, 3423, 3347, 3267, 3185, 3100, 3013, \
		2923, 2831, 2738, 2642, 2545, 2447, 2348, 2249, 2148, 2048, 1948, \
		1847, 1748, 1649, 1551, 1454, 1358, 1265, 1173, 1083, 996, 911, 829, \
		749, 673, 601, 531, 466, 404, 346, 292, 243, 198, 157, 121, 89, 62, \
		40, 23, 11, 3, 1, 3, 11, 23, 40, 62, 89, 121, 157, 198, 243, 292, \
		346, 404, 466, 531, 601, 673, 749, 829, 911, 996, 1083, 1173, 1265, \
		1358, 1454, 1551, 1649, 1748, 1847, 1948};

const uint32 _buzzer_numsamples = 128;
/*const uint16 _buzzer[] = {2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, \
		2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, \
		3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, \
		601, 1, 601, 2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, \
		4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, 601, 1, \
		601, 2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, \
		3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, 601, 1, 601, \
		2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, \
		2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, 601, 1, 601, 2048, \
		3495, 4095, 3495, 2048, 601, 1, 601, 2048, 3495, 4095, 3495, 2048, \
		601, 1, 601};
*/
const int16 _buzzer[] = {0, 0, 3211, 3211, 6392, 6392, 9511, 9511, 12539, 12539, 15446, \
		15446, 18204, 18204, 20787, 20787, 23170, 23170, 25329, 25329, 27245, \
		27245, 28898, 28898, 30273, 30273, 31356, 31356, 32137, 32137, 32609, \
		32609, 32767, 32767, 32609, 32609, 32137, 32137, 31356, 31356, 30273, \
		30273, 28898, 28898, 27245, 27245, 25329, 25329, 23170, 23170, 20787, \
		20787, 18204, 18204, 15446, 15446, 12539, 12539, 9511, 9511, 6392, \
		6392, 3211, 3211, 0, 0, -3212, -3212, -6393, -6393, -9512, -9512, \
		-12540, -12540, -15447, -15447, -18205, -18205, -20788, -20788, \
		-23171, -23171, -25330, -25330, -27246, -27246, -28899, -28899, \
		-30274, -30274, -31357, -31357, -32138, -32138, -32610, -32610, \
		-32768, -32768, -32610, -32610, -32138, -32138, -31357, -31357, \
		-30274, -30274, -28899, -28899, -27246, -27246, -25330, -25330, \
		-23171, -23171, -20788, -20788, -18205, -18205, -15447, -15447, \
		-12540, -12540, -9512, -9512, -6393, -6393, -3212, -3212};
                     
#define NUMREPEATS 500
extern unsigned char debug_strbuf[DEBUG_STRBUF_SIZE];

#define TestTask_printnum(str,num) do{\
				taskENTER_CRITICAL(); \
				fputs(str,stdout); \
				UTIL1_Num16uToStr(debug_strbuf, DEBUG_STRBUF_SIZE, num); \
				fputs(debug_strbuf,stdout); \
				putc('\n',stdout); \
				taskEXIT_CRITICAL();}while(0)

#define TestTask_printmessage(str) do{\
				taskENTER_CRITICAL(); \
				puts(str); \
				taskEXIT_CRITICAL();}while(0)


#define LED_PERIOD 1000
#define MAX_WAIT_TIME portMAX_DELAY
#define DISP_STRBUF_SIZE 16
#define SOUND_OFFSET 8192
unsigned char disp_strbuf[DISP_STRBUF_SIZE];



portTASK_FUNCTION(TestTask, pvParameters)
{
	LED_PutVal(1);
	//enableRTC();
	
	uint16 i=0;
	xSemaphoreHandle complete_semaphore;
	unsigned char src[64];
	unsigned char dest[64];
	unsigned char manbuf[6];
	uint16 da1value[2];
	
	int16 encoder_position = 0;
	
	/*for(;;)
	{
				// flash LED and send message out UART
				// may need to use UTF-8m on teraterm?
			LED_NegVal();
			vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	*/
	
	complete_semaphore = xSemaphoreCreateBinary();

	
	
	// start the SPITask
	if (FRTOS1_xTaskCreate(
			  SPITask,  /* pointer to the task */
		   (unsigned portCHAR *)"SPITask", /* task name for kernel awareness debugging */
		   SPI_TASK_STACK_SIZE, /* task stack size */
		   (void*)NULL, /* optional task startup argument */
		   tskIDLE_PRIORITY+2,  /* initial priority */
		   (xTaskHandle*)NULL /* optional task handle to create */
		 ) != pdPASS) {
	   for(;;){}; /* Out of heap memory? */
	};
	
	// start the display task
	if(1)
	{
	if (FRTOS1_xTaskCreate(
			DisplayTask,  /* pointer to the task */
		   (unsigned portCHAR *)"DisplayTask", /* task name for kernel awareness debugging */
		   DISPLAY_TASK_STACK_SIZE, /* task stack size */
		   (void*)NULL, /* optional task startup argument */
		  tskIDLE_PRIORITY+1,  /* initial priority */
		   (xTaskHandle*)NULL /* optional task handle to create */
		 ) != pdPASS) {
	   for(;;){}; /* Out of heap memory? */
	};
	}
	
	
	// start button and UI tasks
	if (FRTOS1_xTaskCreate(
					  ButtonTask,  /* pointer to the task */
					 (unsigned portCHAR *)"ButtonTask", /* task name for kernel awareness debugging */
					 BUTTON_TASK_STACK_SIZE, /* task stack size */
					 (void*)NULL, /* optional task startup argument */
					tskIDLE_PRIORITY+1,  /* initial priority */
					 (xTaskHandle*)NULL /* optional task handle to create */
				   ) != pdPASS) {
				 for(;;){}; /* Out of heap memory? */
	};

	if(0)
	{
		if (FRTOS1_xTaskCreate(
						 LoadSoundTable,  /* pointer to the task */
					   (unsigned portCHAR *)"LoadSoundTable", /* task name for kernel awareness debugging */
					   configMINIMAL_STACK_SIZE, /* task stack size */
					   (void*)NULL, /* optional task startup argument */
					   tskIDLE_PRIORITY+1,  /* initial priority */
					   (xTaskHandle*)NULL /* optional task handle to create */
					 ) != pdPASS) {
				   for(;;){}; /* Out of heap memory? */
			  };
	}
	
	if(1)
	{
	 if (FRTOS1_xTaskCreate(
						  SoundPlayerTask,  /* pointer to the task */
						 (unsigned portCHAR *)"SoundPlayerTask", /* task name for kernel awareness debugging */
						 SOUNDPLAYER_TASK_STACK_SIZE, /* task stack size */
						 (void*)NULL, /* optional task startup argument */
						 tskIDLE_PRIORITY+2,  /* initial priority */
						 (xTaskHandle*)NULL /* optional task handle to create */
					   ) != pdPASS) {
					 for(;;){}; /* Out of heap memory? */
	};
	}
	 // start the I2CTask
	 	if (FRTOS1_xTaskCreate(
	 			  I2CTask,  /* pointer to the task */
	 		   (unsigned portCHAR *)"I2CTask", /* task name for kernel awareness debugging */
	 		   I2C_TASK_STACK_SIZE, /* task stack size */
	 		   (void*)NULL, /* optional task startup argument */
	 		   tskIDLE_PRIORITY+2,  /* initial priority */
	 		   (xTaskHandle*)NULL /* optional task handle to create */
	 		 ) != pdPASS) {
	 	   for(;;){}; /* Out of heap memory? */
	 	};
	 
	
	 	
	// create a queue for UI events	
	UIQueue = xQueueCreate( 4, sizeof(ButtonMessage) );
	ButtonMessage b;

	

	if(1)
	{
		// create a dummy sound table
		kSoundTable.numsounds = 1;
		kSoundTable.numsamples = pvPortMalloc(sizeof(uint32)*kSoundTable.numsounds);
		kSoundTable.sound_addresses = pvPortMalloc(sizeof(uint32)*kSoundTable.numsounds);
		kSoundTable.strings= pvPortMalloc(sizeof(char*)*kSoundTable.numsounds);
		kSoundTable.stringbufs= pvPortMalloc(sizeof(char*)*kSoundTable.numsounds);
				
		kSoundTable.numsamples[0]=_buzzer_numsamples*NUMREPEATS;
		kSoundTable.sound_addresses[0] = SOUND_OFFSET;
	}
	// wait for things to start
	vTaskDelay(100 / portTICK_PERIOD_MS);
		
	// see if I2C driver works
	
	uint8 i2cbuf[4];
	
	LED_PutVal(0);
	SGTL5000_GetChipID(i2cbuf, complete_semaphore);
	SGTL5000_Init(complete_semaphore);
	
//	for(;;)
//	{
		LED_PutVal(1);
		SGTL5000_UsePassthrough(complete_semaphore);
		vTaskDelay(100);
		LED_PutVal(0);
		SGTL5000_UseMixer(complete_semaphore);
		vTaskDelay(100);
		
						
//	}
	
	LED_PutVal(1);
	// write to chip
	
	// test manufacturer ID
	//TestTask_printmessage("Getting manufacturer ID");
	
	SPIMemory_GetManufacturer(manbuf,complete_semaphore);
	xSemaphoreTake(complete_semaphore, MAX_WAIT_TIME);
	
//	TestTask_printmessage("Waiting for manufacturer ID");
	/*		
	int k;
	for(k=0; k < 6; k++)
	{
		TestTask_printnum("manufacturer: ",manbuf[k]);
	}	
	
	*/
	
//	TestTask_printmessage("erasing chip:");
	SPIMemory_Erase(0, CHIP_ERASE, 0);
	
	TestTask_printmessage("writing sound to memory");

	unsigned char* samps;
	samps  = pvPortMalloc(sizeof(uint16)*_buzzer_numsamples+4);
	uint16 k;
	
	for(k=0;k<NUMREPEATS; k++)
	{	
		memcpy(samps+4, _buzzer, sizeof(uint16)*_buzzer_numsamples); // copy offset by 4 bytes
		SPIMemory_WriteArray(0,SOUND_OFFSET+(_buzzer_numsamples)*sizeof(uint16)*k,samps ,sizeof(uint16)*_buzzer_numsamples+4);
	}
	SPIMemory_GetArray(0,SOUND_OFFSET,samps ,sizeof(uint16)*_buzzer_numsamples+4,complete_semaphore);
	xSemaphoreTake( complete_semaphore, MAX_WAIT_TIME);
		
	vPortFree(samps);

	
	
	for(;;)
	{
		int k;
				
		// flash LED and send message out UART
		// may need to use UTF-8m on teraterm?
		LED_NegVal();
	//	TestTask_printnum("test=",i);
		i++;
		
		// test manufacturer ID
	//	TestTask_printmessage("Getting manufacturer ID");
		
	//	for(k=0;k<10;k++)
		//	TestTask_printmessage("Adding to queue");
		
//		SPIMemory_GetManufacturer(manbuf,complete_semaphore);
	//	TestTask_printmessage("Waiting for manufacturer ID");
				
	//	xSemaphoreTake(complete_semaphore, MAX_WAIT_TIME);
	//	for(k=0; k < 6; k++)
	//	{
	//		TestTask_printnum("manufacturer: ",manbuf[k]);
	//	}
		// test memory
	/*	TestTask_printmessage("erasing memory sector 1:");
		SPIMemory_Erase(MEMORY_CHANNEL, SECTOR_ERASE, 0);
		TestTask_printmessage("writing to memory");
		
		strcpy(src, "    Yeah, buddy!");
		TestTask_printmessage(src+4);
		
		SPIMemory_WriteArray(0,0, src, 13+4);		
		TestTask_printmessage("getting from memory");
		SPIMemory_GetArray(0,0,dest,13+4, complete_semaphore);
		if(xSemaphoreTake( complete_semaphore, MAX_WAIT_TIME)==pdFALSE)
		{

			TestTask_printmessage("timeout while waiting for memory read!");
			TestTask_printmessage(dest+4);			

		}
		else
		{
			TestTask_printmessage("memory read completed:");
			TestTask_printmessage(dest+4);			
		}
		
		SPIMemory_GetArray(0,SOUND_OFFSET+_buzzer_numsamples * sizeof(uint16)*(i%50),dest,64, complete_semaphore);
		xSemaphoreTake( complete_semaphore, MAX_WAIT_TIME);
	
		for(k = 0; k < 8; k++)
		{
			TestTask_printnum("sound: ", ((uint16*)(dest+4))[k]);
			
		}
		*/				
		// test display
		unsigned char disp_strbuf[DISP_STRBUF_SIZE];
		
	//	TestTask_printmessage("testing display");
							
		setTextSize(3);
		setCursorPosition(0,0);
		writeString("Yeah,\nbuddy!\n",13);
		
		setTextSize(2);
		UTIL1_Num16sToStrFormatted(disp_strbuf,DISP_STRBUF_SIZE, i,' ',2);
		writeString(disp_strbuf,strlen(disp_strbuf));	
		
	//	Display_Refresh();	
	
		if(xQueueReceive( UIQueue, & b, LED_PERIOD/portTICK_PERIOD_MS) )
		{
			setTextSize(1);
			setCursorPosition(0,90);
			
			writeString("button\n", 7);
			UTIL1_Num16sToStrFormatted(disp_strbuf,DISP_STRBUF_SIZE, b.buttonID,' ',2);
			writeString(disp_strbuf,strlen(disp_strbuf));		
			UTIL1_Num16sToStrFormatted(disp_strbuf,DISP_STRBUF_SIZE, b.buttonValue,' ',2);
			writeString(disp_strbuf,strlen(disp_strbuf));		
			writeString("encoder ", 9);
			UTIL1_Num16sToStrFormatted(disp_strbuf,DISP_STRBUF_SIZE, encoder_position,' ',2);
			writeString(disp_strbuf,strlen(disp_strbuf));		

//			UTIL1_Num16sToStrFormatted(disp_strbuf,DISP_STRBUF_SIZE, b.dt,' ',2);
	//		writeString(disp_strbuf,strlen(disp_strbuf));		
				
			
			if(b.buttonID==STARTSTOP_BUTTON && b.buttonValue==0)
			{
			//	AmpEnable_PutVal(1);

			//	AmpEnable_PutVal(1);
				for(k=0;k<1;k++)
				{
					AddSoundToQueue(0,0,1,1);
					
/*					AddSoundToQueue(kRadioShack1,0,1,1);
					AddSoundToQueue(kTone3KHz,0, 1, 1);
					AddSoundToQueue(kTone300Hz,0,1,1);*/
					
				}
			}				
			else
			{
			//	AmpEnable_PutVal(0);

			}
			
			if(b.buttonID==ENCODER_STEP)		
			{
				encoder_position += b.buttonValue;
				changeVolume(b.buttonValue);
				
			}
			
			
			
			
			
			
		}
//		vTaskDelay(LED_PERIOD/portTICK_PERIOD_MS);
	}
}



