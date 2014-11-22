/*
 * Interval.h
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include "FRTOS1.h"
#include "display.h"
#include "debug.h"
#include "sound.h"

// Interval timer
// Methods:
// Start() : starts timer
// Stop() : stops timer
// Reset() : resets timer
// Pause() : pauses timer
// GetState() : gets pointer to the state of the system (e.g. current state, paused or not, number of reps, cycles etc)

// How it works:
// One task with nested for-next loop for cycles, reps, work and rest
// in each nest there is a check for if it is paused, running, or if there is an event (e.g. play a sound, update display)
// if paused, lets go of semaphore
// if running: updates internal state, checks for events, updates display if necessary (if it can get display handle), then asks for delay of x ms
// clock is kept in stack (rather than external clock)

// struct keeps track of current state

#define PRE_START 0
#define REST_STATE 1
#define WORK_STATE 2
#define PAUSE_STATE 3
#define END_STATE 4

#define INITupdate_ms (10*portTICK_PERIOD_MS) // would like this to be ~100ms
#define INITreps 8 //10 // 8
#define INITcycles 7 //1 //7
#define INITrest 10000//105000 // 10000
#define INITwork 20000//45000 //20000
#define INITpause 20000//30000 //20000
#define INITcurrent_state PRE_START
#define INITfirstrest 10000 // length of the first rest state default

#define WORK_START_SOUND_DURATION 1000
#define WORK_END_ALERT_DURATION 2000
#define WORK_COUNTDOWN 0 // 3 seconds
#define REST_COUNTDOWN 3
#define PAUSE_COUNTDOWN 3

#define DEFAULT_COMMENTS_FREQUENCY 10

typedef struct IntervalState {
	unsigned long update_ms;  // how many ms between updates - e.g. 100ms for 10 times a second.  This is the delay that it asks for from the OS
	unsigned short reps; // reps per cycle
	unsigned short cycles; // total number of cycles
	unsigned long rest; // rest period, milliseconds
	unsigned long work; // work period, milliseconds
	unsigned short pause; // pause period, milliseconds
	unsigned long first_rest; // first rest period, milliseconds
	
	unsigned short current_state;
	
	unsigned short work_start_sound;
	unsigned long work_start_sound_duration; // in ms
	
	unsigned short work_30sec_alert;
	unsigned short work_10sec_alert;
	unsigned short work_countdown;
	unsigned short work_countdown_alert;
	unsigned short work_end_alert;
	unsigned long work_end_alert_duration;// in ms
		
	unsigned short work_comments_frequency;
	unsigned short work_comments_in_cycle;
	unsigned short work_comments_in_rep;
	
	unsigned short rest_30sec_alert;
	unsigned short rest_10sec_alert;
	unsigned short rest_countdown_alert;
	unsigned short rest_countdown;
	unsigned short rest_comments_freq;
	
	unsigned short pause_30sec_alert;
	unsigned short pause_10sec_alert;
	unsigned short pause_countdown_alert;

	unsigned short pause_countdown;
	unsigned short pause_comments_freq;
	
	
} IntervalState;

void Interval_Init(); 
void Interval_Start(); // starts interval timer at wherever it was - if at top, starts at top, if in middle, restarts in middle
void Interval_Pause(); // pauses interval timer
void Interval_Reset(); // resets (restarts) interval timer
void Interval_Toggle(); // toggles whether interval running or not
bool Interval_Running(); // if interval timer running, returns TRUE

#define INTERVAL_STACK_SIZE 150 // need 220 bytes!
portTASK_FUNCTION_PROTO(IntervalTask, pvParameters);

void drawIntervalDisplay();
void playIntervalSounds();



#endif /* INTERVAL_H_ */
