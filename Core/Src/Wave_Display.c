/*
 * Wave_Display.c
 *
 *  Created on: Oct 22, 2026
 *      Author: Sydney
 */

#include "main.h"
#include "app_freertos.h"
#include "Wave_Display.h"
#include "stm32u0xx_11_gpio.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

/*Config display pixels for top oriented display*/
#ifdef WAVED_TOP_VIEW
  #define SEG_CMD     0xA0
  #define COM_CMD     0xC8
  #define COL_OFFSET  30u
#else
  #define SEG_CMD     0xA1
  #define COM_CMD     0xC0
  #define COL_OFFSET  0u
#endif

#define WaveD_Delay() do{} while(0)

/*Pin Access (LL, direct BSRR Writing*/
static inline void WD_MOSI(int v){ v ? LL_GPIO_SetOutputPin(DOGS_MOSI_GPIO_Port, DOGS_MOSI_Pin): LL_GPIO_ResetOutputPin(DOGS_MOSI_GPIO_Port, DOGS_MOSI_Pin); }
static inline void WD_CLK(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CLK_GPIO_Port, DOGS_CLK_Pin): LL_GPIO_ResetOutputPin(DOGS_CLK_GPIO_Port, DOGS_CLK_Pin); }
static inline void WD_CD(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CD_GPIO_Port, DOGS_CD_Pin): LL_GPIO_ResetOutputPin(DOGS_CD_GPIO_Port, DOGS_CD_Pin); }
static inline void WD_RST(int v){ v ? LL_GPIO_SetOutputPin(DOGS_RST_GPIO_Port, DOGS_RST_Pin): LL_GPIO_ResetOutputPin(DOGS_RST_GPIO_Port, DOGS_RST_Pin); }
static inline void WD_CS(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CS_GPIO_Port, DOGS_CS_Pin): LL_GPIO_ResetOutputPin(DOGS_CS_GPIO_Port, DOGS_CS_Pin); }

/*State*/
static uint8_t fb[WaveD_Paged][WaveD_W]; /*8 pages x 102 columns*/
static SemaphoreHandle_t fb_mutex;
static TaskHandle_t Wave_Task_Handle;

/*Low-level SPI(mode 3, MSB First, Write-only)*/
static void Spi_Byte(uint8_t b)
{
	for (int i=7; i>=0; i--){
		WD_CLK(0);
		WD_MOSI((b>>i)&1);
		WaveD_Delay();
		WD_CLK(1); /*Data will sample on rising edge*/
		WaveD_Delay();
	}
}

static void Write_CMD(uint8_t c)
{
	WD_CS(0);
	WD_CD(0); /*0 is command*/
	Spi_Byte(c);
	WD_CS(1);
}

/*Init: datasheet sequence*/
static void Display_Reset_And_Init(void)
{
	WD_CLK(1); /*Idle Levels*/
	WD_CS(1);

	WD_RST(0);
	vTaskDelay(pdMS_TO_TICKS(10));
	WD_RST(1);
	vTaskDelay(pdMS_TO_TICKS(10));

	write_cmd(0x40);
	write_cmd(SEG_CMD);
	write_cmd(COM_CMD);
	write_cmd(0xA4);
	write_cmd(0xA6);
	write_cmd(0xA2);
	write_cmd(0x2F);
	write_cmd(0x27);
	write_cmd(0x81);
	write_cmd(0x10);
	write_cmd(0xFA);
	write_cmd(0x90);
	write_cmd(0xAF);
}

/*Send frame buffer*/
static void Display_Send_Frame(void)
{
	xSemaphoreTake(fb_mutex, portMAX_DELAY);
	for (uint8_t page=0; page<WaveD_Pages; page++)
	{
		write_cmd(0xB0 | page); /*Page Address*/
		write_cmd(0x10 | ((COLD_OFFSET>>4) & 0x0F)); /*Column MSB*/
		write_cmd(0x00 | (COL_OFFSET &0x0F)); /*Column LSB*/

		WD_CS(0);
		WD_CD(1);

		for (uint8_t x=0; x<WaveD_W; x++)
		{
			Spi_Byte(fb[page][x]);
		}
		WD_CS(1);
	}
	xSemaphoreGive(fb_mutex);
}

/*Public API*/
void WaveD_Init(void)
{
	memset(fb, 0, sizeof fb)
	fb_mutex = xSemaphoreCreateMutex();
}

void WaveD_Task(void)
{
	Wave_Task_Handle = xTaskGetCurrentTaskHandle();
	Display_Reset_And_Init();
	Display_Send_Frame();

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DElAY);
		Display_Send_Frame();
	}
}

void WaveD_Clear(void)
{
	xSemaphoreTake(fb_mutex, portMAX_DELAY);
	memset(fb,0,sizeof fb);
	xSemaphoreGive(fb_mutex);
}

void WaveD_Set_Pixel(uint8_t x,uint8_t y, bool on)
{
	if (x>= WaveD_W || y>=WaveD_H) return;
	xSemaphoreTake(fb_mutex, portMAX_DELAY);
	if (on) fb[y>>3][x] |= (uint8_t)(1u<<(y & 7u));
	else fb[y>>3][x] &= (uint8_t)~(1u<<(y & 7u));
	xSemaphoreGive(fb_mutex);
}

void WaveD_Flush(void)
{
	if (Wave_Task_Handle) xTaskNotifyGive(Wave_Task_Handle); /*Ignore until task starts*/
}
