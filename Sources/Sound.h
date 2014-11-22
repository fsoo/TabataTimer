/*
 * SoundController.h
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#ifndef SOUNDCONTROLLER_H_
#define SOUNDCONTROLLER_H_
#include "FRTOS1.h"
#include "debug.h"
#include "SPIMemory.h"
#include "AudioTimer.h"
//#include "DA1.h"
#include "SSI1_TxDMA.h"
//#include "DMACH1.h"
#include "AmpEnable.h"
#include "SGTL5000.h"

// parameters for sound table
#define SOUND_MAX_NAME_LENGTH 	32 // 32 byte buffer names
#define SAMPLE_BYTES			(2*sizeof(uint16)) // 16 bit samples, stereo
#define BYTES_PER_PAGE			256U // each page in memory is 256 bytes
#define PRE_BUFFER				4 // pad buffers with address
#define SOUND_QUEUE_SIZE 		20 // 10 sounds in buffer
#define SAMPLE_RATE 			22050 // 22 KHz
// sound definitions - pasted from WriteSoundFile2
#define k30_Seconds_F        0
#define kPeanut              1
#define kRadioShack1         2
#define kRefWhistle1         3
#define k10_Seconds_F        4
#define kYeah_Buddy_2        5
#define kCountAccent         6
#define kCountUnaccent       7
#define kCountPause          8
#define kChirp_100ms         9
#define kSilence1Sec         10


typedef struct {
	uint16 version;
	uint16 numsounds;
	uint32* numsamples;
	unsigned char** strings;
	unsigned char** stringbufs; // buffers including prepend
	uint32* sound_addresses;
	
} SoundTable;

typedef struct {
	uint16 id;
	uint16 numsamples;
	unsigned char gain;
	unsigned char enable;
} SoundMessage;

portTASK_FUNCTION_PROTO(LoadSoundTable, pvParameters);

#define SOUNDPLAYER_TASK_STACK_SIZE 80
portTASK_FUNCTION_PROTO(SoundPlayerTask, pvParameters);
void AddSoundToQueue(uint16 sound_id, uint16 numsamples, unsigned char gain, unsigned char enable);
void IncreaseVolume(xSemaphoreHandle complete_semaphore);
void DecreaseVolume(xSemaphoreHandle complete_semaphore);

uint8 kCurrentVolume;

uint16 GetSoundQueueLength();
SoundTable kSoundTable;
LDD_TDeviceData* DA1_DeviceData;
LDD_TDeviceData* SSI1_DeviceData;	
	

#endif /* SOUNDCONTROLLER_H_ */
