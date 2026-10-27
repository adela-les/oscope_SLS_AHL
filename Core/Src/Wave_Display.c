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
