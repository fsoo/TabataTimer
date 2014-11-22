/*
 * SoundController.c
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#include "Sound.h"


QueueHandle_t kSoundQueue;
unsigned char playsound_buffer[2][BYTES_PER_PAGE + PRE_BUFFER];

//__attribute__ ((aligned (512))); // alignment for DMA
xSemaphoreHandle SPI_complete_semaphore; 
xSemaphoreHandle DAC_complete_semaphore;


void AddSoundToQueue(uint16 sound_id, uint16 numsamples, unsigned char gain, unsigned char enable)
{
	SoundMessage s;
	s.id = sound_id;
	s.numsamples = numsamples;
	s.gain = gain;
	s.enable = enable;
	
	if(kSoundQueue)
	{
		
		xQueueSendToBack(kSoundQueue,( void * ) &s, (TickType_t) 0 );
	}
	
	
}

uint16 GetSoundQueueLength()
{
	return uxQueueMessagesWaiting(kSoundQueue);
	
}

uint32 nbytes;
uint32 nsamples;
unsigned char* play_address;
LDD_TDeviceData* DMA_DeviceData;
	
portTASK_FUNCTION(SoundPlayerTask, pvParameters)
{
	SPI_complete_semaphore = xSemaphoreCreateBinary();
	DAC_complete_semaphore = xSemaphoreCreateBinary();
	xSemaphoreHandle I2C_complete_semaphore;
	I2C_complete_semaphore = xSemaphoreCreateBinary();
	kSoundQueue = xQueueCreate(SOUND_QUEUE_SIZE, sizeof(SoundMessage));
	
	LDD_TDeviceData* AudioTimer_DeviceData;
	
	SoundMessage msg;
	uint8 current_buffer = 0;
	
	DEBUG_printf("initializing Timer, DA1 and DMA\r\n");
	// initialize Timer, DA1 and DMA
//	AudioTimer_DeviceData = AudioTimer_Init(NULL);
//	TPM1_SC = (TPM_SC_DMA_MASK  | TPM_SC_TOIE_MASK | TPM_SC_CMOD(0x01) | TPM_SC_PS(0x00)); /* Set up status and control register */
//	DA1_DeviceData = DA1_Init(NULL);
	DMA_DeviceData = SSI1_TxDMA_Init(NULL);
	SSI1_DeviceData = SSI1_Init(NULL);
	uint32 numbytes;
	uint16 numpages;
	uint16 leftover;
	uint16 buf[2];
	buf[0]= 2048;
	buf[1]=2048;
	// turn off amp
	AmpEnable_PutVal(0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
		
	// start up SGTL5000
	uint8 i2cbuf[4];
				
	LED_PutVal(1);
	SGTL5000_GetChipID(i2cbuf, I2C_complete_semaphore);		
	LED_PutVal(0);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	SGTL5000_Init(I2C_complete_semaphore);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	SGTL5000_UsePassthrough(I2C_complete_semaphore);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	SGTL5000_UseMixer(I2C_complete_semaphore);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	kCurrentVolume = 10; // start at max volume
	SGTL5000_SetVolume(kCurrentVolume, I2C_complete_semaphore);
	vTaskDelay(10 / portTICK_PERIOD_MS);


	
//	DEBUG_printStack("SoundPlayer0");
	for(;;)
	{
		TickType_t start_tick;
		xQueueReceive(kSoundQueue, &msg, portMAX_DELAY); // check if there is a sound in the queue
		// prime the first buffer and wait for it to be done
		// calculate number of bytes, number of pages and leftover at end
		numbytes = (msg.numsamples ==0  ? kSoundTable.numsamples[msg.id]: msg.numsamples)*SAMPLE_BYTES; // total number of bytes to transfer
 		
		numpages = numbytes / BYTES_PER_PAGE;
		leftover = numbytes - (numpages * BYTES_PER_PAGE);
		
		// fill the first buffer with first page
		nbytes = (numpages>0 ? BYTES_PER_PAGE : leftover);
		nsamples = (numpages>0 ? BYTES_PER_PAGE/(sizeof(int16)) : leftover / sizeof(int16));  
				
		SPIMemory_GetArray(
				MEMORY_CHANNEL,
				kSoundTable.sound_addresses[msg.id],
				playsound_buffer[current_buffer], 
				nbytes+PRE_BUFFER, // if less than one page, only get leftover
				SPI_complete_semaphore);
		
		start_tick = xTaskGetTickCount();
	//	DA1_SetBuffer(DA1_DeviceData, buf, 2, 0 );
		xSemaphoreTake(SPI_complete_semaphore, portMAX_DELAY);
/*		if(xSemaphoreTake(SPI_complete_semaphore, 10)==pdFALSE)
		{
			DEBUG_printf("SPI A: %lu ticks\r\n",xTaskGetTickCount()-start_tick);  // wait for sound to finish playing

		}
	*/	
		
		AmpEnable_PutVal(msg.enable);	
		AmpGain_PutVal(msg.gain);
		AmpGain2_PutVal(msg.gain);
				
		// begin playing buffer
		taskENTER_CRITICAL();
		SSI1_SendBlock(SSI1_DeviceData, playsound_buffer[current_buffer]+4, nsamples); 
		SSI1_EnableTransfer(SSI1_DeviceData, LDD_SSI_TRANSMITTER);
				
	//	SSI1_TxDMA_DisableRequest(DMA_DeviceData);
	//	SSI1_TxDMA_SetSourceAddress(DMA_DeviceData, playsound_buffer[current_buffer]+4); // remember to offset by front padding
	//	SSI1_TxDMA_SetRequestCount(DMA_DeviceData, nbytes/SAMPLE_BYTES);
	//	SSI1_TxDMA_EnableRequest(DMA_DeviceData);
		taskEXIT_CRITICAL();
		// start I2S
		
		//SSI1_EnableTransfer();
		
//		I2S_PDD_EnableTxDataChannel(I2S0_BASE_PTR, PDD_ENABLE); /* Enable Tx data channel */
//		I2S_PDD_ClearTxInterruptFlags(I2S0_BASE_PTR, I2S_PDD_ALL_INT_FLAG); /* Clear all Tx interrupt flags */
	//	    ((SSI1_TDeviceDataPtr)DeviceDataPtr)->EnTransfer |= LDD_SSI_TRANSMITTER; /* Set enable transmitter flag */
		    /* Enable Tx interrupts */
//		I2S_PDD_EnableTxInterrupt(I2S0_BASE_PTR,(I2S_PDD_SYNC_ERROR_INT|I2S_PDD_FIFO_ERROR_INT));
//		I2S_PDD_EnableTxDevice(I2S0_BASE_PTR,PDD_ENABLE); /* Enable transmitter */
		
		
		play_address = 0; // will stop playing if this is the only page
		
		uint16 page;
		for(page = 1; page < numpages+1; page++) // including last page
		{
			nbytes = (page <numpages ? BYTES_PER_PAGE : leftover);
			nsamples = (page <numpages  ? BYTES_PER_PAGE/(sizeof(int16)) : leftover / (sizeof(int16)));  // different than number of bytes, usedby I2S callback
			

			if(nbytes > 0)
			{	
				// fill next buffer, make sure it is complete
				current_buffer = (current_buffer+1)%2;
				play_address = playsound_buffer[current_buffer]+PRE_BUFFER;
								
				SPIMemory_GetArray(
								MEMORY_CHANNEL,
								kSoundTable.sound_addresses[msg.id]+page*BYTES_PER_PAGE,
								playsound_buffer[current_buffer], 
								nbytes+PRE_BUFFER, 
								SPI_complete_semaphore);
				start_tick = xTaskGetTickCount();
			
				//	xSemaphoreTake(SPI_complete_semaphore, portMAX_DELAY);
				//	if((dt = xTaskGetTickCount()-start_tick) > 2)
				xSemaphoreTake(SPI_complete_semaphore, portMAX_DELAY);			
		/*		if(xSemaphoreTake(SPI_complete_semaphore, 10/portTICK_PERIOD_MS)==pdFALSE)
				{
					taskENTER_CRITICAL();
					fputs("timeout in getting from SPI\n", stdout);
					taskEXIT_CRITICAL();
//					DEBUG_printf("SPI: %lu ticks\r\n",xTaskGetTickCount()-start_tick);  // wait for sound to finish playing

				}
				*/
				start_tick = xTaskGetTickCount();
			//	xSemaphoreTake(DAC_complete_semaphore, portMAX_DELAY);
				
				if(xSemaphoreTake(DAC_complete_semaphore, 20/portTICK_PERIOD_MS)==pdFALSE)
				{
					fputs("timeout in DAC\n", stdout);
					for(;;);
				/*	taskENTER_CRITICAL();
										
					SSI1_TxDMA_DisableRequest(DMA_DeviceData);
					SSI1_TxDMA_SetSourceAddress(DMA_DeviceData, play_address); // remember to offset by front padding
					SSI1_TxDMA_SetRequestCount(DMA_DeviceData, nbytes/SAMPLE_BYTES);
					SSI1_TxDMA_EnableRequest(DMA_DeviceData);
					taskEXIT_CRITICAL();
				*/
				}
				

			}
		}
		play_address = 0; // tell interrupt that sounds are done.
		// wait until sound is done
		
	//	xSemaphoreTake(DAC_complete_semaphore, portMAX_DELAY);
		
		start_tick = xTaskGetTickCount();
		if(xSemaphoreTake(DAC_complete_semaphore, 20/portTICK_PERIOD_MS)==pdFALSE)
		{
			fputs("timeout in DAC2\n", stdout);
			for(;;);					
		}
		
		SSI1_DisableTransfer(SSI1_DeviceData, LDD_SSI_TRANSMITTER);
				
		// if no other sound available, shut down amp
		if(xQueuePeek(kSoundQueue, &msg, 0)==pdFALSE)
		{
			
			AmpEnable_PutVal(0);
			AmpGain_PutVal(0);
			AmpGain2_PutVal(0);

		}
//		DA1_SetBuffer(DA1_DeviceData, buf, 2, 0 );
	
	}
	
}

portTASK_FUNCTION(LoadSoundTable, pvParameters)
{
	xSemaphoreHandle complete_semaphore;
	complete_semaphore = xSemaphoreCreateBinary();
	
	
	//DEBUG_printHeap();
	for(;;)
	{
		uint32 offset = 0;
		uint16 i;	
		
	//	DEBUG_printf("Loading sound table:\r\n");
		
		SPIGet16(MEMORY_CHANNEL, offset, kSPIBUF, complete_semaphore, kSoundTable.version);
		offset+= sizeof(uint16);
			
	//	DEBUG_printf("version: %u\r\n", kSoundTable.version);	

		SPIGet16(MEMORY_CHANNEL, offset, kSPIBUF, complete_semaphore, kSoundTable.numsounds);
		offset+= sizeof(uint16);
	//	DEBUG_printf("numsounds: %u\r\n", kSoundTable.numsounds);	
							
		// allocate tables for sounds

	//	DEBUG_printf("Allocating data for table: \r\n");

		kSoundTable.numsamples = pvPortMalloc(sizeof(uint32)*kSoundTable.numsounds);
		kSoundTable.sound_addresses = pvPortMalloc(sizeof(uint32)*kSoundTable.numsounds);
		kSoundTable.strings= pvPortMalloc(sizeof(char*)*kSoundTable.numsounds);
		kSoundTable.stringbufs= pvPortMalloc(sizeof(char*)*kSoundTable.numsounds);
						
		for(i =0 ;i < kSoundTable.numsounds; i++)
		{
			kSoundTable.stringbufs[i] = pvPortMalloc(sizeof(char)*SOUND_MAX_NAME_LENGTH+PREPADDING);
			kSoundTable.strings[i] = kSoundTable.stringbufs[i]+PREPADDING;
		}
		
	//	DEBUG_printf("Loading table: \r\nNumber of samples: \r\n");
		for(i=0; i < kSoundTable.numsounds;i++)
		{
			SPIGet32(MEMORY_CHANNEL, offset, kSPIBUF, complete_semaphore, kSoundTable.numsamples[i]);
			offset+= sizeof(uint32);
			DEBUG_printf("%lu\r\n", kSoundTable.numsamples[i]);
		}
		
		/*
		 * 
		 * load strings
		 */
		for(i=0; i < kSoundTable.numsounds;i++)
		{
			SPIGetArray(MEMORY_CHANNEL, offset,kSoundTable.stringbufs[i], SOUND_MAX_NAME_LENGTH, complete_semaphore,kSoundTable.stringbufs[i] ); // returns unpadded array
			offset+= SOUND_MAX_NAME_LENGTH;
	//		DEBUG_printf("name: %s id: %u\r\n", kSoundTable.strings[i],i);
		}

		// calculate addresses
		
		for(i=0;i<kSoundTable.numsounds; i++)
		{
			kSoundTable.sound_addresses[i] = offset;
			offset += kSoundTable.numsamples[i] * SAMPLE_BYTES;	
			DEBUG_printf("address: %lu\r\n", kSoundTable.sound_addresses[i]);
			
		}
		
	//	DEBUG_printf("sounds loaded!\r\n");
		
		vTaskDelete(NULL); // done!
		
		/*
		vTaskDelay(100/portTICK_PERIOD_MS);
		// clean up
		
		
		for(i =0 ;i < kSoundTable.numsounds; i++)
		{
			vPortFree(kSoundTable.stringbufs[i]); // remember that the pointers are offset by prepadding
		}
		vPortFree(kSoundTable.numsamples);
		vPortFree(kSoundTable.sound_addresses);
		vPortFree(kSoundTable.strings);
		vPortFree(kSoundTable.stringbufs);
				
			*/	
	}
}

void IncreaseVolume(xSemaphoreHandle complete_semaphore)
{
	if(kCurrentVolume <10)
	{
		kCurrentVolume++;
		SGTL5000_SetVolume(kCurrentVolume, complete_semaphore);
	}
	else
	{
		xSemaphoreGive(complete_semaphore);
	}
}

void DecreaseVolume(xSemaphoreHandle complete_semaphore)
{
	if(kCurrentVolume >0)
	{
		kCurrentVolume--;
		SGTL5000_SetVolume(kCurrentVolume, complete_semaphore);
	}
	else
	{
		xSemaphoreGive(complete_semaphore);		
	}
}



