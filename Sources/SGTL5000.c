#include "SGTL5000.h"


void SGTL5000_GetChipID(uint8* i2cbuf, xSemaphoreHandle done)
{
	i2cbuf[0]= 0x00;
	i2cbuf[1]= 0x00;
	I2CDriver_AddTransferRequestToQueue(I2C_TX, SGTL5000_I2C_SLAVE_ADDRESS, i2cbuf, 2,LDD_I2C_SEND_STOP, done);
	xSemaphoreTake(done, portMAX_DELAY); 
	I2CDriver_AddTransferRequestToQueue(I2C_RX, SGTL5000_I2C_SLAVE_ADDRESS, i2cbuf, 2, LDD_I2C_SEND_STOP, done);
	xSemaphoreTake(done, portMAX_DELAY); 			
	
}


uint16 r;
uint16 v;
uint16 writebuf[4];

#define WRITE(REG, VAL) do{\
		writebuf[0]=(REG << 8 | REG >> 8);\
		writebuf[1]=(VAL << 8 | VAL >> 8);\
		I2CDriver_AddTransferRequestToQueue(I2C_TX, SGTL5000_I2C_SLAVE_ADDRESS, &writebuf, 4,LDD_I2C_SEND_STOP, done); \
		xSemaphoreTake(done, portMAX_DELAY); \
		}while(0)

#define READ(REG) do{\
		r=(REG << 8 | REG >> 8);\
		I2CDriver_AddTransferRequestToQueue(I2C_TX, SGTL5000_I2C_SLAVE_ADDRESS, &r, 2,LDD_I2C_SEND_STOP, done); \
		xSemaphoreTake(done, portMAX_DELAY); \
		I2CDriver_AddTransferRequestToQueue(I2C_RX, SGTL5000_I2C_SLAVE_ADDRESS, &v, 2, LDD_I2C_SEND_STOP, done); \
		xSemaphoreTake(done, portMAX_DELAY); \
		v=(v << 8 | v >> 8);\
		}while(0)



void SGTL5000_Init(xSemaphoreHandle done)
{
	
	// power up internal linear regulator
	// 15:7 - 0 reserved
	// 6 - 0 use VDDA (if ovrd is set)
	// 5 - 0 don't override
	// 4 - 0 reserved
	// 3 - 1 set to 1.2V
	// 2 - 0
	// 1 - 0
	// 0 - 0
	
	WRITE(CHIP_LINREG_CTRL, 0x0008);
//	READ(CHIP_LINREG_CTRL);

	// turn off startup power supplies

	// first set MCLK to default settings, including turning off dependence on PLL
	// 15:6 0 reserved
	// 5:4 0 SYS_FS specifies the rate
	// 3 1 0x1 48kHz // default to 48kHz
	// 2 0 "
	// 1 0 - master clock is 256 FS (even if it isn't, don't want to use PLL until set frequency)
	// 0 0
	
	WRITE(CHIP_CLK_CTRL, 0x0008);
//	READ(CHIP_CLK_CTRL);

	// Turn off simple regulators/startup circuitry
	// also power down PLL and VCO amplifier so can set frequencies
	// turn on everything else
	
	// 15 - 0 Reserved
	// 14 - 1 Stereo DAC mode
	// 13 - 0 turn off VDDD simple regulator
	// 12 - 0 keep startup circuitry powered down
	// 11 - 1 power up charge pump
	// 10 - 0 don't power up PLL yet
	// 9  - 1 powerup primary VDDD linear regulator
	// 8  - 0 don't power up VCO amplifier yet
	
	// 7  - 1 power up VAG ref buffer
	// 6  - 1 stereo ADC
	// 5  - 1 power up reference bias currents
	// 4  - 1 power up headphone amplifiers
	// 3  - 1 power up DACS
	// 2  - 1 power up capless headphone mode
	// 1  - 1 power up ADCs
	// 0  - 1 power up lineout amplifiers
	
	WRITE(CHIP_ANA_POWER, 0x4AFF);
//	READ(CHIP_ANA_POWER);

	// power up all digital blocks execept for I2S output (not used)	
	// 15:7 - 0 reserved
	// 6 - 1 - enable ADC
	// 5 - 1 - enable DAC
	// 4 - 1 - enable DAP
	// 3 - 0 - reserved
	// 2 - 0 - reserved
	// 1 - 1 - don't enable I2S output
	// 0 - 1 - enable I2S input
		
	WRITE(CHIP_DIG_POWER, 0x0071);
//	READ(CHIP_DIG_POWER);

	// configure charge pump to use VDDIO rail
//	WRITE(CHIP_LINREG_CTRL, 0x006C);
//	READ(CHIP_LINREG_CTRL);
	
	// set ground, ADC, DAC reference voltage to VDDA/2
	WRITE(CHIP_REF_CTRL, 0x004E);
//	READ(CHIP_REF_CTRL);
	
	// set Line out reference voltage to VDDIO/2 (1.65V) (bits 5:0) and bias current (bits 11:8) to recommended value of 0.36mA for 10kOhm load with 1.0nF capacitance
	// set line out to 0.8V
	
	WRITE(CHIP_LINE_OUT_CTRL, 0x0320);
//	READ(CHIP_LINE_OUT_CTRL);
	
	// configure slow ramp up to minimize pop (bit 0)
	WRITE(CHIP_REF_CTRL, 0x004F);
//	READ(CHIP_REF_CTRL);

	//Enable short detect mode for headphone left/right
	// and center channel and set short detect current trip level to 75mA
	WRITE(CHIP_SHORT_CTRL, 0x1106);
//	READ(CHIP_SHORT_CTRL);

	
	// enable zero-cross detect for HP_OUT (bit 5) and ADC (bit 1)
	WRITE(CHIP_ANA_CTRL, 0x0133);
//	READ(CHIP_ANA_CTRL);

	
	// set lineout volume level based on voltage reference (VAG) values using this formula:
	// Value = (int)(40*log(VAG_VAL/LO_VAGCNTRL) + 15)
	// Assuming VAG_VAL and LO_VAGCNTRL is set to 0.9 V and 
	//	1.65 V respectively, the // left LO vol (bits 12:8) and right LO 
	//	volume (bits 4:0) value should be set 
	// to 5 
	
	// set to 0B0B to get 3db down
	//WRITE(CHIP_LINE_OUT_VOL, 0x0B0B);
	
	WRITE(CHIP_LINE_OUT_VOL, 0x0B0F);
	READ(CHIP_LINE_OUT_VOL);

	
	// set I2S
	//15:9 - Reserved
	// 8 - 1 - bitclock is 32x FS
	// 7 - 1 master mode
	// 6 - 1 data on falling edge of i2s clock
	// 5 - 1 16 bit data length
	// 4 - 1 "
	// 3 - 0 I2S mode
	// 2 - 0 "
	// 1 - 0 LR Align - data word starts 1 clock after transition
	// 0 - 0 polarity - 0 = left, 1 = right 
	
	WRITE(CHIP_I2S_CTRL,0x01F0);
//	READ(CHIP_I2S_CTRL);


	
	// set PLL
	
	// using 20.97152mhz clock > 20mHz so need to divide by two
	WRITE(CHIP_CLK_TOP_CTRL, 0x0008);
//	READ(CHIP_CLK_TOP_CTRL);
		
	// using 44.1kHz sample rate, so PLL output frequency should be 180.6336MHz
	// integer divisor: (floor 180.6336/(20.97152/2) = floor( 17.22) = 17 = 10001
	// frac divisor: (17.22 - 17) * 2048 = 464 = 0x01D0 = 00111010000
	// 10001001 11010000 = 0x89D0
	
	// if using 22.05kHz, PLL output should be 90.3168
	// integer divisor: (fllor 90.3168/(20.97152/2) = floor (8.61328125) = 8 = 01000
	// frac divisor: (8.61328125-8)*2048 = 1256 = 0b10011101000
	// 01000100 11101000 = 0x;
	
	WRITE(CHIP_PLL_CTRL, 0x44E8);
//	READ(CHIP_PLL_CTRL);


	
	// now powerup PLL and VCO amplifier
	// also power down PLL and VCO amplifier so can set frequencies
	// turn on everything else
	
	// 15 - 0 Reserved
	// 14 - 1 Stereo DAC mode
	// 13 - 0 turn off VDDD simple regulator
	// 12 - 0 keep startup circuitry powered down
	// 11 - 1 power up charge pump
	// 10 - 1 power up PLL 
	// 9  - 1 powerup primary VDDD linear regulator
	// 8  - 1 power up VCO amplifier yet
	
	// 7  - 1 power up VAG ref buffer
	// 6  - 1 stereo ADC
	// 5  - 1 power up reference bias currents
	// 4  - 1 power up headphone amplifiers
	// 3  - 1 power up DACS
	// 2  - 1 power up capless headphone mode
	// 1  - 1 power up ADCs
	// 0  - 1 power up lineout amplifiers
	
	WRITE(CHIP_ANA_POWER, 0x4FFF);
//	READ(CHIP_ANA_POWER);


	// Now OK to use PLL for MCLK 
	// 15:6 0 reserved
	// 5:4 0 SYS_FS specifies the rate
	// 3 0 0x1 44.1KHz
	// 2 1 "
	// 1 1 - use PLL
	// 0 1
	
	WRITE(CHIP_CLK_CTRL, 0x0007);
	READ(CHIP_CLK_CTRL);
	
}
void SGTL5000_UsePassthrough(xSemaphoreHandle done)
{
	
	// turn on headphone passthrough and mute ADC
	// 0 - unmute lineout
	// 0 - reserved
	// 1 - Select linein as headphone input
	// 1 -enable headphone zero cross detection 
	// 0 - unmute headphone output
	// 0 - reserved
	// 0 - select linein as ADC input
	// 0 - enable ADC analog zero cross detector
	// 0 - unmute ADC analog volume
	
	WRITE(CHIP_ANA_CTRL, 0x0060);
	READ(CHIP_ANA_CTRL);

	// mute DACs for mixer
	// 15 - 0 reserved
	// 14 - 0 reserved
	// 13 - 0 volume busy DAC right (read only)
	// 12 - 0 volume busy DAC left  (read only)
	// 11 - 0 reserved
	// 10 - 0 reserved
	// 9  - 1 volume ramp enabled
	// 8  - 0 use linear ramp 
	
	// 7  - 0 reserved
	// 6  - 0 reserved
	// 5  - 0 reserved
	// 4  - 0 reserved
	// 3  - 1 mute right DAC
	// 2  - 1 mute left DAC
	// 1  - 0 normal high pass filter operation (can freeze DC offset)
	// 0  - 0 normal high pass filter operation

//	WRITE(CHIP_ADCDAC_CTRL, 0x020C);
//	READ(CHIP_ADCDAC_CTRL);

	// disable DAP and mixer
	
	// 15:5 reserved
	// 4 	- 0 Disable DAP mixer path
	// 3:1 reserved
	// 0 	- 0 Disable DAP (needs to be enabled to use mixer)
	
//	WRITE(DAP_CONTROL, 0x0000);
//	READ(DAP_CONTROL);

	// turn off digital blocks
	// 15:7 reserved
	// 6 - 0 Enable ADC block
	// 5 - 0 Enable DAC block
	// 4 - 0 Enable DAP block
	// 3 - 0 reserved
	// 2 - 0 reserved
	// 1 - 0 leave I2S output block off
	// 0 - 0 enable I2S data input
		
//	WRITE(CHIP_DIG_POWER, 0x0000);
//	READ(CHIP_DIG_POWER);
	
	// power down analog blocks for passthrough only mode (also turns off lineout amplifiers)
	
	// mute DACs for mixer
	// 15 - 0 Reserved
	// 14 - 1 Stereo DAC mode
	// 13 - 0 turn off VDDD simple regulator
	// 12 - 0 keep startup circuitry powered down
	// 11 - 1 keep charge pump powered up
	// 10 - 1 keep PLL powered up
	// 9  - 1 powerup primary VDDD linear regulator
	// 8  - 1 keep PLL VCO amplifier powered up
	
	// 7  - 1 power up VAG ref buffer
	// 6  - 1 stereo ADC
	// 5  - 1 power up reference bias currents
	// 4  - 1 power up headphone amplifiers
	// 3  - 0 power down DACS
	// 2  - 1 power up capless headphone mode
	// 1  - 0 power down ADCs
	// 0  - 0 power down lineout amplifiers
	
//	WRITE(CHIP_ANA_POWER, 0x4FF4);
//	READ(CHIP_ANA_POWER);
		
}

void SGTL5000_UseMixer(xSemaphoreHandle done)
{
	
	// power up  digital blocks
	// 15:7 reserved
	// 6 - 1 Enable ADC block
	// 5 - 1 Enable DAC block
	// 4 - 1 Enable DAP block
	// 3 - 0 reserved
	// 2 - 0 reserved
	// 1 - 0 leave I2S output block off
	// 0 - 1 enable I2S data input
	
	WRITE(CHIP_DIG_POWER, 0x0071);
	READ(CHIP_DIG_POWER);
	
	// set mixer control
	// 15 - 0 reserved
	// 14 - 0 DAP mixer swap normal mode
	// 13 - 0 DAP input swap normal mode
	// 12 - 0 DAC_LRSWAP normal operation
	// 11 - 0 reserved
	// 10 - 0 I2S_DOUT swap normal operation
	// 9  - 0 select I2S_IN as data source for DAP mixer (mix channel)
	// 8  - 1 "
	
	// 7  - 0 select ADC as data source for DAP (main channel)
	// 6  - 0 "
	// 5  - 1 select DAP as data source for DAC
	// 4  - 1 " 
	// 3  - 0 Reserved
	// 2  - 0 reserved
	// 1  - 0 select ADC as data source for I2S out
	// 0  - 0
	
	WRITE(CHIP_SSS_CTRL, 0x0130);
	READ(CHIP_SSS_CTRL);

	
	// unmute DACs for mixer
	// unmute ADC
	// 15 - 0 reserved
	// 14 - 0 reserved
	// 13 - 0 volume busy DAC right (read only)
	// 12 - 0 volume busy DAC left  (read only)
	// 11 - 0 reserved
	// 10 - 0 reserved
	// 9  - 1 volume ramp enabled
	// 8  - 0 use linear ramp 
	
	// 7  - 0 reserved
	// 6  - 0 reserved
	// 5  - 0 reserved
	// 4  - 0 reserved
	// 3  - 0 unmute right DAC
	// 2  - 0 unmute left DAC
	// 1  - 0 normal high pass filter operation (can freeze DC offset)
	// 0  - 0 normal high pass filter operation

	WRITE(CHIP_ADCDAC_CTRL, 0x0200);
	READ(CHIP_ADCDAC_CTRL);

	// set chip DAC volume to 0dB (3C, 0.5017dB steps, with 0x3C = 0dB.  0xF0= -90dB
	WRITE(CHIP_DAC_VOL, 0x3C3C);
	READ(CHIP_DAC_VOL);
	
	// set ADC volume to 0dB (3C, 0.5017dB steps, with 0x3C = 0dB.  0xF0= -90dB
	WRITE(CHIP_ANA_ADC_CTRL, 0x0000);
	READ(CHIP_ANA_ADC_CTRL);
	
	// set DAP main channel volume to 100%
	WRITE(DAP_MAIN_CHAN, 0x8000);
	READ(DAP_MAIN_CHAN);
	
	// set DAP mix volume to 100%
	WRITE(DAP_MIX_CHAN, 0x8000);
	READ(DAP_MIX_CHAN);
		
	
	
	// enable DAP and mixer
	
	// 15:5 reserved
	// 4 	- 1 Enable DAP mixer path
	// 3:1 reserved
	// 0 	- 1 Enable DAP (needs to be enabled to use mixer)
	
	WRITE(DAP_CONTROL, 0x0011);
	READ(DAP_CONTROL);

	// power up blocks for mixer mode
		
	// mute DACs for mixer
	// 15 - 0 Reserved
	// 14 - 1 Stereo DAC mode
	// 13 - 0 turn off VDDD simple regulator
	// 12 - 0 keep startup circuitry powered down
	// 11 - 1 power up charge pump
	// 10 - 1 power up PLL
	// 9  - 1 powerup primary VDDD linear regulator
	// 8  - 1 power up PLL VCO amplifier
	
	// 7  - 1 power up VAG ref buffer
	// 6  - 1 stereo ADC
	// 5  - 1 power up reference bias currents
	// 4  - 1 power up headphone amplifiers
	// 3  - 1 power up DACS
	// 2  - 1 power up capless headphone mode
	// 1  - 1 power up ADCs
	// 0  - 1 power up lineout amplifiers
		
	WRITE(CHIP_ANA_POWER, 0x4FFF);
	READ(CHIP_ANA_POWER);
	
	
	// set routing through DAP and mixer
	// 15:9 0 - reserved
	// 8 - 0 - unmute lineout
	
	// 7 - 0 - reserved
	// 6 - 0 - Select DAC as headphone input
	// 5 - 1 -enable headphone zero cross detection 
	// 4 - 0 - unmute headphone output
	
	// 3 - 0 - reserved
	// 2 - 1 - select linein as ADC input
	// 1 - 1 - enable ADC analog zero cross detector
	// 0 - 0 - unmute ADC analog volume

	WRITE(CHIP_ANA_CTRL, 0x0026);
	READ(CHIP_ANA_CTRL);
	
}
void SGTL5000_SetVolume(uint8 volume,xSemaphoreHandle done)
{
	// volume in range from 0 to 10 (0 mute, 10 max)
	// 0x3C (60)= 0dB
	// 0xF0 (240) = 90dB
	// 0xFC and greater = muted
	
	uint16 volbits;
	if(volume == 0)
	{
		volbits = 0xFCFC; // muted
	}
	else if(volume > 10)
	{
		volbits = 0x3C3C;
	}
	else // volume between 1 and 10
	{
		uint8 v;
		v = (10-volume)*((120-60)/10) + 60; // i.e. if volume = 9, 1*((240-60)/10)= 18= 0
		
		// volume  	v
		// 10		18+60 = 78 // 0 dB
		// 9		36+60 = 96
		// ..
		// 2		162+60 = 222
		// 1		180+60 = 240 // 
		// 0 		192+60 = 252 // muted
		
		volbits = v << 8 | v;
		
	}
	
	WRITE(CHIP_DAC_VOL, volbits);
	READ(CHIP_DAC_VOL);
	
}

