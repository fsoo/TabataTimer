/*
 * debug.h
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#include "stdio.h"
#include "FRTOS1.h"
#include "SPIMemory.h"
#include "Sound.h"
#include "Display.h"
#include "UI.h"
#include "UTIL1.h"
#ifndef DEBUG_H_
#define DEBUG_H_

//#define DEBUG_ON
//#define DEBUG_TASK_ON
#define DEBUG_HEAP_ON
#define DEBUG_STACK_ON

#define D_printf(...) do{taskENTER_CRITICAL(); printf(__VA_ARGS__); taskEXIT_CRITICAL();}while(0)

#ifdef DEBUG_ON
#define DEBUG_printf(...) do{taskENTER_CRITICAL(); printf(__VA_ARGS__); taskEXIT_CRITICAL();}while(0)
#else
#define DEBUG_printf(...) ;
#endif

#define DEBUG_STRBUF_SIZE 8
extern unsigned char debug_strbuf[DEBUG_STRBUF_SIZE];

#ifdef DEBUG_HEAP_ON

extern const char* heapstr;

#define DEBUG_printHeap() do{\
				taskENTER_CRITICAL(); \
				fputs(heapstr,stdout); \
				UTIL1_Num16uToStr(debug_strbuf, DEBUG_STRBUF_SIZE, xPortGetFreeHeapSize()); \
				fputs(debug_strbuf,stdout); \
				putc('\n',stdout); \
				taskEXIT_CRITICAL();}while(0)
#else
#define DEBUG_printHeap() ; 
#endif

#ifdef DEBUG_STACK_ON


extern const char* stackstr;

#define DEBUG_printStack(location) do{\
		taskENTER_CRITICAL(); \
		fputs(location, stdout); \
		fputs(stackstr,stdout); \
		UTIL1_Num32uToStr(debug_strbuf,DEBUG_STRBUF_SIZE,uxTaskGetStackHighWaterMark( NULL ) ); \
		fputs(debug_strbuf,stdout); \
		putc('\n',stdout);\
		taskEXIT_CRITICAL();}while(0)
	

#else
#define DEBUG_printStack(location) ; 
#endif


#ifdef DEBUG_TASK_ON
	portTASK_FUNCTION_PROTO(DebugTask, pvParameters);
#endif
	

	

#endif /* DEBUG_H_ */
