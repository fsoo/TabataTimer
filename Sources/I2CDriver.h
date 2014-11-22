/*
 * I2CDriver.h
 *
 *  Created on: Aug 25, 2014
 *      Author: fsoo
 */

#ifndef I2CDRIVER_H_
#define I2CDRIVER_H_

#include "FRTOS1.h"
#include "queue.h"
#include "semphr.h"
//#include "GI2C1.h"
#include "CI2C2.h"
#include "LED.h"
#include "I2C_PDD.h"

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#endif /* I2CDRIVER_H_ */
portTASK_FUNCTION_PROTO(I2CTask, pvParameters);

void I2CDriver_AddTransferRequestToQueue(uint8 direction, uint8 address, LDD_TData* buf, unsigned long numbytes,uint8 send_stop, xSemaphoreHandle semph);

#define I2CTQSIZE 5 // number of transfers in circular buffer
#define I2C_TASK_STACK_SIZE 60

#define I2C_TX 0
#define I2C_RX 1


