/*
 * SPIMemory.c
 *
 *  Created on: May 2, 2014
 *      Author: fsoo
 */

#include "SPIMemory.h"

/*
 * SPIMemory.c
 * 
 */


QueueHandle_t TransferQueue;
xSemaphoreHandle xBlockReceivedSemaphore_0; // semaphore for Channel 0 transfers

// buffers

void SPIMemory_AddTransferToQueue(unsigned char channel, unsigned char* src, unsigned char* dest, unsigned long numbytes,xSemaphoreHandle semph)
{
	TransferRequest t;
	t.channel = channel;
	t.src = src;
	t.dest = dest;
	t.numbytes = numbytes;
	t.done_semaphore = semph;
	
	if(TransferQueue)
	{
		while(xQueueSendToBack(TransferQueue,( void * ) &t, (TickType_t) 0 )== errQUEUE_FULL)
		{
			vTaskDelay(10); // wait for a tick, see if queue clears up!
		}
	}
	
	
}

typedef struct {
  LDD_SPIMASTER_TError ErrFlag;        /* Error flags */
  bool RxDMABusy;                      /* Receiver DMA transfer is in progress */
  LDD_TDeviceData *RxDMATransfer;      /* Receiver DMA transfer component's data pointer */
  bool TxDMABusy;                      /* Transmitter DMA transfer is in progress */
  LDD_TDeviceData *TxDMATransfer;      /* Transmitter DMA transfer component's data pointer */
  LDD_DMA_TErrorFlags DMATransferErrors[2]; /* DMA transfer errors */
  LDD_TUserData *UserData;             /* User device data structure */
} SM1_TDeviceData;                     /* Device data structure type */

typedef SM1_TDeviceData* SM1_TDeviceDataPtr; /* Pointer to the device data structure */

portTASK_FUNCTION(SPITask, pvParameters)
{				
	// initialize SPI driver
	LDD_TDeviceData* SM1Ptr;
	
	SM1Ptr = SM1_Init(NULL);		
	
	
	// initialize DMA semaphore and memory transfer queue
	xBlockReceivedSemaphore_0 = xSemaphoreCreateBinary();
	TransferQueue = xQueueCreate( TQSIZE, sizeof(TransferRequest) );
	TransferRequest t;
	
	
#ifdef DEBUG_STACK_ON
	DEBUG_printStack("SPIMemory");
	uint16 count;
#endif

	for(;;)
	{
		// get the next transfer from the queue
		xQueueReceive( TransferQueue, &t, ( TickType_t ) portMAX_DELAY );
#ifdef DEBUG_STACK_ON
		count++;
		if(count%256==0)
		{
	//		DEBUG_printStack("SPIMemory");
		}
#endif
		
		if(t.channel ==MEMORY_CHANNEL)
		{
			
			  /* {FreeRTOS RTOS Adapter} Critical section begin (RTOS function call is defined by FreeRTOS RTOS Adapter property) */
			taskENTER_CRITICAL();
			MEMORY_CHIPSELECT_ASSERT;
		//	SPI0_DL = ((uint8_t*) t.src)[0];
			
			((SM1_TDeviceDataPtr)SM1Ptr)->RxDMABusy = TRUE;
			SM1_RxDMA_SetDestinationAddress(((SM1_TDeviceDataPtr)SM1Ptr)->RxDMATransfer, (LDD_DMA_TData*)t.dest);
			SM1_RxDMA_SetRequestCount(((SM1_TDeviceDataPtr)SM1Ptr)->RxDMATransfer, (LDD_DMA_TRequestCount) t.numbytes);
			if ((SPI_PDD_ReadStatusReg(SPI0_BASE_PTR) & SPI_PDD_RX_BUFFER_FULL) != 0U) {    
						(void)SPI_PDD_ReadData8bit(SPI0_BASE_PTR); // Dummy read of the data register 
			}
			
			SPI0_DL = ((uint8_t*) t.src)[0];
			if(t.numbytes >1)
			{
				((SM1_TDeviceDataPtr)SM1Ptr)->TxDMABusy = TRUE;
				  SM1_TxDMA_SetSourceAddress(((SM1_TDeviceDataPtr)SM1Ptr)->TxDMATransfer, ((LDD_DMA_TData*)t.src+1));
				  SM1_TxDMA_SetRequestCount(((SM1_TDeviceDataPtr)SM1Ptr)->TxDMATransfer, ((LDD_DMA_TRequestCount)t.numbytes-1));
				  SM1_RxDMA_EnableRequest(((SM1_TDeviceDataPtr)SM1Ptr)->RxDMATransfer);
				  SPI_PDD_EnableReceiveDma(SPI0_BASE_PTR, PDD_ENABLE);
				  SM1_TxDMA_EnableRequest(((SM1_TDeviceDataPtr)SM1Ptr)->TxDMATransfer);
				  SPI_PDD_EnableTransmitDma(SPI0_BASE_PTR, PDD_ENABLE);
			 }
			else
			{
				SM1_RxDMA_EnableRequest(((SM1_TDeviceDataPtr)SM1Ptr)->RxDMATransfer);
				SPI_PDD_EnableReceiveDma(SPI0_BASE_PTR, PDD_ENABLE);
									
			}
			
				  /* {FreeRTOS RTOS Adapter} Critical section ends (RTOS function call is defined by FreeRTOS RTOS Adapter property) */
			  taskEXIT_CRITICAL();			
			
			
			
			
		//	SM1_SendBlock(SM1Ptr,t.src, t.numbytes );
			// wait for transfer to finish
		//	fputs("waiting for transfer to finish\n",stdout);
			xSemaphoreTake( xBlockReceivedSemaphore_0, ( TickType_t ) portMAX_DELAY );
		
		//	MEMORY_CHIPSELECT_DEASSERT;
			// send message that transfer done
			if(t.done_semaphore != 0)
				xSemaphoreGive(t.done_semaphore);
		}
	
	}
}
void SPIManager_GetStatus(unsigned char channel, unsigned char* buffer, xSemaphoreHandle complete_semaphore)
{
	buffer[0] = 0x90;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = 0;
	buffer[5] = 0;
	SPIMemory_AddTransferToQueue(channel, buffer, buffer,6,complete_semaphore);
}
unsigned char SPIManager_ReadStatus(unsigned char* buffer)
{
	return buffer[5];
}

void SPIMemory_GetByte(unsigned char channel, uint32 addr24,unsigned char* buffer, xSemaphoreHandle complete_semaphore)
{
//	unsigned char buffer[5];
	buffer[0]=0x03;
	buffer[1]= ((addr24 >> 16)& 0x000000FF);
	buffer[2]= ((addr24 >> 8) & 0x000000FF);
	buffer[3]= (addr24 & 0x000000FF);
	buffer[4]= 0;
	
	SPIMemory_AddTransferToQueue(channel, buffer, buffer, 5,complete_semaphore);
	xSemaphoreTake(complete_semaphore,portMAX_DELAY);
//	return buffer[4];
}

char SPIMemory_ReadByteBuffer(unsigned char* buffer)
{
	return buffer[4];
}

void SPIMemory_Get16(unsigned char channel, uint32 addr24, unsigned char* buffer, xSemaphoreHandle complete_semaphore)
{
//	unsigned char buffer[6];
	buffer[0]=0x03;
	buffer[1]= ((addr24 >> 16)& 0x000000FF);
	buffer[2]= ((addr24 >> 8) & 0x000000FF);
	buffer[3]= (addr24 & 0x000000FF);
	buffer[4]= 0;
	buffer[5]= 0;
	
	SPIMemory_AddTransferToQueue(channel, buffer, buffer, 6, complete_semaphore);
	
	//return *((uint16*)&buffer[4]);
}
uint16 SPIMemory_Read16Buffer(unsigned char* buffer)
{
	return *((uint16*)&buffer[4]);	
}

void SPIMemory_Get32(unsigned char channel, uint32 addr24, unsigned char* buffer, xSemaphoreHandle complete_semaphore)
{
//	unsigned char buffer[8];
	buffer[0]=0x03;
	buffer[1]= ((addr24 >> 16)& 0x000000FF);
	buffer[2]= ((addr24 >> 8) & 0x000000FF);
	buffer[3]= (addr24 & 0x000000FF);
	buffer[4]= 0;
	buffer[5]= 0;
	buffer[6]= 0;
	buffer[7]= 0;
		
	
	SPIMemory_AddTransferToQueue(channel, buffer, buffer, 8, complete_semaphore);
}

uint32 SPIMemory_Read32Buffer(unsigned char* buffer)
{
	return *((uint32*)&buffer[4]);	
}




void SPIMemory_GetArray(unsigned char channel, uint32 addr24, unsigned char* buf_including_prepend, uint32 numbytes,
		xSemaphoreHandle complete_semaphore) // need to prepend 4 bytes to buf to accomodate instruction 
{
	buf_including_prepend[0]=0x03;
	buf_including_prepend[1]= ((addr24 >> 16)& 0x000000FF);
	buf_including_prepend[2]= ((addr24 >> 8) & 0x000000FF);
	buf_including_prepend[3]= (addr24 & 0x000000FF);
	SPIMemory_AddTransferToQueue(channel, buf_including_prepend, buf_including_prepend, numbytes, complete_semaphore);
	
//	return buf_including_prepend+4; // returns array without offset
	
}

void SPIMemory_WriteArray(unsigned char channel, uint32 addr24, unsigned char* buf_including_prepend, uint32 numbytes) // need to prepend 4 bytes to buf to accomodate instruction 
{
	static uint8 writeenable = 0x06; // write enable
	static uint8 writeenabledump;
	xSemaphoreHandle write_complete;
	write_complete = xSemaphoreCreateBinary();

	
	buf_including_prepend[0]=0x02;
	buf_including_prepend[1]= ((addr24 >> 16)& 0x000000FF);
	buf_including_prepend[2]= ((addr24 >> 8) & 0x000000FF);
	buf_including_prepend[3]= (addr24 & 0x000000FF);

	SPIMemory_AddTransferToQueue(channel, &writeenable, &writeenabledump,1, NULL); // enable write
	
	SPIMemory_AddTransferToQueue(channel, buf_including_prepend, buf_including_prepend, numbytes, write_complete); // write to memory	
	
	
	
	unsigned char busyquery[2];
	unsigned char busyquerydump[2];
	busyquerydump[0]=0;
	busyquerydump[1]=0;
	busyquery[0] = 0x05;
	busyquery[1] =0;
	
	do {
		SPIMemory_AddTransferToQueue(channel,busyquery, busyquerydump,2,write_complete);
		xSemaphoreTake(write_complete, portMAX_DELAY);	

	}
	while(busyquerydump[1]!=0);

	vSemaphoreDelete(write_complete);
		
}


void SPIMemory_GetManufacturer(unsigned char* manbuf, xSemaphoreHandle complete_semaphore)
{
	manbuf[0]= 0xAB;
	manbuf[1]= 0x0;
	manbuf[2]= 0x0;
	manbuf[3]= 0x0;
	manbuf[4]= 0x0;
	manbuf[5]= 0x0;

	SPIMemory_AddTransferToQueue(MEMORY_CHANNEL, manbuf, manbuf,6, complete_semaphore);// enable write
	
}

unsigned char* SPIMemory_ReadArray(unsigned char* buffer)
{
	return buffer; 	
}


void SPIMemory_Erase(unsigned char channel, uint8 size, uint32 addr24)//, xSemaphoreHandle complete_semaphore)
{
	static xSemaphoreHandle transfer_complete;
	static uint8 writeenable = 0x06; // write enable
	static uint8 writeenabledump;
	volatile uint8 busyquery[2] = {0x05,0}; // status register
	volatile uint8 busyquerydump[2] = {0,0}; // status register

	transfer_complete = xSemaphoreCreateBinary();
	unsigned char erase_command[4];
	switch (size)
	{
		case SECTOR_ERASE:
			erase_command[0] = 0x20;
		break;
		case BLOCK32K_ERASE:
			erase_command[0] = 0x52;
		break;
		case BLOCK64K_ERASE:
			erase_command[0] = 0xD8;
		break;
		case CHIP_ERASE:
			erase_command[0] = 0xC7;
		break;
		default:
			erase_command[0]=0x20;
	}
	
	
	erase_command[1] = (addr24>>16) & 0x0000FF;;
	erase_command[2] = (addr24>>8) & 0x0000FF;;
	erase_command[3] = (addr24) & 0x0000FF;
	
	SPIMemory_AddTransferToQueue(channel, &writeenable, &writeenabledump,1, transfer_complete);
	xSemaphoreTake(transfer_complete, portMAX_DELAY);	

	if(size == CHIP_ERASE)
	{
		SPIMemory_AddTransferToQueue(channel, erase_command, erase_command, 1, transfer_complete);
		
	}
	else
	{
		SPIMemory_AddTransferToQueue(channel, erase_command, erase_command, 4, transfer_complete);
	}
	xSemaphoreTake(transfer_complete, portMAX_DELAY);

	do {
		busyquerydump[0]=0;
		busyquerydump[1]=0;
		busyquery[0] = 0x05;
		busyquery[1] =0;
		SPIMemory_AddTransferToQueue(channel,busyquery, busyquerydump,2,transfer_complete);
		xSemaphoreTake(transfer_complete, portMAX_DELAY);	

	}
	while(busyquerydump[1]!=0);
	fputs("memory erase received done semaphore\n",stdout);
			
	vSemaphoreDelete(transfer_complete); 
	
}

/*
portTASK_FUNCTION(SPITest, pvParameters)
{
	xSemaphoreHandle complete_semaphore;
	complete_semaphore = xSemaphoreCreateBinary();
	for(;;)
	{
		uint32 offset = 0;
			
		unsigned int version;
		unsigned int numsounds;
		
		DEBUG_printf("SPITest:\n");
		// Sounds located in SPI memory
		DEBUG_printf("Loading sound table:\r\n");
		
		SPIGet16(offset, kSPIBUF, complete_semaphore, version);
		offset+= sizeof(uint16);
			
		DEBUG_printf("version: %u\r\n", version);	
			
		SPIMemory_Get16(offset, kSPIBUF,complete_semaphore);
		xSemaphoreTake(complete_semaphore, portTICK_PERIOD_MS);
		numsounds = SPIMemory_ReadByteBuffer(kSPIBUF);
		offset+= sizeof(uint16);
					
		DEBUG_printf("numsounds: %u\r\n", numsounds);	
							
			
		vTaskDelay(1000/portTICK_PERIOD_MS);
		
		
	}
	
}*/

