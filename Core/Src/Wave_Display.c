/*
 * Wave_Display.c
 *
 *  Created on: Oct 22, 2026
 *      Author: Sydney
 */

#include "main.h"
#include "app_freertos.h"
#include "Wave_Display.h"
#include "stm32u0xx_ll_gpio.h"
#include "stm32u0xx_ll_bus.h"
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

#define WD_CD_PORT GPIOA
#define WD_CD_PIN LL_GPIO_PIN_4
#define WD_RST_PORT GPIOA
#define WD_RST_PIN LL_GPIO_PIN_5
#define WD_CS_PORT GPIOA
#define WD_CS_PIN LL_GPIO_PIN_8

/*Pin Access (LL, direct BSRR Writing*/
static inline void WD_CD(int v){ v ? LL_GPIO_SetOutputPin(WD_CD_PORT, WD_CD_PIN): LL_GPIO_ResetOutputPin(WD_CD_PORT, WD_CD_PIN); }
static inline void WD_RST(int v){ v ? LL_GPIO_SetOutputPin(WD_RST_PORT, WD_RST_PIN): LL_GPIO_ResetOutputPin(WD_RST_PORT, WD_RST_PIN); }
static inline void WD_CS(int v){ v ? LL_GPIO_SetOutputPin(WD_CS_PORT, WD_CS_PIN): LL_GPIO_ResetOutputPin(WD_CS_PORT, WD_CS_PIN); }

/*State*/
static uint8_t fb[WaveD_PAGES][WaveD_W]; /*8 pages x 102 columns*/
static SemaphoreHandle_t fb_mutex;
static TaskHandle_t Wave_Task_Handle;

/*Reconfigure SPI1 for the display: write only, 8-bit, mode 3 (idle high, 2nd edge), 2 MHz*/
static void WD_SPI_Start(void)
{
	LL_SPI_Disable(SPI1);                                        /* settings can only change while off */
	LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);    /* write only */
	LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);            /* CubeMX has 4-bit right now */
	LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);         /* clock idles high */
	LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);              /* data taken on the 2nd edge */
	LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV8);
	LL_SPI_SetTransferBitOrder(SPI1, LL_SPI_MSB_FIRST);
	LL_SPI_DisableNSSPulseMgt(SPI1);                             /* CubeMX has this enabled */
	LL_SPI_Enable(SPI1);
}


/*Low-level SPI(mode 3, MSB First, Write-only)*/
static void Spi_Byte(uint8_t b)
{
	while (!LL_SPI_IsActiveFlag_TXE(SPI1)){}
	LL_SPI_TransmitData8(SPI1, b);
	while (LL_SPI_GetTxFIFOLevel(SPI1) != LL_SPI_TX_FIFO_EMPTY) {}
	while (LL_SPI_IsActiveFlag_BSY(SPI1)) {}
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
	WD_CS(1); /*Idle Levels*/

	WD_RST(0);
	vTaskDelay(pdMS_TO_TICKS(10));
	WD_RST(1);
	vTaskDelay(pdMS_TO_TICKS(10));

	Write_CMD(0x40);
	Write_CMD(SEG_CMD);
	Write_CMD(COM_CMD);
	Write_CMD(0xA4);
	Write_CMD(0xA6);
	Write_CMD(0xA2);
	Write_CMD(0x2F);
	Write_CMD(0x27);
	Write_CMD(0x81);
	Write_CMD(0x10);
	Write_CMD(0xFA);
	Write_CMD(0x90);
	Write_CMD(0xAF);
}

/*Send frame buffer*/
static void Display_Send_Frame(void)
{
	xSemaphoreTake(fb_mutex, portMAX_DELAY);
	for (uint8_t page=0; page<WaveD_PAGES; page++)
	{
		Write_CMD(0xB0 | page); /*Page Address*/
		Write_CMD(0x10 | ((COL_OFFSET>>4) & 0x0F)); /*Column MSB*/
		Write_CMD(0x00 | (COL_OFFSET &0x0F)); /*Column LSB*/

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
//void WaveD_Init(void)
//{
//	WD_SPI_Start();
//	memset(fb, 0, sizeof fb);
//	fb_mutex = xSemaphoreCreateMutex();
//}

void WaveD_Init(void)
{
	/*CS is PA8: make sure it is a driven output, idle high*/
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_GPIO_SetOutputPin(WD_CS_PORT, WD_CS_PIN);
	LL_GPIO_SetPinMode(WD_CS_PORT, WD_CS_PIN, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType(WD_CS_PORT, WD_CS_PIN, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinSpeed(WD_CS_PORT, WD_CS_PIN, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinPull(WD_CS_PORT, WD_CS_PIN, LL_GPIO_PULL_NO);

	WD_SPI_Start();
	memset(fb, 0, sizeof fb);
	fb_mutex = xSemaphoreCreateMutex();
}

void WaveD_Task_Body(void)
{
	Wave_Task_Handle = xTaskGetCurrentTaskHandle();
	Display_Reset_And_Init();
	Display_Send_Frame();

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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
