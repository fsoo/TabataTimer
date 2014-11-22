/*
 * SPIDriver.c
 *
 *  Created on: Aug 25, 2014
 *      Author: fsoo
 */
#include "I2CDriver.h"


QueueHandle_t I2CTransferQueue;
xSemaphoreHandle I2CBlockSemaphore; // semaphore to signal completion of I2C transfer

typedef struct
{
	uint8 direction; // transmit or receive
	uint8 address; // slave address
	LDD_TData* buf; // buffer for transmit or receive data
	uint16 numbytes; // number of bytes in buffer
	uint8 send_stop; // stop at end or not
	xSemaphoreHandle done_semaphore; // to notify if done
	
} I2CTransferRequest;


void I2CDriver_AddTransferRequestToQueue(uint8 direction, 
		uint8 address, 
		LDD_TData* buf, 
		unsigned long numbytes,
		uint8 send_stop, 
		xSemaphoreHandle semph)
{
	I2CTransferRequest t;
	t.direction = direction;
	t.address = address;
	t.buf = buf;
	t.numbytes = numbytes;
	t.send_stop = send_stop;
	t.done_semaphore = semph;
	
	if(I2CTransferQueue)
	{
		while(xQueueSendToBack(I2CTransferQueue,( void * ) &t, (TickType_t) 0 )== errQUEUE_FULL)
		{
			vTaskDelay(10); // wait for a tick, see if queue clears up!
		}
	}
	
	
}


portTASK_FUNCTION(I2CTask, pvParameters)
{				
	// initialize SPI driver
	LDD_TDeviceData* I2CPtr;
	
	I2CPtr = CI2C2_Init(NULL);	
	// slave address is set in initialization (processor expert).  
	// initialize DMA semaphore and memory transfer queue
	I2CBlockSemaphore = xSemaphoreCreateBinary();  // using one semaphore for send/receive
	I2CTransferQueue = xQueueCreate( I2CTQSIZE, sizeof(I2CTransferRequest) );
	I2CTransferRequest t;
	
	for(;;)
	{
		// get the next transfer from the queue
		xQueueReceive( I2CTransferQueue, &t, ( TickType_t ) portMAX_DELAY );
		
		// check if address is different than address here
		//	if(t.address != )
			

		// should assert t.direction is 0 or 1
		if(t.direction == I2C_TX)
		{
			CI2C2_MasterSendBlock(I2CPtr, t.buf, t.numbytes, t.send_stop);
			// wait for block to finish being sent
			xSemaphoreTake( I2CBlockSemaphore, ( TickType_t ) portMAX_DELAY );
		}
		else if (t.direction == I2C_RX)
		{
			CI2C2_MasterReceiveBlock(I2CPtr, t.buf, t.numbytes, t.send_stop);
			xSemaphoreTake( I2CBlockSemaphore, ( TickType_t ) portMAX_DELAY );
		}		
		xSemaphoreGive(t.done_semaphore);
			
		
	}
}
