/*
 * UI.h
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#ifndef UI_H_
#define UI_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "debug.h"
#include "StartStopButton.h"
#include "ModeButton.h"
#include "EncoderA.h"
#include "EncoderB.h"

#include "PTD.h"
#include "PORT_PDD.h"
#include "Display.h"

// responds to user requests - starts and stops tasks, manages different modes
// interrupt driven (not polled)

#define UI_TASK_STACK_SIZE 	170 // originally 70, but now have display 
portTASK_FUNCTION_PROTO(UITask, pvParameters);
#define BUTTON_TASK_STACK_SIZE 70 
portTASK_FUNCTION_PROTO(ButtonTask, pvParameters);

#define butDEBOUNCE_DELAY	( 10 / portTICK_RATE_MS )
#define butLONG_DURATION   (  800/ portTICK_RATE_MS )
#define STARTSTOP_BUTTON 1
#define MODE_BUTTON 2
#define ENCODER_STEP 3


#define STARTSTOP_BUTTON_DOWN 0  // pull up resistor
#define STARTSTOP_BUTTON_UP 1

#define MODE_BUTTON_DOWN 0  // pull up resistor
#define MODE_BUTTON_UP 1


#define UIQUEUESIZE 10 // 10 messages possible in queue, should be plenty!

typedef struct ButtonMessage
 {
    char buttonID;
    int16 buttonValue;
    TickType_t dt; // time since last press 
 } ButtonMessage;
 
 QueueHandle_t UIQueue;
 xSemaphoreHandle xButtonSemaphore;


 void changeVolume(int steps); 
 
#endif /* UI_H_ */
