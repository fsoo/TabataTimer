/*
 * SPIMemory.h
 *
 *  Created on: May 2, 2014
 *      Author: fsoo
 */

#ifndef SPIMEMORY_H_
#define SPIMEMORY_H_

#include "FRTOS1.h"
#include "queue.h"
#include "semphr.h"
#include "SM1.h"
#include "DisplayChipSelect.h"
#include "MemoryChipSelect.h"
#include "debug.h"

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "SPI_PDD.h"

/*
 * SPIMemory.h
 * 
 * interface for FLASH-memory via SPI bus
 * can use SPI0 or SPI1
 * 
 * how does it work?
 * 
 * 1) AddTransferToQueue - includes source and destination buffer, number of bytes, direction (future) Expected 
 * that the source buffer has instructions at the beginning of transfer.  Parses the transfer and adds it to the queu
 * 2) SPITask - watches the transfer queue, and sequences the transfers
 * 3) Interrupt handler - signals to SPITask when transfer is done
 */


portTASK_FUNCTION_PROTO(SPITask, pvParameters);

#define TQSIZE 20 // number of transfers in circular buffer
#define SPICOMSIZE 8 // number of bytes in command buffer
unsigned char kSPIBUF[SPICOMSIZE];

#define MEMORY_CHANNEL 0
#define DISPLAY_CHANNEL 1
		
typedef struct
{
	uint8 channel;
	LDD_TData* src;
	LDD_TData* dest;
	uint16 numbytes;
	xSemaphoreHandle done_semaphore;
	
} TransferRequest;

#define MEMORY_CHIPSELECT_ASSERT	MemoryChipSelect_ClrVal()
#define MEMORY_CHIPSELECT_DEASSERT  MemoryChipSelect_SetVal()
#define DISPLAY_CHIPSELECT_ASSERT   DisplayChipSelect_SetVal() // reversed for Sharp Mem display
#define DISPLAY_CHIPSELECT_DEASSERT DisplayChipSelect_ClrVal() 
#define SPI_MEMORY_CONFIGURATION 0
#define SPI_DISPLAY_CONFIGURATION 1

#define SECTOR_ERASE 	0
#define BLOCK32K_ERASE 	1
#define BLOCK64K_ERASE 	2
#define CHIP_ERASE 		3

#define PREPADDING  			4 // buffers are prepadded by four bytes so can include address


void SPIMemory_AddTransferToQueue(unsigned char channel, unsigned char* src, unsigned char* dest, unsigned long numbytes,xSemaphoreHandle semph);

#define SPI_TASK_STACK_SIZE 70
// was 65 before adding SPI_GetManfuacturer

portTASK_FUNCTION_PROTO(SPITask, pvParameters);
void SPIManager_GetStatus(unsigned char channel, unsigned char* buffer, xSemaphoreHandle complete_semaphore);
unsigned char SPIManager_ReadStatus(unsigned char* buffer);


void SPIMemory_GetByte(unsigned char channel, uint32 addr24,unsigned char* buffer, xSemaphoreHandle complete_semaphore);
char SPIMemory_ReadByteBuffer(unsigned char* buffer);
#define SPIGetByte(channel, addr24, buffer, complete_semaphore, dest)\
			SPIMemory_GetByte(channel, addr24, buffer, complete_semaphore);\
			xSemaphoreTake(complete_semaphore, portMAX_DELAY);\
			dest = buffer[4];


void SPIMemory_Get16(unsigned char channel, uint32 addr24, unsigned char* buffer, xSemaphoreHandle complete_semaphore);
uint16 SPIMemory_Read16Buffer(unsigned char* buffer);

#define SPIGet16(channel, addr24, buffer, complete_semaphore, dest)\
			SPIMemory_Get16(channel, addr24, buffer, complete_semaphore);\
			xSemaphoreTake(complete_semaphore, portMAX_DELAY);\
			dest = *((uint16*)&buffer[4]);


void SPIMemory_Get32(unsigned char channel, uint32 addr24, unsigned char* buffer, xSemaphoreHandle complete_semaphore);
uint32 SPIMemory_Read32Buffer(unsigned char* buffer);

#define SPIGet32(channel, addr24, buffer, complete_semaphore, dest)\
			SPIMemory_Get32(channel, addr24, buffer, complete_semaphore);\
			xSemaphoreTake(complete_semaphore, portMAX_DELAY);\
			dest = *((uint32*)&buffer[4]);

void SPIMemory_GetArray(unsigned char channel,uint32 addr24, unsigned char* buf_including_prepend, uint32 numbytes, xSemaphoreHandle complete_sempahore); // need to prepend 4 bytes to buf to accomodate instruction 
unsigned char* SPIMemory_ReadArray(unsigned char* buffer);
#define SPIGetArray(channel, addr24, buffer, numbytes, complete_semaphore, dest)\
			SPIMemory_GetArray(channel, addr24, buffer,numbytes, complete_semaphore);\
			xSemaphoreTake(complete_semaphore, portMAX_DELAY);\
			dest = buffer;

void SPIMemory_WriteArray(unsigned char channel,uint32 addr24, unsigned char* buf_including_prepend, uint32 numbytes); // need to prepend 4 bytes to buf to accomodate instruction 

void SPIMemory_Erase(unsigned char channel, uint8 size, uint32 addr24);//, xSemaphoreHandle complete_semaphore);

void SPIMemory_GetManufacturer(unsigned char* manbuf, xSemaphoreHandle complete_semaphore);

/*portTASK_FUNCTION_PROTO(SPITest, pvParameters);*/


#endif /* SPIMEMORY_H_ */
