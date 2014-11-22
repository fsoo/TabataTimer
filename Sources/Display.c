/*
 * Display.c
 *
 *  Created on: May 1, 2014
 *      Author: fsoo
 */

#include "Display.h"




#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);
DisplayStruct kDisplay;
QueueHandle_t DisplayQueue;
xSemaphoreHandle xDisplayReceivedSemaphore;
xSemaphoreHandle xBlockReceivedSemaphore_1; // semaphore for Channel 1 transfers


typedef struct {
  LDD_SPIMASTER_TError ErrFlag;        /* Error flags */
  uint16_t InpRecvDataNum;             /* The counter of received characters */
  uint8_t *InpDataPtr;                 /* The buffer pointer for received characters */
  uint16_t InpDataNumReq;              /* The counter of characters to receive by ReceiveBlock() */
  bool TxDMABusy;                      /* Transmitter DMA transfer is in progress */
  LDD_TDeviceData *TxDMATransfer;      /* Transmitter DMA transfer component's data pointer */
  LDD_DMA_TErrorFlags DMATransferErrors[1]; /* DMA transfer errors */
  LDD_TUserData *UserData;             /* User device data structure */
} SM2_TDeviceData;                     /* Device data structure type */

typedef SM2_TDeviceData* SM2_TDeviceDataPtr; /* Pointer to the device data structure */


portTASK_FUNCTION(DisplayTask, pvParameters)
{
	
	LDD_TDeviceData* SM2Ptr;	
	SM2Ptr = SM2_Init(NULL);
	
	xBlockReceivedSemaphore_1 = xSemaphoreCreateBinary();
	unsigned char buf[2];
		
	vSemaphoreCreateBinary( xDisplayReceivedSemaphore );
	DisplayQueue = xQueueCreate( DQSIZE, sizeof(unsigned char) );
	DisplayMessage d;
	uint8 _sharpmem_vcom = SHARPMEM_BIT_VCOM;
	
	init(); // initializes display buffer
	

	// add a clear display to queue
	d = DISPLAY_CLEAR;
	xQueueSendToBack(DisplayQueue, &d, 0);
	
	d = DISPLAY_REFRESH;
	xQueueSendToBack(DisplayQueue, &d, 0);
//	DEBUG_printStack("DisplayTask");
	for(;;)
	{
		if( xQueueReceive( DisplayQueue, & d, ( TickType_t ) DISPLAY_WAIT_TICKS ) )
		{
			switch (d)
			{
				case DISPLAY_CLEAR:
					buf[0] = SHARPMEM_BIT_CLEAR;
					buf[1] = 0;
					DISPLAY_CHIPSELECT_ASSERT;
					
					taskENTER_CRITICAL();
					SPI1_DL = ((uint8_t*) buf)[0]; // put the first byte into the transmit buffer
				
					((SM2_TDeviceDataPtr)SM2Ptr)->TxDMABusy = TRUE;
					SM2_TxDMA_SetSourceAddress(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer, (LDD_DMA_TData*)(buf+1));
					SM2_TxDMA_SetRequestCount(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer, (LDD_DMA_TRequestCount)1); // already transmitted first byte
					SM2_TxDMA_EnableRequest(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer);
					SPI_PDD_EnableTransmitDma(SPI1_BASE_PTR, PDD_ENABLE);
				  /* {FreeRTOS RTOS Adapter} Critical section ends (RTOS function call is defined by FreeRTOS RTOS Adapter property) */
					taskEXIT_CRITICAL();
			
					
					// send src block, receive dest block
			//		SM2_ReceiveBlock(SM2Ptr,buf,0); // don't receive data
			//		SM2_SendBlock(SM2Ptr,buf,2);
					// wait for transfer to finish
					xSemaphoreTake( xBlockReceivedSemaphore_1, ( TickType_t ) portMAX_DELAY); 								
					
						
				break;
				case DISPLAY_REFRESH:
					TOGGLE_VCOM;
					kDisplay.sharpmem_buffer[0] |= _sharpmem_vcom;
					DISPLAY_CHIPSELECT_ASSERT;
					taskENTER_CRITICAL();
						SPI1_DL = ((uint8_t*) kDisplay.sharpmem_buffer)[0]; // put the first byte into the transmit buffer
					
						((SM2_TDeviceDataPtr)SM2Ptr)->TxDMABusy = TRUE;
						SM2_TxDMA_SetSourceAddress(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer, (LDD_DMA_TData*)(kDisplay.sharpmem_buffer));
						SM2_TxDMA_SetRequestCount(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer, (LDD_DMA_TRequestCount)(SHARPMEM_BUFFER_SIZE)); // already transmitted first byte
						SM2_TxDMA_EnableRequest(((SM2_TDeviceDataPtr)SM2Ptr)->TxDMATransfer);
						SPI_PDD_EnableTransmitDma(SPI1_BASE_PTR, PDD_ENABLE);
					  /* {FreeRTOS RTOS Adapter} Critical section ends (RTOS function call is defined by FreeRTOS RTOS Adapter property) */
					taskEXIT_CRITICAL();
					
					
					// send src block, receive dest block
//					SM2_ReceiveBlock(SM2Ptr,kDisplay.sharpmem_buffer,0); // receive zero bytes of data - otherwise will overwrite buffer
//					SM2_SendBlock(SM2Ptr,kDisplay.sharpmem_buffer,SHARPMEM_BUFFER_SIZE);
					// wait for transfer to finish
					xSemaphoreTake( xBlockReceivedSemaphore_1, ( TickType_t ) portMAX_DELAY); 								
					
				break;
				
			}
			
		}
		
	}
}

void Display_Refresh()
{
	DisplayMessage d;
	d = DISPLAY_REFRESH;
	xQueueSendToBack(DisplayQueue, &d, 0);
		
	
}

void Display_Clear()
{
	DisplayMessage d;
	d = DISPLAY_CLEAR;
	xQueueSendToBack(DisplayQueue, &d, 0);
}

uint8 getDisplayMutex(TickType_t xTicksToWait)
{
	uint8 res;
	if(displayMutex != NULL)
	{
		res = xSemaphoreTake(displayMutex, xTicksToWait);
		
	}
	
	return res;
	
}

void releaseDisplayMutex()
{
	if(displayMutex != NULL)
	{
		xSemaphoreGive(displayMutex);
	}
	
	
}


void init()
{
	// create the display mutex
	displayMutex = xSemaphoreCreateMutex();
	//initialize the buffer
	kDisplay.sharpmem_display_buffer = kDisplay.sharpmem_buffer+1;
	kDisplay.width = SHARPMEM_LCD_WIDTH;
	kDisplay.height = SHARPMEM_LCD_HEIGHT;
	// zero the buffer, and write row addresses in each row
	kDisplay.sharpmem_buffer[0]=SHARPMEM_BIT_WRITECMD;
	
	byte i;
	for(i=0; i < SHARPMEM_LCD_HEIGHT; i++)
	{
		kDisplay.sharpmem_display_buffer[i*SHARPMEM_ROW_BYTES]=i+1; // offset address begins with 1
		byte j;
		for(j=1;j<SHARPMEM_ROW_BYTES-1;j++) // begin at 2nd byte and zero the row, including trailing byte
		{
				kDisplay.sharpmem_display_buffer[i*SHARPMEM_ROW_BYTES+j]=0xFF;
			
		}	
		kDisplay.sharpmem_display_buffer[(i+1)*SHARPMEM_ROW_BYTES-1]=0x00; // zero trailing byte
	}
	kDisplay.sharpmem_buffer[SHARPMEM_BUFFER_SIZE-1] = 0; // zero out the last end of buffer byte

	initText();
};

void drawPixel(uint16 x , uint16 y , uint16 color) 
{
  if ((x >= SHARPMEM_LCD_WIDTH) || (y >= SHARPMEM_LCD_HEIGHT))
    return;

  if (color)
    kDisplay.sharpmem_display_buffer[(y*SHARPMEM_ROW_BYTES) + x/8+SHARPMEM_ADDRESS_BYTE] |= (1 << x % 8);
  else
    kDisplay.sharpmem_display_buffer[(y*SHARPMEM_ROW_BYTES) + x/8+SHARPMEM_ADDRESS_BYTE] &= ~(1 << x % 8);
}


void getPixel(uint16 x, uint16 y)
{
	
};

unsigned int current_row = 0;

void drawline()
{
	byte* r;
	r = kDisplay.sharpmem_display_buffer+current_row*SHARPMEM_ROW_BYTES;
	int i;
	for(i=0;i<5;i++)
	{
		r[1+i]=0xFF;
	}

	current_row = (current_row+1) % SHARPMEM_LCD_HEIGHT;
	r = kDisplay.sharpmem_display_buffer+current_row*SHARPMEM_ROW_BYTES;
		
	for(i=0;i<12;i++)
	{
		r[1+i]=0x0F;
	}
	
}

void putChar(uint16 x, uint16 y, char c, uint8 color, uint8 bg, uint8 size)
{
	 if((x >= SHARPMEM_LCD_WIDTH)            || // Clip right
	     (y >= SHARPMEM_LCD_WIDTH)           || // Clip bottom
	     ((x + 6 * size - 1) < 0) || // Clip left
	     ((y + 8 * size - 1) < 0))   // Clip top
	    return;
	 int8 i,j;
	 
	  for (i=0; i<6; i++ ) {
	    uint8 line;
	    if (i == 5) 
	      line = 0x0;
	    else 
	      line = font[(c*5)+i];
	    for (j = 0; j<8; j++) {
	      if (line & 0x1) {
	        if (size == 1) // default size
	          drawPixel(x+i, y+j, color);
	        else {  // big size
	          fillRect(x+(i*size), y+(j*size), size, size, color);
	        } 
	      } else if (bg != color) {
	        if (size == 1) // default size
	          drawPixel(x+i, y+j, bg);
	        else {  // big size
	          fillRect(x+i*size, y+j*size, size, size, bg);
	        }
	      }
	      line >>= 1;
	    }
	  }

	
	
}


void drawRect(int16 x, int16 y,
			    int16 w, int16 h,
			    uint16 color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void drawFastVLine(int16 x, int16 y,
				 int16 h, uint16 color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void drawFastHLine(int16 x, int16 y,
				 int16 w, uint16 color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

// Bresenham's algorithm - thx wikpedia
void drawLine(int16 x0, int16 y0,
			    int16 x1, int16 y1,
			    uint16 color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void fillRect(int16 x, int16 y, int16 w, int16 h,
			    uint16 color) {
  // Update in subclasses if desired!
	int16 i;
  for (i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

typedef struct {
	uint16 cursor_x;
	uint16 cursor_y;
	uint8 textsize;
	uint8 textcolor;
	uint8 textbgcolor;
	bool wrap;
} TextStruct;

TextStruct _text;

void initText()
{
	_text.cursor_x = 0;
	_text.cursor_y = 0;
	_text.textsize = 1;
	_text.textcolor = 0;
	_text.textbgcolor = 1;
	_text.wrap=1;
	

}
void setCursorPosition(uint16 x, uint16 y)
{
	_text.cursor_x = x;
	_text.cursor_y = y;
	
}

void setTextSize(uint8 size)
{
	_text.textsize = size;
}

void setTextBackgroundColor(uint8 bgcolor)
{
	_text.textbgcolor = bgcolor;
}

void setTextColor(uint8 color)
{
	_text.textcolor = color;
}

void writeText(uint8_t c) {
  if (c == '\n') {
    _text.cursor_y += _text.textsize*8;
    _text.cursor_x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    putChar(_text.cursor_x, _text.cursor_y, c, _text.textcolor, _text.textbgcolor, _text.textsize);
    _text.cursor_x += _text.textsize*6;
    if (_text.wrap && (_text.cursor_x > (kDisplay.width - _text.textsize*6))) {
      _text.cursor_y += _text.textsize*8;
      _text.cursor_x = 0;
    }
  }
}

void writeString(char* str, uint8 length)
{
	int i;
	for(i=0;i<length;i++)
		writeText(str[i]);
}


#define MASTER_DISPLAY_STRBUF_SIZE 16
unsigned char master_display_strbuf[MASTER_DISPLAY_STRBUF_SIZE];
	
void Draw_Master_Display()
{
	UTIL1_Num16sToStrFormatted(master_display_strbuf,MASTER_DISPLAY_STRBUF_SIZE, kCurrentVolume,' ',2);
	
	// include master controls like volume, flashing
	// also tell volume
	setCursorPosition(Ax0,Dy0+6*CHAR_HEIGHT);
	setTextSize(2);
	writeString("vol ",4);
//	setCursorPosition(Ax0+, Dy0+8*CHAR_HEIGHT);
//	setTextSize(2);
	writeString(master_display_strbuf,strlen(master_display_strbuf));
//	fillRect(Ax0+2*CHAR_WIDTH*7,Dy0+6*CHAR_HEIGHT, 128,Dy0+8*CHAR_HEIGHT,1);
	
	
}
