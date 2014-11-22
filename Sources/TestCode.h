/*
 * TestCode.h
 *
 *  Created on: Jun 10, 2014
 *      Author: fsoo
 */

#ifndef TESTCODE_H_
#define TESTCODE_H_

#include "FRTOS1.h"
#include "UTIL1.h"
#include "debug.h"
#include "Interval.h"
#include "UI.h"
#include "SPIMemory.h"
#include "Sound.h"
#include "Display.h"
#include "debug.h"
#include "PassthroughEnable.h"
#include "RTCSimple.h"
#include "SGTL5000.h"

/* BOARD TEST ROUTINES
 * LED/power/UART
 * Memory
 * Display
 * Sound
 * Pushbuttons
 * Encoders
 */

#define TEST_TASK_STACK_SIZE 200

portTASK_FUNCTION_PROTO(TestTask, pvParameters);
#endif /* TESTCODE_H_ */
