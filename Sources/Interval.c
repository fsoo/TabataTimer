/*
 * Interval.c
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */
#include "Interval.h"



IntervalState is = {
		.update_ms = INITupdate_ms,
		.reps = INITreps, 
		.cycles= INITcycles,
		.rest = INITrest, 
		.work = INITwork,
		.pause = INITpause,
		.current_state = INITcurrent_state,
		.first_rest = INITfirstrest,
		.work_start_sound = kRefWhistle1,
		.work_start_sound_duration = WORK_START_SOUND_DURATION,
		.work_30sec_alert = k30_Seconds_F,
		.work_10sec_alert = k10_Seconds_F,
		.work_countdown = WORK_COUNTDOWN,
		.work_countdown_alert = kChirp_100ms,
		.work_end_alert = kRefWhistle1,
		.work_end_alert_duration = WORK_END_ALERT_DURATION,
		.work_comments_frequency = DEFAULT_COMMENTS_FREQUENCY, // how many times per cycle on average
		.work_comments_in_cycle = 0,
		
		.rest_30sec_alert = k30_Seconds_F,
		.rest_10sec_alert = k10_Seconds_F,
		.rest_countdown = REST_COUNTDOWN,
		.rest_countdown_alert = kChirp_100ms,
				
		.pause_30sec_alert = k30_Seconds_F,
		.pause_10sec_alert = k10_Seconds_F,
		.pause_countdown = PAUSE_COUNTDOWN,
		.pause_countdown_alert = kChirp_100ms		
};

xTaskHandle interval_task_handle = 0;

#define INTERVAL_START_NOW 1
#define INTERVAL_START_SUSPEND 0

static unsigned long w,r,p;
static unsigned short rep;
static unsigned short cycle;
static int prev_rep = -1;
static int prev_cycle = -1;
static uint8 prev_vol = 10;

portTASK_FUNCTION(IntervalTask, pvParameters) {
  char param = *((char*)pvParameters); 
  TickType_t xLastWakeTime;
  TickType_t xNowTime;
  const TickType_t xFrequency = is.update_ms/portTICK_PERIOD_MS; /* need to be careful that update ms is a multiple of tick period */
  
  xLastWakeTime = xTaskGetTickCount();
  DEBUG_printStack("IntervalTask");
 
  
  
  for(;;)
  {
	  // reset cycle, rep counters
	  cycle = 0;
	  rep =0;
	  r=0;
	  w=0;
	  p=0;
	  prev_rep = -1; // also reset prev rep counter
	  prev_cycle=-1;
	  
	  DEBUG_printf("task at pre-start!\n");
	  is.current_state = PRE_START;
	  drawIntervalDisplay();

	  if(param==INTERVAL_START_SUSPEND) // suspend here, will be unsuspended by UI controller
		  vTaskSuspend(NULL);
	  
	  for(cycle = 0; cycle < is.cycles; cycle++)
	  {
		  DEBUG_printf("cycle %u!\n", cycle);
		  // reset comments
		  is.work_comments_in_cycle = 0;

		  for(rep = 0; rep < is.reps; rep++)
		  {
			  is.work_comments_in_rep = 0;
			  DEBUG_printf("rep %u rest!\n", rep);
			  DEBUG_printStack("IntervalTask");
			  
			  is.current_state = REST_STATE;
			  
		//	  drawIntervalDisplay();

			  for(r=0; r < ((cycle == 0 && rep ==0) ? is.first_rest : is.rest); r += is.update_ms)
			  {
				  if(xLastWakeTime+xFrequency < (xNowTime = xTaskGetTickCount()))
				  {
					  // must have been suspended - so reset xLastWakeTime to now
					  xLastWakeTime = xNowTime;
				  }
				  drawIntervalDisplay();

				  if(r%1000 == 0) // once a second
				  {
					  playIntervalSounds();
				  }
				  vTaskDelayUntil( &xLastWakeTime, xFrequency );
			  }
			  
			  DEBUG_printStack("IntervalTask");
			  is.current_state = WORK_STATE;
			  for(w = 0; w < is.work; w += is.update_ms)
			  {
				  if(xLastWakeTime+xFrequency < (xNowTime = xTaskGetTickCount()))
				  {
					  // must have been suspended - so reset xLastWakeTime to now
					  xLastWakeTime = xNowTime;
					  
				  }
				  drawIntervalDisplay();

				  if(w%1000 == 0) // once a second
				  {
					  playIntervalSounds();
					  
				  }
				  vTaskDelayUntil( &xLastWakeTime, xFrequency );
			  }
		  }
		  DEBUG_printStack("IntervalTask");
		  is.current_state = PAUSE_STATE;
		  for(p=0; p < is.pause; p+= is.update_ms)
		  {
			  if(xLastWakeTime+xFrequency < (xNowTime = xTaskGetTickCount()))
			  {
				  // must have been suspended - so reset xLastWakeTime to now
				  xLastWakeTime = xNowTime;
			  }
			  drawIntervalDisplay();

			  if(p%1000 == 0) // once a second
			  {
				  playIntervalSounds();
			  }
			  vTaskDelayUntil( &xLastWakeTime, xFrequency );
		  }
	  }  
  }  
}



#define DISPLAY_STRBUF_SIZE 16
unsigned char display_strbuf[DISPLAY_STRBUF_SIZE];
unsigned char display_strbuf2[DISPLAY_STRBUF_SIZE];


void drawIntervalDisplay()
{
/*	if(getDisplayMutex(100/portTICK_PERIOD_MS) != pdTRUE)
	{
		// could not obtain semaphore - but don't block.
		for(;;);
	}
	*/
	if(prev_rep != rep)
	{
		UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.reps - rep),' ',2);

		setTextSize(2);
		setCursorPosition(Dx0,Dy0);
		writeString("reps\n",5);
		setTextSize(2);
		setCursorPosition(Dx0, Dy1);	
		writeString(display_strbuf,strlen(display_strbuf));		
		prev_rep = rep;
	}
	
	if(prev_cycle != cycle)
	{
		UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.cycles - cycle),' ',2);
		setCursorPosition(Ex0,Ey0);
		setTextSize(2);
		writeString("cycs\n",5);
		setCursorPosition(Ex0, Ey1);
		setTextSize(2);
		writeString(display_strbuf,strlen(display_strbuf));	
		prev_cycle = cycle;
	}
			
	switch(is.current_state)
	{
	case PRE_START:
		UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.first_rest - r)/1000,' ',3);
		UTIL1_Num16sToStrFormatted(display_strbuf2,DISPLAY_STRBUF_SIZE, (is.first_rest - r)/100 %10,' ',1);
		
	
		setCursorPosition(Ax0,Ay0);
		setTextSize(2);
		writeString("READY ",6);
	//	drawRect(Ax0,Ay0, Ax1,Ay1,0);
		setTextSize(4);
		setCursorPosition(Cx0, Cy0);
		writeString(display_strbuf,strlen(display_strbuf));
		writeString(".",1);
		writeString(display_strbuf2,strlen(display_strbuf2));		
		
			
		break;
	case REST_STATE:
	
		if(rep == 0 && cycle == 0) // first rest is special case
		{
			UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.first_rest - r)/1000,' ',3);
			UTIL1_Num16sToStrFormatted(display_strbuf2,DISPLAY_STRBUF_SIZE, (is.first_rest - r)/100 %10,' ',1);
		}
		else
		{
			UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.rest - r)/1000,' ',3);
			UTIL1_Num16sToStrFormatted(display_strbuf2,DISPLAY_STRBUF_SIZE, (is.rest - r)/100 %10,' ',1);
				
		}
			setCursorPosition(Ax0,Ay0);
		setTextSize(2);
		// if in rep 0, cycle 0, then in pre-start
//		if(rep == 0 && cycle == 0)
//		{
			writeString("REST  ",6);
			
//		}
//		else
//		{
//			writeString("REST  ",6);
//		}
	//	drawRect(Ax0,Ay0, Ax1,Ay1,0);
		setTextSize(4);
		setCursorPosition(Cx0, Cy0);
		writeString(display_strbuf,strlen(display_strbuf));
		writeString(".",1);
		writeString(display_strbuf2,strlen(display_strbuf2));		
	
		
		break;
	case WORK_STATE:
		
		UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.work - w)/1000,' ',3);
		UTIL1_Num16sToStrFormatted(display_strbuf2,DISPLAY_STRBUF_SIZE, (is.work - w)/100 %10,' ',1);
	
		setCursorPosition(Ax0,Ay0);
		setTextSize(2);
		writeString("WORK  ",6);
		setCursorPosition(Cx0, Cy0);
		setTextSize(4);
		writeString(display_strbuf,strlen(display_strbuf));
		writeString(".",1);
		writeString(display_strbuf2,strlen(display_strbuf2));		
	
		break;
	case PAUSE_STATE:

		UTIL1_Num16sToStrFormatted(display_strbuf,DISPLAY_STRBUF_SIZE, (is.pause - p + is.rest)/1000,' ',3
				);
		UTIL1_Num16sToStrFormatted(display_strbuf2,DISPLAY_STRBUF_SIZE, (is.pause - p + is.rest)/100 %10,' ',1);
	
		setCursorPosition(Ax0,Ay0);
		setTextSize(2);
		writeString("REST  ",6);
		setCursorPosition(Cx0, Cy0);
		setTextSize(4);
		
		// pause includes rest interval - i.e. counts down from 30 secs to 20 secs
		writeString(display_strbuf,strlen(display_strbuf));
		writeString(".",1);
		writeString(display_strbuf2,strlen(display_strbuf2));		
	
		break;
	}
	if(prev_vol != kCurrentVolume)
	{
		Draw_Master_Display();
		//	releaseDisplayMutex();
		prev_vol = kCurrentVolume;
	}
	
	Display_Refresh();
	
}


void playIntervalSounds()
{
	uint16 secs;
	switch(is.current_state)
	{
	case PRE_START:
		break;
	case REST_STATE:
		if(cycle == 0 && rep == 0) // special case, just started clock
		{	secs = (is.first_rest - r)/1000;
			
		}
		else
		{
			secs = (is.rest-r) / 1000;
		}
		if(r ==0 && rep>0) // just ended work cycle
		{
			int i;
			for(i = 0; i < rep*2; i++) // 1,3,5,7,11
			{
				if(i%2==0)
				{
					// on even i (0,2,4) sound queue
					AddSoundToQueue(is.work_end_alert,is.work_end_alert_duration/1000.0*SAMPLE_RATE / (rep*2+1),1,1);
				}
				else
				{
					// on odd, silence
					AddSoundToQueue(kSilence1Sec,is.work_end_alert_duration/1000.0*SAMPLE_RATE / (rep*2+1),0,0);
				}
			}
		}			
		else if(secs == 30) // 30 second queue
			AddSoundToQueue(is.rest_30sec_alert, 0,1,1);
		else if(secs== 10) // 10 second queue
			AddSoundToQueue(is.rest_10sec_alert, 0,1,1);
		else if(secs <= is.rest_countdown && secs >0) // countdown
			AddSoundToQueue(is.rest_countdown_alert,0,1,1);
		
		break;
	case WORK_STATE:
		secs = (is.work-w) / 1000;
		if(w ==0) // just started
			AddSoundToQueue(is.work_start_sound,is.work_start_sound_duration/1000.0*SAMPLE_RATE,1,1);
		else if(secs == 30) // 30 second queue
			AddSoundToQueue(is.work_30sec_alert, 0,1,1);
		else if(secs == 10) // 10 second queue
			AddSoundToQueue(is.work_10sec_alert, 0,1,1);
		else if(secs <= is.work_countdown && secs >0) // countdown
			AddSoundToQueue(is.work_countdown_alert,0,1,1);
		else if(is.work_comments_in_cycle < is.work_comments_frequency && is.work_comments_in_rep ==0) // still need to make some comments, but only one per cycle
		{
			int r2;
			r2 = rand();
			r = (RAND_MAX / (is.work/1000));
			if( r2 < r)
			{
			
				// choose which comment to make
				r = rand();
				
				if(r< RAND_MAX/2)
					AddSoundToQueue(kYeah_Buddy_2,0,1,1);
				else
					AddSoundToQueue(kPeanut, 0, 1, 1);
				is.work_comments_in_cycle++;
				is.work_comments_in_rep++;
			}
		}
		break;
		
		

	case PAUSE_STATE:
		secs = (is.pause-p) / 1000;
		if(p==0) // just started pause
		{
			int i; // play end sirens from last work interval
			for(i = 0; i < rep*2; i++) // 1,3,5,7,11
			{
				if(i%2==0)
				{
					// on even i (0,2,4) sound queue
					AddSoundToQueue(is.work_end_alert,is.work_end_alert_duration/1000.0*SAMPLE_RATE / (rep*2+1),1,1);
				}
				else
				{
					// on odd, silence
					AddSoundToQueue(kSilence1Sec,is.work_end_alert_duration/1000.0*SAMPLE_RATE / (rep*2+1),0,0);
				}
			}
		}			
		
	//	if(secs == 30) // 30 second queue
	//		AddSoundToQueue(is.pause_30sec_alert, 0,1,1);
	//	else if(secs== 10) // 10 second queue
	//		AddSoundToQueue(is.pause_10sec_alert, 0,1,1);
	//	else if(secs <= is.pause_countdown && secs >0) // countdown
	//		AddSoundToQueue(is.pause_countdown_alert,0,1,1);

		
	/*	else if((rand() % is.work_comments_frequency) == 0)
		{
			int r;
			r = rand() % 10;
			if(r<8)
				AddSoundToQueue(kYeah_Buddy_2,0,1,1);
			else
				AddSoundToQueue(kPeanut, 0, 1, 1);
		}

		*/		
		break;
	}
	
	
}

static char interval_param;

void Interval_Init()
{
	if(interval_task_handle!=0)
	{
		vTaskDelete(interval_task_handle);
		interval_task_handle =0;
	}
	interval_param = INTERVAL_START_SUSPEND;
	if (FRTOS1_xTaskCreate(
				IntervalTask,  /* pointer to the task */
				(unsigned portCHAR *)"IntervalTask", /* task name for kernel awareness debugging */
				INTERVAL_STACK_SIZE, /* task stack size */
				(void*)&interval_param, /* optional task startup argument */
				tskIDLE_PRIORITY+1,  /* initial priority */
				(xTaskHandle*) &interval_task_handle /* optional task handle to create */
			  ) != pdPASS) {
			for(;;){}; /* Out of heap memory? */
	};
	
}
void Interval_Start()
{// starts interval timer at wherever it was - if at top, starts at top, if in middle, restarts in middle
	if(interval_task_handle==0) // needs initialization
	{
		DEBUG_printHeap();
		interval_param = INTERVAL_START_NOW;
		if (FRTOS1_xTaskCreate(
					IntervalTask,  /* pointer to the task */
					(unsigned portCHAR *)"IntervalTask", /* task name for kernel awareness debugging */
					INTERVAL_STACK_SIZE, /* task stack size */
					(void*)&interval_param, /* optional task startup argument */
					tskIDLE_PRIORITY+1,  /* initial priority */
					(xTaskHandle*) &interval_task_handle /* optional task handle to create */
				  ) != pdPASS) {
				for(;;){}; /* Out of heap memory? */
		};
	}
	else
		vTaskResume(interval_task_handle);
}
void Interval_Pause() // pauses interval timer, if it has been running
{
	
	// release any display mutex
//	releaseDisplayMutex();
	if(interval_task_handle)
	{
		vTaskSuspend(interval_task_handle);
	}
	else
	{
		Interval_Init();
	}
}
void Interval_Reset() // resets interval timer and pauses it
{
	Interval_Init();
}

void Interval_Toggle()
{
	if(interval_task_handle==0)
	{
		Interval_Start();
	}
	else if(eTaskGetState(interval_task_handle)== eSuspended)
		Interval_Start();
	else
		Interval_Pause();
		
}

bool Interval_Running()
{
	if(interval_task_handle==0)
	{
		Interval_Init();
		return FALSE;
	}
	if(eTaskGetState(interval_task_handle)== eSuspended)
		return FALSE;
	
	return TRUE;
}
