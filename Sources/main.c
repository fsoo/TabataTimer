/* ###################################################################
**     Filename    : main.c
**     Project     : TabataTimer
**     Processor   : MKL25Z128VLK4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2014-05-01, 10:22, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "FRTOS1.h"
#include "UTIL1.h"
#include "CsIO1.h"
#include "IO1.h"
#include "PTD.h"
#include "StartStopButton.h"
#include "SM1.h"
#include "SM1_RxDMA.h"
#include "SM1_TxDMA.h"
#include "DisplayChipSelect.h"
#include "BitIoLdd1.h"
#include "MemoryChipSelect.h"
#include "BitIoLdd2.h"
#include "DMA1.h"
#include "AudioTimer.h"
#include "TU2.h"
#include "AmpEnable.h"
#include "BitIoLdd3.h"
#include "SM2.h"
#include "SM2_TxDMA.h"
#include "LED.h"
#include "BitIoLdd4.h"
#include "AmpGain.h"
#include "BitIoLdd5.h"
#include "ModeButton.h"
#include "PTC.h"
#include "PassthroughEnable.h"
#include "BitIoLdd6.h"
#include "EncoderA.h"
#include "EncoderB.h"
#include "PreampEnable.h"
#include "RTC.h"
#include "WAIT1.h"
#include "WAIT2.h"
#include "CI2C2.h"
#include "PTB.h"
#include "SSI1.h"
#include "SSI1_TxDMA.h"
#include "AmpGain2.h"
#include "BitIoLdd7.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "Interval.h"
#include "UI.h"
#include "SPIMemory.h"
#include "Sound.h"
#include "Display.h"
#include "debug.h"
#include "TestCode.h"

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */

{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  
//  DEBUG_printHeap();
  if(0) // go into test task
  {
	  

	  if (FRTOS1_xTaskCreate(
	 				  TestTask,  /* pointer to the task */
	 				 (unsigned portCHAR *)"TestTask", /* task name for kernel awareness debugging */
	 				 TEST_TASK_STACK_SIZE, /* task stack size */
	 				 (void*)NULL, /* optional task startup argument */
	 				tskIDLE_PRIORITY+1,  /* initial priority */
	 				 (xTaskHandle*)NULL /* optional task handle to create */
	 			   ) != pdPASS) {
	 			 for(;;){}; /* Out of heap memory? */
	 		};
	// 	  DEBUG_printHeap();
	    
  }
  else // start all other tasks up
  {
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
	  DEBUG_printHeap();
	  
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
	  
	  DEBUG_printHeap();
		
	  if (FRTOS1_xTaskCreate(
				  UITask,  /* pointer to the task */
				 (unsigned portCHAR *)"UITask", /* task name for kernel awareness debugging */
				 UI_TASK_STACK_SIZE, /* task stack size */
				(void*)NULL, /* optional task startup argument */
				 tskIDLE_PRIORITY+1,  /* initial priority */
				 (xTaskHandle*)NULL /* optional task handle to create */
			   ) != pdPASS) {
			 for(;;){}; /* Out of heap memory? */
		};
	  DEBUG_printHeap();
		
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
	  DEBUG_printHeap();
	
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
	  DEBUG_printHeap();
		
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
	  
	  Interval_Init();
	  DEBUG_printHeap();
	#ifdef DEBUG_TASK_ON
	  if (FRTOS1_xTaskCreate(
					DebugTask,  /* pointer to the task */
					 (unsigned portCHAR *)"DebugTask", /* task name for kernel awareness debugging */
					 configMINIMAL_STACK_SIZE, /* task stack size */
					 (void*)NULL, /* optional task startup argument */
					tskIDLE_PRIORITY+1,  /* initial priority */
					 (xTaskHandle*)NULL /* optional task handle to create */
				   ) != pdPASS) {
				 for(;;){}; /* Out of heap memory? */
			};
	  DEBUG_printHeap();
      
	#endif
  	 }   
  /* For example: for(;;) { } */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
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
