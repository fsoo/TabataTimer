/* ###################################################################
**     Filename    : Events.c
**     Project     : TabataTimer
**     Processor   : MKL25Z128VLK4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2014-05-01, 10:22, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         Cpu_OnNMIINT - void Cpu_OnNMIINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"
#include "Sound.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */

/*
** ===================================================================
**     Event       :  FRTOS1_vApplicationStackOverflowHook (module Events)
**
**     Component   :  FRTOS1 [FreeRTOS]
**     Description :
**         if enabled, this hook will be called in case of a stack
**         overflow.
**     Parameters  :
**         NAME            - DESCRIPTION
**         pxTask          - Task handle
**       * pcTaskName      - Pointer to task name
**     Returns     : Nothing
** ===================================================================
*/
void FRTOS1_vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
  /* This will get called if a stack overflow is detected during the context
     switch.  Set configCHECK_FOR_STACK_OVERFLOWS to 2 to also check for stack
     problems within nested interrupts, but only do this for debug purposes as
     it will increase the context switch time. */
  (void)pxTask;
  (void)pcTaskName;
  taskDISABLE_INTERRUPTS();
  /* Write your code here ... */
  for(;;) {}
}

/*
** ===================================================================
**     Event       :  FRTOS1_vApplicationTickHook (module Events)
**
**     Component   :  FRTOS1 [FreeRTOS]
**     Description :
**         If enabled, this hook will be called by the RTOS for every
**         tick increment.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void FRTOS1_vApplicationTickHook(void)
{
  /* Called for every RTOS tick. */
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  FRTOS1_vApplicationIdleHook (module Events)
**
**     Component   :  FRTOS1 [FreeRTOS]
**     Description :
**         If enabled, this hook will be called when the RTOS is idle.
**         This might be a good place to go into low power mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void FRTOS1_vApplicationIdleHook(void)
{
  /* Called whenever the RTOS is idle (from the IDLE task).
     Here would be a good place to put the CPU into low power mode. */
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  FRTOS1_vApplicationMallocFailedHook (module Events)
**
**     Component   :  FRTOS1 [FreeRTOS]
**     Description :
**         If enabled, the RTOS will call this hook in case memory
**         allocation failed.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void FRTOS1_vApplicationMallocFailedHook(void)
{
  /* Called if a call to pvPortMalloc() fails because there is insufficient
     free memory available in the FreeRTOS heap.  pvPortMalloc() is called
     internally by FreeRTOS API functions that create tasks, queues, software
     timers, and semaphores.  The size of the FreeRTOS heap is set by the
     configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  taskDISABLE_INTERRUPTS();
  /* Write your code here ... */
  for(;;) {}
}

/*
** ===================================================================
**     Interrupt handler : PortDInterrupt
**
**     Description :
**         User interrupt service routine. 
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/



/*
** ===================================================================
**     Event       :  SM1_OnBlockReceived (module Events)
**
**     Component   :  SM1 [SPIMaster_LDD]
*/
/*!
**     @brief
**         This event is called when the requested number of data is
**         moved to the input buffer. This method is available only if
**         the ReceiveBlock method is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/
extern xSemaphoreHandle xBlockReceivedSemaphore_0;
void SM1_OnBlockReceived(LDD_TUserData *UserDataPtr)
{
	MEMORY_CHIPSELECT_DEASSERT;
			
  // signal block received
	BaseType_t xHigherPriorityTaskWoken;
	if( xBlockReceivedSemaphore_0 != 0 )			
		xSemaphoreGiveFromISR( xBlockReceivedSemaphore_0, &xHigherPriorityTaskWoken );
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
	//SPI_PDD_EnableInterruptMask(SPI0_BASE_PTR, SPI_PDD_RX_BUFFER_FULL_OR_FAULT); /* Disable Rx buffer full interrupt */
}

/*
** ===================================================================
**     Event       :  AudioTimer_OnInterrupt (module Events)
**
**     Component   :  AudioTimer [TimerInt_LDD]
*/
/*!
**     @brief
**         Called if periodic event occur. Component and OnInterrupt
**         event must be enabled. See [SetEventMask] and [GetEventMask]
**         methods. This event is available only if a [Interrupt
**         service/event] is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer passed as
**                           the parameter of Init method.
*/
/* ===================================================================*/
void AudioTimer_OnInterrupt(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  DMACH1_OnComplete (module Events)
**
**     Component   :  DMACH1 [DMAChannel_LDD]
*/
/*!
**     @brief
**         Called at the end of a DMA transfer. If the Half complete
**         property in initialization section is enabled, this event is
**         also called when current major iteration count reaches the
**         halfway point. See SetEventMask() and GetEventMask() methods.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
*/
/* ===================================================================*/
xSemaphoreHandle DAC_complete_semaphore;
extern 	LDD_TDeviceData* DMA_DeviceData;
extern unsigned char* play_address;
extern uint32 nbytes;

void DMACH1_OnComplete(LDD_TUserData *UserDataPtr)
{
	taskENTER_CRITICAL();

	DMACH1_DisableRequest(DMA_DeviceData);
	if(play_address != 0) // not the last page
	{
		DMACH1_SetSourceAddress(DMA_DeviceData, play_address); // remember to offset by front padding
		DMACH1_SetRequestCount(DMA_DeviceData, nbytes/SAMPLE_BYTES);
		DMACH1_EnableRequest(DMA_DeviceData);
	}
	taskEXIT_CRITICAL();
	// signal block received
	BaseType_t xHigherPriorityTaskWoken;
	if( DAC_complete_semaphore != 0 )
	{
		xSemaphoreGiveFromISR( DAC_complete_semaphore, &xHigherPriorityTaskWoken );
	}
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
}

/*
** ===================================================================
**     Event       :  FRTOS1_vOnPreSleepProcessing (module Events)
**
**     Component   :  FRTOS1 [FreeRTOS]
**     Description :
**         Used in tickless idle mode only, but required in this mode.
**         Hook for the application to enter low power mode.
**     Parameters  :
**         NAME            - DESCRIPTION
**         expectedIdleTicks - expected idle
**                           time, in ticks
**     Returns     : Nothing
** ===================================================================
*/
void FRTOS1_vOnPreSleepProcessing(portTickType expectedIdleTicks)
{
#if 1
  /* example for Kinetis (enable SetOperationMode() in CPU component): */
  Cpu_SetOperationMode(DOM_WAIT, NULL, NULL);
  /* or to wait for interrupt:
    __asm volatile("dsb");
    __asm volatile("wfi");
    __asm volatile("isb");
  */
#elif 0
  /* example for S08/S12/ColdFire V1 (enable SetWaitMode() in CPU): */
  Cpu_SetWaitMode();
#elif 0
  /* example for ColdFire V2: */
   __asm("stop #0x2000"); */
#else
  #error "you *must* enter low power mode (wait for interrupt) here!"
#endif
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  SM2_OnBlockSent (module Events)
**
**     Component   :  SM2 [SPIMaster_LDD]
*/
/*!
**     @brief
**         This event is called after the last character from the
**         output buffer is moved to the transmitter. This event is
**         available only if the SendBlock method is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/
extern xSemaphoreHandle xBlockReceivedSemaphore_1;

void SM2_OnBlockSent(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
	
	DISPLAY_CHIPSELECT_DEASSERT;
	taskENTER_CRITICAL();

	// signal block received
	BaseType_t xHigherPriorityTaskWoken;
	if( xBlockReceivedSemaphore_1 != 0 )
	{
	//	fputs("display semaphore give from ISR\n", stdout);
		xSemaphoreGiveFromISR( xBlockReceivedSemaphore_1, &xHigherPriorityTaskWoken );
	}
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
	// SPI_PDD_DisableInterruptMask(SPI1_BASE_PTR, SPI_PDD_RX_BUFFER_FULL_OR_FAULT); /* Disable Rx buffer full interrupt */
	taskEXIT_CRITICAL();

}

/*
** ===================================================================
**     Event       :  SM2_OnBlockReceived (module Events)
**
**     Component   :  SM2 [SPIMaster_LDD]
*/
/*!
**     @brief
**         This event is called when the requested number of data is
**         moved to the input buffer. This method is available only if
**         the ReceiveBlock method is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/

void SM2_OnBlockReceived(LDD_TUserData *UserDataPtr)
{
	if(FALSE)
	{
		taskENTER_CRITICAL();
	
		// signal block received
		BaseType_t xHigherPriorityTaskWoken;
		if( xBlockReceivedSemaphore_1 != 0 )
		{
		//	fputs("display semaphore give from ISR\n", stdout);
			xSemaphoreGiveFromISR( xBlockReceivedSemaphore_1, &xHigherPriorityTaskWoken );
		}
		// return to the correct priority task
		if(xHigherPriorityTaskWoken)
		{
			taskYIELD();
		}
		taskEXIT_CRITICAL();
	}
}



/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL25Z128LK4]
*/
/*!
**     @brief
**         This event is called when the Non maskable interrupt had
**         occurred. This event is automatically enabled when the [NMI
**         interrupt] property is set to 'Enabled'.
*/
/* ===================================================================*/
void Cpu_OnNMIINT(void)
{
  /* Write your code here ... */
 // clear the NMI flag
	
	

}


/*
** ===================================================================
**     Event       :  I2C0_OnMasterBlockSent (module Events)
**
**     Component   :  I2C0 [I2C_LDD]
*/
/*!
**     @brief
**         This event is called when I2C in master mode finishes the
**         transmission of the data successfully. This event is not
**         available for the SLAVE mode and if MasterSendBlock is
**         disabled. 
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
*/
/* ===================================================================*/

extern xSemaphoreHandle I2CBlockSemaphore; // semaphore to signal completion of I2C transfer
void CI2C2_OnMasterBlockSent(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
	taskENTER_CRITICAL();

		// signal block received
		BaseType_t xHigherPriorityTaskWoken;
		if( I2CBlockSemaphore != 0 )
		{
		//	fputs("display semaphore give from ISR\n", stdout);
			xSemaphoreGiveFromISR( I2CBlockSemaphore, &xHigherPriorityTaskWoken );
		}
		// return to the correct priority task
		if(xHigherPriorityTaskWoken)
		{
			taskYIELD();
		}
		taskEXIT_CRITICAL();
	
}

/*
** ===================================================================
**     Event       :  I2C0_OnMasterBlockReceived (module Events)
**
**     Component   :  I2C0 [I2C_LDD]
*/
/*!
**     @brief
**         This event is called when I2C is in master mode and finishes
**         the reception of the data successfully. This event is not
**         available for the SLAVE mode and if MasterReceiveBlock is
**         disabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
*/
/* ===================================================================*/

void CI2C2_OnMasterBlockReceived(LDD_TUserData *UserDataPtr)
{

	taskENTER_CRITICAL();

	// signal block received
	BaseType_t xHigherPriorityTaskWoken;
	if( I2CBlockSemaphore != 0 )
	{
	//	fputs("display semaphore give from ISR\n", stdout);
		xSemaphoreGiveFromISR( I2CBlockSemaphore, &xHigherPriorityTaskWoken );
	}
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
	taskEXIT_CRITICAL();

}

/*
** ===================================================================
**     Event       :  SSI1_OnBlockSent (module Events)
**
**     Component   :  SSI1 [SSI_LDD]
*/
/*!
**     @brief
**         This event is called after the last character from the
**         output buffer is moved to the transmitter. This event is
**         available only if the SendBlock method is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/

extern nsamples;
void SSI1_OnBlockSent(LDD_TUserData *UserDataPtr)
{
	taskENTER_CRITICAL();

//	DMACH1_DisableRequest(DMA_DeviceData);
	if(play_address != 0) // not the last page
	{
		SSI1_SendBlock(SSI1_DeviceData,play_address, nsamples);
		SSI1_EnableTransfer(SSI1_DeviceData, LDD_SSI_TRANSMITTER);
		
//		DMACH1_SetSourceAddress(DMA_DeviceData, play_address); // remember to offset by front padding
//		DMACH1_SetRequestCount(DMA_DeviceData, nbytes/SAMPLE_BYTES);
//		DMACH1_EnableRequest(DMA_DeviceData);
	}
	taskEXIT_CRITICAL();
	// signal block received
	BaseType_t xHigherPriorityTaskWoken;
	if( DAC_complete_semaphore != 0 )
	{
		xSemaphoreGiveFromISR( DAC_complete_semaphore, &xHigherPriorityTaskWoken );
	}
	// return to the correct priority task
	if(xHigherPriorityTaskWoken)
	{
		taskYIELD();
	}
	
	/* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  SSI1_OnError (module Events)
**
**     Component   :  SSI1 [SSI_LDD]
*/
/*!
**     @brief
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/
void SSI1_OnError(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  SM1_OnBlockSent (module Events)
**
**     Component   :  SM1 [SPIMaster_LDD]
*/
/*!
**     @brief
**         This event is called after the last character from the
**         output buffer is moved to the transmitter. This event is
**         available only if the SendBlock method is enabled.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer is passed
**                           as the parameter of Init method. 
*/
/* ===================================================================*/
void SM1_OnBlockSent(LDD_TUserData *UserDataPtr)
{
	//MEMORY_CHIPSELECT_DEASSERT;
				
	  // signal block received
		BaseType_t xHigherPriorityTaskWoken;
		if( xBlockReceivedSemaphore_0 != 0 )			
			xSemaphoreGiveFromISR( xBlockReceivedSemaphore_0, &xHigherPriorityTaskWoken );
		// return to the correct priority task
		if(xHigherPriorityTaskWoken)
		{
			taskYIELD();
		}
}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
