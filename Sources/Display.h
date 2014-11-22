/*
 * Display.h
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "FRTOS1.h"
#include "debug.h"

#include "font.c" 
#include "math.h"
#include "stdlib.h"

#include "PE_Types.h"
#include "SPIMemory.h"
#include "Sound.h"

#define swap(a, b) { int16 t = a; a = b; b = t; }

// LCD Dimensions
#define SHARPMEM_LCD_WIDTH       (128)
#define SHARPMEM_LCD_HEIGHT      (128) 
#define SHARPMEM_ADDRESS_BYTE	1
#define SHARPMEM_TRAILING_BYTE	1
#define SHARPMEM_ROW_BYTES		((SHARPMEM_LCD_WIDTH / 8) + SHARPMEM_ADDRESS_BYTE+SHARPMEM_TRAILING_BYTE) // include an address byte and a trailer byte so can DMA an entire row at a time
#define SHARPMEM_END_OF_BUFFER_BYTE 1
#define SHARPMEM_WRITE_COMMAND_BYTE 1
#define SHARPMEM_BUFFER_SIZE  (SHARPMEM_WRITE_COMMAND_BYTE + SHARPMEM_LCD_HEIGHT * SHARPMEM_ROW_BYTES + SHARPMEM_END_OF_BUFFER_BYTE)

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

#define SHARPMEM_BIT_WRITECMD   (0x01)
#define SHARPMEM_BIT_VCOM       (0x02)
#define SHARPMEM_BIT_CLEAR      (0x04)

typedef struct 
{
	byte sharpmem_buffer[SHARPMEM_BUFFER_SIZE]; // includes address byte at start of row, extra single byte at end 
//	byte sharpmem_bufferdump[SHARPMEM_BUFFER_SIZE]; // SPI requires dumping data into a dummy buffer
	
	byte* sharpmem_display_buffer;
	uint16 width; 
	uint16 height;
	
} DisplayStruct;

#define DISPLAY_INIT 0
#define DISPLAY_CLEAR 1
#define DISPLAY_REFRESH 2
#define DQSIZE 10
#define DISPLAY_WAIT_TICKS portMAX_DELAY // number of ticks to wait for display to finish SPI command
typedef unsigned char DisplayMessage;

void Display_Refresh();
void Display_Clear();
void Draw_Master_Display();

// also define a mutex to manage display access
SemaphoreHandle_t displayMutex;


#define DISPLAY_TASK_STACK_SIZE 150 // originally 60 but added DrawMasterDisplay

#define CHAR_HEIGHT 7
#define CHAR_WIDTH 5 

// Upper left box - 'mode'
#define Ax0 1
#define Ay0 1
#define Ax1 SHARPMEM_LCD_WIDTH / 2
#define Ay1 CHAR_HEIGHT * 3

// upper right box 'preset'
#define Bx0 Ax1
#define By0 1
#define Bx1 SHARPMEM_LCD_WIDTH
#define By1 CHAR_HEIGHT * 3

// middle box - time
#define Cx0 1
#define Cy0 Ay1+2
#define Cx1 SHARPMEM_LCD_WIDTH
#define Cy1 Ay1 + CHAR_HEIGHT * 6


// bottom left box - reps
#define Dx0 1
#define Dy0 Cy1
#define Dx1 SHARPMEM_LCD_WIDTH / 2
#define Dy1 Cy1 + CHAR_HEIGHT * 3

// bottom right box - reps
#define Ex0 SHARPMEM_LCD_WIDTH/2
#define Ey0 Cy1
#define Ex1 SHARPMEM_LCD_WIDTH 
#define Ey1 Cy1 + CHAR_HEIGHT * 3


portTASK_FUNCTION_PROTO(DisplayTask, pvParameters);
// display driver
void init();

uint8 getDisplayMutex(TickType_t xTicksToWait);
void releaseDisplayMutex();


// graphics routines

void drawPixel(uint16 x, uint16 y, uint16 color);
void getPixel(uint16 x, uint16 y);

void putChar(uint16 x, uint16 y, char c, uint8 color, uint8 bg, uint8 size);
void drawRect(int16 x, int16 y,
			    int16 w, int16 h,
			    uint16 color);

void drawFastVLine(int16 x, int16 y,
				 int16 h, uint16 color);

void drawFastHLine(int16 x, int16 y,
				 int16 w, uint16 color);
// Bresenham's algorithm - thx wikpedia
void drawLine(int16 x0, int16 y0,
			    int16 x1, int16 y1,
			    uint16 color);
void fillRect(int16 x, int16 y, int16 w, int16 h,
			    uint16 color);

void initText();
void setCursorPosition(uint16 x, uint16 y);
void setTextSize(uint8 size);
void setTextColor(uint8 color);
void setTextBackgroundColor(uint8 bgcolor);
void writeText(uint8_t c); 
void writeString(char* str, uint8 length);

#endif /* DISPLAY_H_ */
