/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include <stdint.h>
#include <string.h>
#include "stm32/stm32f10x.h"
#include "platform_config.h"
#include "main.h"
#include "rtc.h"
#include "fat_sd/diskio.h" /* disk_timerproc */
#include "fat_sd/ff.h"
#include "ui_lcd.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
BYTE Buff[48 * 1024] __attribute__ ((aligned (4)));
/* Working buffer */
FIL File1;
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void TFT_delay(int time);
//-----------------------
int TFT_18Z_demo(void);
void TFT_18Z_Init(void);
void TFT_18Z_Write_Command(unsigned char command);
void TFT_18Z_Write_Data(unsigned char data1);
void TFT_18Z_FULL_ON(unsigned int dat);
//-----------------------
int TFT_24S_demo(void);
void TFT_24S_Init(void);
void TFT_24S_Write_Command(unsigned int command);
void TFT_24S_Write_Data(unsigned int data1);
void TFT_24S_FULL_ON(unsigned int dat);
//-----------------------
int TFT_35M_demo(void);
void TFT_35M_Init(void);
void TFT_35M_Write_Command(unsigned char command);
void TFT_35M_Write_Data(unsigned char data1);
void TFT_35M_Command_Write(unsigned char REG, unsigned char VALUE);
void TFT_35M_SendData(unsigned long color);
void TFT_35M_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y);
void TFT_35M_FULL_ON(unsigned long dat);
//-----------------------
int TFT_43M_demo(void);
void TFT_43M_Init(void);
void TFT_43M_Write_Command(unsigned char command);
void TFT_43M_Write_Data(unsigned char data1);
void TFT_43M_Command_Write(unsigned char REG, unsigned char VALUE);
void TFT_43M_SendData(unsigned long color);
void TFT_43M_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y);
void TFT_43M_FULL_ON(unsigned long dat);
//-----------------------
int TFT_57B_demo(void);
void TFT_57B_Init(void);
void TFT_57B_Write_Command(unsigned char command);
void TFT_57B_Write_Data(unsigned char data1);
void TFT_57B_Command_Write(unsigned char REG, unsigned char VALUE);
void TFT_57B_SendData(unsigned long color);
void TFT_57B_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y);
void TFT_57B_FULL_ON(unsigned long dat);
//-----------------------
int TFT_57F_demo(void);
void TFT_57F_Init(void);
void TFT_57F_Write_Command(unsigned char command);
void TFT_57F_Write_Data(unsigned char data1);
void TFT_57F_Command_Write(unsigned char REG, unsigned char VALUE);
void TFT_57F_SendData(unsigned long color);
void TFT_57F_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y);
void TFT_57F_FULL_ON(unsigned long dat);
/* Private functions ---------------------------------------------------------*/
void TFT_delay(int time) {
	int i, j;
	for (i = 0; i < (time ^ 1000); i++) {
		for (j = 0; j < (time ^ 1000); j++) {
			;
		}
	}
}
/*-------------------------------------------------------*/
/*------------------3.5"---------------------------------*/
/*-------------------------------------------------------*/
/*------------------1.8"---------------------------------*/
int TFT_18Z_demo(void) {
	char filename[18] = "1_8_128160ZF_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));
	TFT_delay(10);
	TFT_18Z_Init();
	TFT_delay(10);

	filename[13] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_18Z_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		f_read(&File1, Buff, 38400, &blen);
		for (i = 0; i < 38400; i++) {
			GPIO_Write(GPIOB, Buff[i]);
			GPIO_ResetBits(GPIOC, nWR);
			GPIO_SetBits(GPIOC, nWR);
		}

		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECTED");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DISK_CHANGE");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("not open");
				image = 0x30;
			}
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : TFT_18Z_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_18Z_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	delay(5);
	TFT_delay(10);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	delay(100);
	TFT_delay(10);
	TFT_18Z_Write_Command(0x11);
	TFT_delay(100);
	TFT_18Z_Write_Command(0x26);
	TFT_18Z_Write_Data(0x04);
	TFT_18Z_Write_Command(0xF2);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Command(0xB1);
	TFT_18Z_Write_Data(0x0A);
	TFT_18Z_Write_Data(0x14);
	TFT_18Z_Write_Command(0xC0);
	TFT_18Z_Write_Data(0x0A);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Command(0xC1);
	TFT_18Z_Write_Data(0x02);
	TFT_18Z_Write_Command(0xC5);
	TFT_18Z_Write_Data(0x2F);
	TFT_18Z_Write_Data(0x3E);
	TFT_18Z_Write_Command(0xC7);
	TFT_18Z_Write_Data(0x40);
	TFT_18Z_Write_Command(0x2A);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x7F);
	TFT_18Z_Write_Command(0x2B);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x00);
	TFT_18Z_Write_Data(0x9F);
	TFT_18Z_Write_Command(0x36);
	TFT_18Z_Write_Data(0xC0);
	TFT_18Z_Write_Command(0x3A);
	TFT_18Z_Write_Data(0xC5);
	TFT_18Z_Write_Command(0x29);
	TFT_18Z_Write_Command(0x2C);
}

void TFT_18Z_Write_Command(unsigned char command) {
	GPIO_ResetBits(GPIOC, RS);
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_18Z_Write_Data(unsigned char data1) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, data1);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_18Z_FULL_ON(unsigned int dat) {
	unsigned long x;
	for (x = 0; x < 19200; x++) {
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}

/*-------------------------------------------------------*/
/*------------------2.4"---------------------------------*/
int TFT_24S_demo(void) {
	char filename[18] = "2_4_240320SF_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, n, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));
	TFT_delay(10);
	TFT_24S_Init();
	TFT_delay(10);

	filename[13] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_24S_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		for (n = 0; n < 5; n++) { //read from file 5 times, 30KB each time
			f_read(&File1, Buff, 30720, &blen); //320x240x2 = 150KB per screen
			for (i = 0; i < 30720; i++) {
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i]);
				GPIO_ResetBits(GPIOC, nWR);
			}
		}

		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECTED");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DISK_CHANGE");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("not open");
				image = 0x30;
			}
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : TFT_24Z_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_24S_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	TFT_delay(10);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	TFT_delay(100);
	TFT_24S_Write_Command(0x00E5);
	TFT_24S_Write_Data(0x8000);
	TFT_24S_Write_Command(0x0000);
	TFT_24S_Write_Data(0x0001);
	TFT_24S_Write_Command(0x0001);
	TFT_24S_Write_Data(0x0100);
	TFT_24S_Write_Command(0x0002);
	TFT_24S_Write_Data(0x0700);
	TFT_24S_Write_Command(0x0003);
	TFT_24S_Write_Data(0x1030);
	TFT_24S_Write_Command(0x0004);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0008);
	TFT_24S_Write_Data(0x0202);
	TFT_24S_Write_Command(0x0009);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x000A);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x000C);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x000D);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x000F);
	TFT_24S_Write_Data(0x0000);

	TFT_24S_Write_Command(0x0010);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0011);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0012);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0013);
	TFT_24S_Write_Data(0x0000);
	TFT_delay(100);
	TFT_24S_Write_Command(0x0010);
	TFT_24S_Write_Data(0x17B0);
	TFT_24S_Write_Command(0x0011);
	TFT_24S_Write_Data(0x0137);
	TFT_delay(100);
	TFT_24S_Write_Command(0x0012);
	TFT_24S_Write_Data(0x013B);
	TFT_delay(100);
	TFT_24S_Write_Command(0x0013);
	TFT_24S_Write_Data(0x1900);
	TFT_24S_Write_Command(0x0029);
	TFT_24S_Write_Data(0x0007);
	TFT_24S_Write_Command(0x002B);
	TFT_24S_Write_Data(0x0020);
	TFT_delay(100);
	TFT_24S_Write_Command(0x0020);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0021);
	TFT_24S_Write_Data(0x0000);

	TFT_24S_Write_Command(0x0030);
	TFT_24S_Write_Data(0x0007);
	TFT_24S_Write_Command(0x0031);
	TFT_24S_Write_Data(0x0504);
	TFT_24S_Write_Command(0x0032);
	TFT_24S_Write_Data(0x0703);
	TFT_24S_Write_Command(0x0035);
	TFT_24S_Write_Data(0x0002);
	TFT_24S_Write_Command(0x0036);
	TFT_24S_Write_Data(0x0707);
	TFT_24S_Write_Command(0x0037);
	TFT_24S_Write_Data(0x0406);
	TFT_24S_Write_Command(0x0038);
	TFT_24S_Write_Data(0x0006);
	TFT_24S_Write_Command(0x0039);
	TFT_24S_Write_Data(0x0404);
	TFT_24S_Write_Command(0x003C);
	TFT_24S_Write_Data(0x0700);
	TFT_24S_Write_Command(0x003D);
	TFT_24S_Write_Data(0x0A08);

	TFT_24S_Write_Command(0x0050);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0051);
	TFT_24S_Write_Data(0x00EF);
	TFT_24S_Write_Command(0x0052);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0053);
	TFT_24S_Write_Data(0x013F);
	TFT_24S_Write_Command(0x0060);
	TFT_24S_Write_Data(0x2700);
	TFT_24S_Write_Command(0x0061);
	TFT_24S_Write_Data(0x0001);
	TFT_24S_Write_Command(0x006A);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0090);
	TFT_24S_Write_Data(0x0010);
	TFT_24S_Write_Command(0x0092);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0093);
	TFT_24S_Write_Data(0x0003);
	TFT_24S_Write_Command(0x0095);
	TFT_24S_Write_Data(0x0110);
	TFT_24S_Write_Command(0x0097);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0098);
	TFT_24S_Write_Data(0x0000);
	TFT_24S_Write_Command(0x0007);
	TFT_24S_Write_Data(0x0173);
	TFT_delay(100);
	TFT_24S_Write_Command(0x0022);
}

void TFT_24S_Write_Command(unsigned int command) {
	GPIO_ResetBits(GPIOC, RS);
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_24S_Write_Data(unsigned int data1) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, data1);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_24S_FULL_ON(unsigned int dat) {
	unsigned long x;
	for (x = 0; x < 76800; x++) {
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}

int TFT_35M_demo(void) {
	char filename[18] = "3_5_320240MF_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, n, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));
	TFT_delay(10);
	TFT_35M_Init();
	//TFT_35M_QUADS();
	TFT_delay(10);
	TFT_35M_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_35M_Write_Command(0x2c);

	filename[13] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_35M_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		TFT_35M_Write_Command(0x28); //SET display off
		TFT_35M_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
		TFT_35M_Write_Command(0x2c);
		GPIO_SetBits(GPIOC, RS);
		for (n = 0; n < 54; n++) {
			GPIO_SetBits(GPIOC, nWR);
			GPIO_Write(GPIOB, 0x00);
			GPIO_ResetBits(GPIOC, nWR);
		}

		for (n = 0; n < 5; n++) { //read from file 5 times, 45KB each time
			f_read(&File1, Buff, 46080, &blen); //320x240xRGB = 225KB per screen
			for (i = 0; i < 46080; i++) {
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i]);
				GPIO_ResetBits(GPIOC, nWR);
			}
		} //read next 45KB bytes
		TFT_35M_Write_Command(0x29); //SET display on


		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECTED");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DISK_CHANGE");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("not open");
				image = 0x30;
			}
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : TFT_35M_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_35M_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	delay(5);
	TFT_delay(10);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	delay(100);
	TFT_delay(10);
	TFT_35M_Write_Command(0x01); //Software Reset
	delay(10);
	TFT_delay(10);
	TFT_35M_Write_Command(0x01); //Software Reset
	delay(10);
	TFT_delay(10);
	TFT_35M_Command_Write(0xe0, 0x01); //START PLL
	TFT_35M_Command_Write(0xe0, 0x03); //LOCK PLL
	TFT_35M_Write_Command(0xb0); //SET LCD MODE  SET TFT 18Bits MODE
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x0c); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_35M_Write_Data(0x80); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_35M_Write_Data(0x01); //SET horizontal size=320-1 HightByte
	TFT_35M_Write_Data(0x3f); //SET horizontal size=320-1 LowByte
	TFT_35M_Write_Data(0x00); //SET vertical size=240-1 HightByte
	TFT_35M_Write_Data(0xef); //SET vertical size=240-1 LowByte
	TFT_35M_Write_Data(0x00); //SET even/odd line RGB seq.=RGB
	TFT_35M_Command_Write(0xf0, 0x00); //SET pixel data I/F format=8bit
	TFT_35M_Command_Write(0x3a, 0x60); // SET R G B format = 6 6 6
	TFT_35M_Write_Command(0xe6); //SET PCLK freq=4.94MHz  ; pixel clock frequency
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x0a); //02
	TFT_35M_Write_Data(0x3d); //ff
	TFT_35M_Write_Data(0x70); //ff
	TFT_35M_Write_Command(0xb4); //SET HBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x01); //SET HSYNC Total=440
	TFT_35M_Write_Data(0xb8);
	TFT_35M_Write_Data(0x00); //SET HBP 68
	TFT_35M_Write_Data(0x46);
	TFT_35M_Write_Data(0x0f); //SET VBP 16=15+1
	TFT_35M_Write_Data(0x00); //SET Hsync pulse start position
	TFT_35M_Write_Data(0x00);
	TFT_35M_Write_Data(0x00); //SET Hsync pulse subpixel start position
	TFT_35M_Write_Command(0xb6); //SET VBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x01); //SET Vsync total
	TFT_35M_Write_Data(0x48);
	TFT_35M_Write_Data(0x00); //SET VBP=19
	TFT_35M_Write_Data(0x0b);
	TFT_35M_Write_Data(0x07); //SET Vsync pulse 8=7+1
	TFT_35M_Write_Data(0x00); //SET Vsync pulse start position
	TFT_35M_Write_Data(0x00);
	TFT_35M_Write_Command(0x2a); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x00); //SET start column address=0
	TFT_35M_Write_Data(0x00);
	TFT_35M_Write_Data(0x01); //SET end column address=319
	TFT_35M_Write_Data(0x3f);
	TFT_35M_Write_Command(0x2b); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(0x00); //SET start page address=0
	TFT_35M_Write_Data(0x00);
	TFT_35M_Write_Data(0x00); //SET end page address=239
	TFT_35M_Write_Data(0xef);
	TFT_35M_Write_Command(0x13); //SET normal mode
	TFT_35M_Write_Command(0x29); //SET display on
}

void TFT_35M_Write_Command(unsigned char command) {
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_35M_Write_Data(unsigned char data1) {
	GPIO_Write(GPIOB, data1);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_35M_Command_Write(unsigned char REG, unsigned char VALUE) {
	TFT_35M_Write_Command(REG);
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data(VALUE);
}

void TFT_35M_SendData(unsigned long color) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, (color >> 16));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color >> 8));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_35M_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y) {
	TFT_35M_Write_Command(0x2a); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data((s_x) >> 8); //SET start page address=0
	TFT_35M_Write_Data(s_x);
	TFT_35M_Write_Data((e_x) >> 8); //SET end page address=319
	TFT_35M_Write_Data(e_x);

	TFT_35M_Write_Command(0x2b); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_35M_Write_Data((s_y) >> 8); //SET start column address=0
	TFT_35M_Write_Data(s_y);
	TFT_35M_Write_Data((e_y) >> 8); //SET end column address=239
	TFT_35M_Write_Data(e_y);
}

void TFT_35M_FULL_ON(unsigned long dat) {
	unsigned long x;
	TFT_35M_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_35M_Write_Command(0x2c);
	for (x = 0; x < 307200; x++) {
		//TFT_35M_SendData(dat);
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat >> 16));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}

void TFT_35M_QUADS(void) {
	unsigned int row, column;
	TFT_35M_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_35M_Write_Command(0x2c);
	GPIO_SetBits(GPIOC, RS);
	for (row = 0; row < 240; row++) {
		for (column = 0; column < 22; column++) {
			TFT_35M_SendData(0xFFFFFF);//white
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xFF69B4);//pink
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xFFFF00);//yellow
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xFF6600);//orange
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xFF0000);//red
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x8B0000);//dark red
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xCC0099);//red-violet
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x660099);//violet
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x0000FF);//blue
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x00FF00);//green
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x228B22);//forest green
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0xA52A2A);//brown
		}
		for (column = 0; column < 23; column++) {
			TFT_35M_SendData(0x696969);//gray
		}
		for (column = 0; column < 22; column++) {
			TFT_35M_SendData(0x000000);//black
		}
	}
}
/*-------------------------------------------------------*/
/*------------------4.3"---------------------------------*/

int TFT_43M_demo(void) {
	char filename[18] = "4_3_480272MF_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, n, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));
	TFT_delay(10);
	TFT_43M_Init();
	TFT_delay(10);
	TFT_43M_WindowSet(0x0000, 0x01df, 0x0000, 0x010f);
	TFT_43M_Write_Command(0x2c);

	filename[13] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_43M_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		//TFT_43M_Write_Command(0x28);		//SET display off
		GPIO_ResetBits(GPIOC, DISP);
		TFT_43M_WindowSet(0x0000, 0x01df, 0x0000, 0x010f);
		TFT_43M_Write_Command(0x2c);
		GPIO_SetBits(GPIOC, RS);
		for (n = 0; n < 54; n++) {
			GPIO_SetBits(GPIOC, nWR);
			GPIO_Write(GPIOB, 0x00);
			GPIO_ResetBits(GPIOC, nWR);
		}

		for (n = 0; n < 8; n++) { //read from file 9 times, 42.5KB each time
			f_read(&File1, Buff, 48960, &blen); //480x272xRGB = 382.5KB per screen
			for (i = 0; i < 48960; i += 3) {
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i + 2]);
				GPIO_ResetBits(GPIOC, nWR);
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i + 1]);
				GPIO_ResetBits(GPIOC, nWR);
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i]);
				GPIO_ResetBits(GPIOC, nWR);
			}
		} //read next 42.5KB bytes
		GPIO_SetBits(GPIOC, DISP);
		//TFT_43M_Write_Command(0x29);		//SET display on


		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		if (image == 0x3A) {
			image = 0x41;
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECTED");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DISK_CHANGE");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("not open");
				image = 0x30;
			}
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : TFT_43M_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_43M_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, DISP);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	delay(5);
	TFT_delay(10);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	delay(100);
	TFT_delay(10);
	TFT_43M_Write_Command(0x01); //Software Reset
	delay(10);
	TFT_delay(10);
	TFT_43M_Write_Command(0x01); //Software Reset
	delay(10);
	TFT_delay(10);
	TFT_43M_Command_Write(0xe0, 0x01); //START PLL
	TFT_43M_Command_Write(0xe0, 0x03); //LOCK PLL
	TFT_43M_Write_Command(0xb0); //SET LCD MODE  SET TFT 18Bits MODE
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x08); //SET TFT MODE - 0x0c
	TFT_43M_Write_Data(0x80); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_43M_Write_Data(0x01); //SET horizontal size=480-1 HightByte
	TFT_43M_Write_Data(0xdf); //SET horizontal size=480-1 LowByte
	TFT_43M_Write_Data(0x01); //SET vertical size=272-1 HightByte
	TFT_43M_Write_Data(0x0f); //SET vertical size=272-1 LowByte
	TFT_43M_Write_Data(0x00); //SET even/odd line RGB seq.=RGB
	TFT_43M_Command_Write(0xf0, 0x00); //SET pixel data I/F format=8bit
	TFT_43M_Command_Write(0x3a, 0x60); // SET R G B format = 6 6 6
	TFT_43M_Write_Command(0xe6); //SET PCLK freq=4.94MHz  ; pixel clock frequency
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x01); //02
	TFT_43M_Write_Data(0x45); //ff
	TFT_43M_Write_Data(0x47); //ff
	TFT_43M_Write_Command(0xb4); //SET HBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x02); //SET HSYNC Total=600
	TFT_43M_Write_Data(0x0d);
	TFT_43M_Write_Data(0x00); //SET HBP 68
	TFT_43M_Write_Data(0x2b);
	TFT_43M_Write_Data(0x28); //SET VBP 16=15+1
	TFT_43M_Write_Data(0x00); //SET Hsync pulse start position
	TFT_43M_Write_Data(0x00);
	TFT_43M_Write_Data(0x00); //SET Hsync pulse subpixel start position
	TFT_43M_Write_Command(0xb6); //SET VBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x01); //SET Vsync total=360
	TFT_43M_Write_Data(0x1d);
	TFT_43M_Write_Data(0x00); //SET VBP=19
	TFT_43M_Write_Data(0x0c);
	TFT_43M_Write_Data(0x09); //SET Vsync pulse 8=7+1
	TFT_43M_Write_Data(0x00); //SET Vsync pulse start position
	TFT_43M_Write_Data(0x00);
	TFT_43M_Write_Command(0x2a); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x00); //SET start column address=0
	TFT_43M_Write_Data(0x00);
	TFT_43M_Write_Data(0x01); //SET end column address=479
	TFT_43M_Write_Data(0xdf);
	TFT_43M_Write_Command(0x2b); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(0x00); //SET start page address=0
	TFT_43M_Write_Data(0x00);
	TFT_43M_Write_Data(0x01); //SET end page address=271
	TFT_43M_Write_Data(0x0f);
	TFT_43M_Write_Command(0x13); //SET normal mode
	TFT_43M_Write_Command(0x38); //SET normal mode
	TFT_43M_Write_Command(0x29); //SET display on
}

void TFT_43M_Write_Command(unsigned char command) {
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_43M_Write_Data(unsigned char data1) {
	GPIO_Write(GPIOB, data1);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_43M_Command_Write(unsigned char REG, unsigned char VALUE) {
	TFT_43M_Write_Command(REG);
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data(VALUE);
}

void TFT_43M_SendData(unsigned long color) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, (color >> 16));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color >> 8));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_43M_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y) {
	TFT_43M_Write_Command(0x2a); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data((s_x) >> 8); //SET start page address=0
	TFT_43M_Write_Data(s_x);
	TFT_43M_Write_Data((e_x) >> 8); //SET end page address=319
	TFT_43M_Write_Data(e_x);

	TFT_43M_Write_Command(0x2b); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_43M_Write_Data((s_y) >> 8); //SET start column address=0
	TFT_43M_Write_Data(s_y);
	TFT_43M_Write_Data((e_y) >> 8); //SET end column address=239
	TFT_43M_Write_Data(e_y);
}

void TFT_43M_FULL_ON(unsigned long dat) {
	unsigned long x;
	TFT_43M_WindowSet(0x0000, 0x01df, 0x0000, 0x010f);
	TFT_43M_Write_Command(0x2c);
	for (x = 0; x < 130560; x++) {
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat >> 16));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}
/*-------------------------------------------------------*/
/*------------------5.7-320240---------------------------*/
int TFT_57B_demo(void) {
	char filename[19] = "5_7_320240WFB_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, n, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));

	TFT_57B_Init();
	TFT_57B_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_57B_Write_Command(0x2c);

	filename[14] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_57B_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		TFT_57B_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
		TFT_57B_Write_Command(0x2c);
		GPIO_SetBits(GPIOC, RS);
		for (n = 0; n < 54; n++) {
			GPIO_SetBits(GPIOC, nWR);
			GPIO_Write(GPIOB, 0x00);
			GPIO_ResetBits(GPIOC, nWR);
		}

		for (n = 0; n < 32; n++) { //read from file 32 times, 7200 bytes each time
			f_read(&File1, Buff, 7200, &blen); // read line of pixels
			for (i = 0; i < 7200; i++) {
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i]);
				GPIO_ResetBits(GPIOC, nWR);
			}
		} //read next 7200 bytes

		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		filename[14] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				image = 0x30;
			}
		}
		filename[14] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : UILCD_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_57B_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	TFT_delay(5);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	TFT_delay(100);
	TFT_57B_Write_Command(0x01); //Software Reset
	TFT_57B_Write_Command(0x01);
	TFT_57B_Write_Command(0x01);
	TFT_delay(10);
	TFT_57B_Command_Write(0xe0, 0x01); //START PLL
	TFT_57B_Command_Write(0xe0, 0x03); //LOCK PLL
	TFT_57B_Write_Command(0xb0); //SET LCD MODE  SET TFT 18Bits MODE
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x0c); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_57B_Write_Data(0x80); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_57B_Write_Data(0x01); //SET horizontal size=640-1 HightByte
	TFT_57B_Write_Data(0x3f); //SET horizontal size=640-1 LowByte
	TFT_57B_Write_Data(0x00); //SET vertical size=480-1 HightByte
	TFT_57B_Write_Data(0xEf); //SET vertical size=480-1 LowByte
	TFT_57B_Write_Data(0x00); //SET even/odd line RGB seq.=RGB
	TFT_57B_Command_Write(0xf0, 0x00); //SET pixel data I/F format=8bit
	TFT_57B_Command_Write(0x3a, 0x60); // SET R G B format = 6 6 6
	TFT_57B_Write_Command(0xe6); //SET PCLK freq=4.94MHz  ; pixel clock frequency
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x00);
	TFT_57B_Write_Data(0xE7);
	TFT_57B_Write_Data(0x4f);
	TFT_57B_Write_Command(0xb4); //SET HBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x01); //SET HSYNC Total=760
	TFT_57B_Write_Data(0xB8);
	TFT_57B_Write_Data(0x00); //SET HBP 68
	TFT_57B_Write_Data(0x44);
	TFT_57B_Write_Data(0x0f); //SET VBP 16=15+1
	TFT_57B_Write_Data(0x00); //SET Hsync pulse start position
	TFT_57B_Write_Data(0x00);
	TFT_57B_Write_Data(0x00); //SET Hsync pulse subpixel start position
	TFT_57B_Write_Command(0xb6); //SET VBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x01); //SET Vsync total
	TFT_57B_Write_Data(0x08);
	TFT_57B_Write_Data(0x00); //SET VBP=19
	TFT_57B_Write_Data(0x13);
	TFT_57B_Write_Data(0x07); //SET Vsync pulse 8=7+1
	TFT_57B_Write_Data(0x00); //SET Vsync pulse start position
	TFT_57B_Write_Data(0x00);
	TFT_57B_Write_Command(0x2a); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x00); //SET start column address=0
	TFT_57B_Write_Data(0x00);
	TFT_57B_Write_Data(0x01); //SET end column address=639
	TFT_57B_Write_Data(0x3f);
	TFT_57B_Write_Command(0x2b); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(0x00); //SET start page address=0
	TFT_57B_Write_Data(0x00);
	TFT_57B_Write_Data(0x00); //SET end page address=479
	TFT_57B_Write_Data(0xEf);
	TFT_57B_Write_Command(0x29); //SET display on
}

void TFT_57B_Write_Command(unsigned char command) {
	//GPIO_SetBits(GPIOC, nRD);
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	//GPIO_ResetBits(GPIOC, CS1);
	//GPIO_SetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nWR);
	//GPIO_WriteBit(GPIOC, nRD, Bit_SET);
	//if((command & 0x01)==0){GPIO_WriteBit(GPIOB, DB0, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB0, Bit_SET);}
	//if((command & 0x02)==0){GPIO_WriteBit(GPIOB, DB1, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB1, Bit_SET);}
	//if((command & 0x04)==0){GPIO_WriteBit(GPIOB, DB2, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB2, Bit_SET);}
	//if((command & 0x08)==0){GPIO_WriteBit(GPIOB, DB3, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB3, Bit_SET);}
	//if((command & 0x10)==0){GPIO_WriteBit(GPIOB, DB4, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB4, Bit_SET);}
	//if((command & 0x20)==0){GPIO_WriteBit(GPIOB, DB5, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB5, Bit_SET);}
	//if((command & 0x40)==0){GPIO_WriteBit(GPIOB, DB6, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB6, Bit_SET);}
	//if((command & 0x80)==0){GPIO_WriteBit(GPIOB, DB7, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB7, Bit_SET);}
	//GPIO_WriteBit(GPIOC, RS, Bit_RESET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_SET);
}

void TFT_57B_Write_Data(unsigned char data1) {
	//GPIO_WriteBit(GPIOC, nRD, Bit_SET);
	//if((data1 & 0x01)==0){GPIO_WriteBit(GPIOB, DB0, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB0, Bit_SET);}
	//if((data1 & 0x02)==0){GPIO_WriteBit(GPIOB, DB1, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB1, Bit_SET);}
	//if((data1 & 0x04)==0){GPIO_WriteBit(GPIOB, DB2, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB2, Bit_SET);}
	//if((data1 & 0x08)==0){GPIO_WriteBit(GPIOB, DB3, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB3, Bit_SET);}
	//if((data1 & 0x10)==0){GPIO_WriteBit(GPIOB, DB4, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB4, Bit_SET);}
	//if((data1 & 0x20)==0){GPIO_WriteBit(GPIOB, DB5, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB5, Bit_SET);}
	//if((data1 & 0x40)==0){GPIO_WriteBit(GPIOB, DB6, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB6, Bit_SET);}
	//if((data1 & 0x80)==0){GPIO_WriteBit(GPIOB, DB7, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB7, Bit_SET);}
	//GPIO_WriteBit(GPIOC, RS, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_SET);
	GPIO_Write(GPIOB, data1);
	//GPIO_SetBits(GPIOC, nRD);
	//GPIO_SetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	//GPIO_ResetBits(GPIOC, CS1);
	//GPIO_SetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_57B_Command_Write(unsigned char REG, unsigned char VALUE) {
	TFT_57B_Write_Command(REG);
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data(VALUE);
}

void TFT_57B_SendData(unsigned long color) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, (color >> 16));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color >> 8));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	//TFT_57F_Write_Data((color)>>16);	//red
	//TFT_57F_Write_Data((color)>>8);		//green
	//TFT_57F_Write_Data(color);			//blue
}

void TFT_57B_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y) {
	TFT_57B_Write_Command(0x2a); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data((s_x) >> 8); //SET start page address=0
	TFT_57B_Write_Data(s_x);
	TFT_57B_Write_Data((e_x) >> 8); //SET end page address=639
	TFT_57B_Write_Data(e_x);

	TFT_57B_Write_Command(0x2b); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_57B_Write_Data((s_y) >> 8); //SET start column address=0
	TFT_57B_Write_Data(s_y);
	TFT_57B_Write_Data((e_y) >> 8); //SET end column address=479
	TFT_57B_Write_Data(e_y);
}

void TFT_57B_FULL_ON(unsigned long dat) {
	unsigned long x;
	TFT_57F_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_57F_Write_Command(0x2c);
	for (x = 0; x < 76800; x++) {
		//TFT_57F_SendData(dat);
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat >> 16));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}

void TFT_57B_QUADS(void) {
	unsigned int i, j;
	TFT_57B_WindowSet(0x0000, 0x013f, 0x0000, 0x00ef);
	TFT_57B_Write_Command(0x2c);
	for (j = 0; j < 120; j++) {
		for (i = 0; i < 160; i++) {
			TFT_57B_SendData(0x0000FF); //blue
		}
		for (i = 0; i < 160; i++) {
			TFT_57B_SendData(0xFF0000); //red
		}
	}
	for (j = 0; j < 120; j++) {
		for (i = 0; i < 160; i++) {
			TFT_57B_SendData(0xFFFF00); //yellow
		}
		for (i = 0; i < 160; i++) {
			TFT_57B_SendData(0x00FF00); //green
		}
	}
}

/*-------------------------------------------------------*/
/*------------------5.7-640480---------------------------*/
int TFT_57F_demo(void) {
	char filename[18] = "5_7_640480WF_";
	char filetype[4] = ".bmp";
	char filenumber[1] = "0";
	int i, n, result;
	char image = 0x30;
	UINT blen = sizeof(Buff);
	memset(Buff,3,sizeof(Buff));

	TFT_57F_Init();
	TFT_57F_WindowSet(0x0000, 0x027f, 0x0000, 0x01df);
	TFT_57F_Write_Command(0x2c);

	filename[13] = '\0';
	filenumber[0] = image;
	strncat(filename,filenumber,1);
	strncat(filename,filetype,4);

	while (1) {
		TFT_57F_Init();
		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Reading SD Card ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("                ");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString(filename);

		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				while (1)
					;
				break;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				while (1)
					;
				break;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				while (1)
					;
				break;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				while (1)
					;
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				while (1)
					;
				break;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				while (1)
					;
				break;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				while (1)
					;
				break;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				while (1)
					;
				break;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				while (1)
					;
				break;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				while (1)
					;
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				while (1)
					;
				break;
			}
			return result;
		}
		result = f_lseek(&File1, 108);
		if (result != FR_OK) {
			switch (result) {
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				break;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				break;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("R/W Error       ");
				break;
			}
			return result;
		}

		TFT_57F_WindowSet(0x0000, 0x027f, 0x0000, 0x01df);
		TFT_57F_Write_Command(0x2c);
		GPIO_SetBits(GPIOC, RS);
		for (n = 0; n < 54; n++) {
			GPIO_SetBits(GPIOC, nWR);
			GPIO_Write(GPIOB, 0x00);
			GPIO_ResetBits(GPIOC, nWR);
		}

		for (n = 0; n < 128; n++) { //read from file 128 times, 7200 bytes each time
			f_read(&File1, Buff, 7200, &blen); // read line of pixels
			for (i = 0; i < 7200; i++) {
				GPIO_SetBits(GPIOC, nWR);
				GPIO_Write(GPIOB, Buff[i]);
				GPIO_ResetBits(GPIOC, nWR);
			}
		} //read next 7200 bytes

		result = f_close(&File1); // close file
		if (result != FR_OK) {
			UILCD_SetCursor(0, 0);
			UILCD_DisplayString("File not Closed ");
			while (1)
				;
			return result;
		}

		UILCD_SetCursor(0, 0);
		UILCD_DisplayString("Press SEL button");
		UILCD_SetCursor(1, 0);
		UILCD_DisplayString("for next file.  ");
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) != 0) {
			;
		}

		image++;
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
		result = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
		if (result != FR_OK) { // debug information, can be deleted in case of code size limitation (Keil)
			switch (result) {
			case FR_NO_FILE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILE      ");
				image = 0x30;
			case FR_NO_PATH:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_PATH      ");
				image = 0x30;
			case FR_INVALID_NAME:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INVALID_NAME ");
				image = 0x30;
			case FR_DENIED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_DENIED       ");
				image = 0x30;
			case FR_NOT_READY:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_READY    ");
				image = 0x30;
			case FR_WRITE_PROTECTED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_WRITE_PROTECT");
				image = 0x30;
			case FR_RW_ERROR:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_RW_ERROR     ");
				image = 0x30;
			case FR_INCORRECT_DISK_CHANGE:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_INCORRECT_DSK");
				image = 0x30;
			case FR_NOT_ENABLED:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NOT_ENABLED  ");
				image = 0x30;
			case FR_NO_FILESYSTEM:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("FR_NO_FILESYSTEM");
				image = 0x30;
			default:
				UILCD_SetCursor(0, 0);
				UILCD_DisplayString("NOT OPEN        ");
				image = 0x30;
			}
		}
		filename[13] = '\0';
		filenumber[0] = image;
		strncat(filename,filenumber,1);
		strncat(filename,filetype,4);
	}
	return 1;
}

/*******************************************************************************
 * Function Name  : UILCD_Init
 * Description    : Initializes LCD.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TFT_57F_Init(void) {
	GPIO_ResetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nRD);
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_WriteBit(GPIOC, RES, Bit_RESET);
	TFT_delay(5);
	GPIO_WriteBit(GPIOC, RES, Bit_SET);
	TFT_delay(100);
	TFT_57F_Write_Command(0x01); //Software Reset
	TFT_57F_Write_Command(0x01);
	TFT_57F_Write_Command(0x01);
	TFT_delay(10);
	TFT_57F_Command_Write(0xe0, 0x01); //START PLL
	TFT_57F_Command_Write(0xe0, 0x03); //LOCK PLL
	TFT_57F_Write_Command(0xb0); //SET LCD MODE  SET TFT 18Bits MODE
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x0c); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_57F_Write_Data(0x80); //SET TFT MODE & hsync+Vsync+DEN MODE
	TFT_57F_Write_Data(0x02); //SET horizontal size=640-1 HightByte
	TFT_57F_Write_Data(0x7f); //SET horizontal size=640-1 LowByte
	TFT_57F_Write_Data(0x01); //SET vertical size=480-1 HightByte
	TFT_57F_Write_Data(0xdf); //SET vertical size=480-1 LowByte
	TFT_57F_Write_Data(0x00); //SET even/odd line RGB seq.=RGB
	TFT_57F_Command_Write(0xf0, 0x00); //SET pixel data I/F format=8bit
	TFT_57F_Command_Write(0x3a, 0x60); // SET R G B format = 6 6 6
	TFT_57F_Write_Command(0xe6); //SET PCLK freq=4.94MHz  ; pixel clock frequency
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x02);
	TFT_57F_Write_Data(0xff);
	TFT_57F_Write_Data(0xff);
	TFT_57F_Write_Command(0xb4); //SET HBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x02); //SET HSYNC Total=760
	TFT_57F_Write_Data(0xf8);
	TFT_57F_Write_Data(0x00); //SET HBP 68
	TFT_57F_Write_Data(0x44);
	TFT_57F_Write_Data(0x0f); //SET VBP 16=15+1
	TFT_57F_Write_Data(0x00); //SET Hsync pulse start position
	TFT_57F_Write_Data(0x00);
	TFT_57F_Write_Data(0x00); //SET Hsync pulse subpixel start position
	TFT_57F_Write_Command(0xb6); //SET VBP,
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x01); //SET Vsync total
	TFT_57F_Write_Data(0xf8);
	TFT_57F_Write_Data(0x00); //SET VBP=19
	TFT_57F_Write_Data(0x13);
	TFT_57F_Write_Data(0x07); //SET Vsync pulse 8=7+1
	TFT_57F_Write_Data(0x00); //SET Vsync pulse start position
	TFT_57F_Write_Data(0x00);
	TFT_57F_Write_Command(0x2a); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x00); //SET start column address=0
	TFT_57F_Write_Data(0x00);
	TFT_57F_Write_Data(0x02); //SET end column address=639
	TFT_57F_Write_Data(0x7f);
	TFT_57F_Write_Command(0x2b); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(0x00); //SET start page address=0
	TFT_57F_Write_Data(0x00);
	TFT_57F_Write_Data(0x01); //SET end page address=479
	TFT_57F_Write_Data(0xdf);
	TFT_57F_Write_Command(0x29); //SET display on
}

void TFT_57F_Write_Command(unsigned char command) {
	//GPIO_SetBits(GPIOC, nRD);
	GPIO_Write(GPIOB, command);
	GPIO_ResetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	//GPIO_ResetBits(GPIOC, CS1);
	//GPIO_SetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nWR);
	//GPIO_WriteBit(GPIOC, nRD, Bit_SET);
	//if((command & 0x01)==0){GPIO_WriteBit(GPIOB, DB0, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB0, Bit_SET);}
	//if((command & 0x02)==0){GPIO_WriteBit(GPIOB, DB1, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB1, Bit_SET);}
	//if((command & 0x04)==0){GPIO_WriteBit(GPIOB, DB2, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB2, Bit_SET);}
	//if((command & 0x08)==0){GPIO_WriteBit(GPIOB, DB3, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB3, Bit_SET);}
	//if((command & 0x10)==0){GPIO_WriteBit(GPIOB, DB4, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB4, Bit_SET);}
	//if((command & 0x20)==0){GPIO_WriteBit(GPIOB, DB5, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB5, Bit_SET);}
	//if((command & 0x40)==0){GPIO_WriteBit(GPIOB, DB6, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB6, Bit_SET);}
	//if((command & 0x80)==0){GPIO_WriteBit(GPIOB, DB7, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB7, Bit_SET);}
	//GPIO_WriteBit(GPIOC, RS, Bit_RESET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_SET);
}

void TFT_57F_Write_Data(unsigned char data1) {
	//GPIO_WriteBit(GPIOC, nRD, Bit_SET);
	//if((data1 & 0x01)==0){GPIO_WriteBit(GPIOB, DB0, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB0, Bit_SET);}
	//if((data1 & 0x02)==0){GPIO_WriteBit(GPIOB, DB1, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB1, Bit_SET);}
	//if((data1 & 0x04)==0){GPIO_WriteBit(GPIOB, DB2, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB2, Bit_SET);}
	//if((data1 & 0x08)==0){GPIO_WriteBit(GPIOB, DB3, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB3, Bit_SET);}
	//if((data1 & 0x10)==0){GPIO_WriteBit(GPIOB, DB4, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB4, Bit_SET);}
	//if((data1 & 0x20)==0){GPIO_WriteBit(GPIOB, DB5, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB5, Bit_SET);}
	//if((data1 & 0x40)==0){GPIO_WriteBit(GPIOB, DB6, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB6, Bit_SET);}
	//if((data1 & 0x80)==0){GPIO_WriteBit(GPIOB, DB7, Bit_RESET);}else{GPIO_WriteBit(GPIOB, DB7, Bit_SET);}
	//GPIO_WriteBit(GPIOC, RS, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_RESET);
	//GPIO_WriteBit(GPIOC, CS1, Bit_SET);
	//GPIO_WriteBit(GPIOC, nWR, Bit_SET);
	GPIO_Write(GPIOB, data1);
	//GPIO_SetBits(GPIOC, nRD);
	//GPIO_SetBits(GPIOC, RS);
	GPIO_ResetBits(GPIOC, nWR);
	//GPIO_ResetBits(GPIOC, CS1);
	//GPIO_SetBits(GPIOC, CS1);
	GPIO_SetBits(GPIOC, nWR);
}

void TFT_57F_Command_Write(unsigned char REG, unsigned char VALUE) {
	TFT_57F_Write_Command(REG);
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data(VALUE);
}

void TFT_57F_SendData(unsigned long color) {
	GPIO_SetBits(GPIOC, RS);
	GPIO_Write(GPIOB, (color >> 16));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color >> 8));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	GPIO_Write(GPIOB, (color));
	GPIO_ResetBits(GPIOC, nWR);
	GPIO_SetBits(GPIOC, nWR);
	//TFT_57F_Write_Data((color)>>16);	//red
	//TFT_57F_Write_Data((color)>>8);		//green
	//TFT_57F_Write_Data(color);			//blue
}

void TFT_57F_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y,
		unsigned int e_y) {
	TFT_57F_Write_Command(0x2a); //SET page address
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data((s_x) >> 8); //SET start page address=0
	TFT_57F_Write_Data(s_x);
	TFT_57F_Write_Data((e_x) >> 8); //SET end page address=639
	TFT_57F_Write_Data(e_x);

	TFT_57F_Write_Command(0x2b); //SET column address
	GPIO_SetBits(GPIOC, RS);
	TFT_57F_Write_Data((s_y) >> 8); //SET start column address=0
	TFT_57F_Write_Data(s_y);
	TFT_57F_Write_Data((e_y) >> 8); //SET end column address=479
	TFT_57F_Write_Data(e_y);
}

void TFT_57F_FULL_ON(unsigned long dat) {
	unsigned long x;
	TFT_57F_WindowSet(0x0000, 0x027f, 0x0000, 0x01df);
	TFT_57F_Write_Command(0x2c);
	for (x = 0; x < 307200; x++) {
		//TFT_57F_SendData(dat);
		GPIO_SetBits(GPIOC, RS);
		GPIO_Write(GPIOB, (dat >> 16));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat >> 8));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		GPIO_Write(GPIOB, (dat));
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
	}
}

void TFT_57F_QUADS(void) {
	unsigned int i, j;
	TFT_57F_WindowSet(0x0000, 0x027f, 0x0000, 0x01df);
	TFT_57F_Write_Command(0x2c);
	for (j = 0; j < 240; j++) {
		for (i = 0; i < 320; i++) {
			TFT_57F_SendData(0x0000FF); //blue
		}
		for (i = 0; i < 320; i++) {
			TFT_57F_SendData(0xFF0000); //red
		}
	}
	for (j = 0; j < 240; j++) {
		for (i = 0; i < 320; i++) {
			TFT_57F_SendData(0xFFFF00); //yellow
		}
		for (i = 0; i < 320; i++) {
			TFT_57F_SendData(0x00FF00); //green
		}
	}
}
/*-------------------------------------------------------*/

