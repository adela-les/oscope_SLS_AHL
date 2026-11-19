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
#include "stm32u0xx_ll_spi.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stddef.h>

/*Display orientation: 1 = top view, 0 = bottom view.
 *Replaces the old #ifdef WAVED_TOP_VIEW. If you used to define WAVED_TOP_VIEW
 *(project settings or another header), set this to 1 to keep the same behavior.*/
#define WAVED_TOP_VIEW_ENABLED 0

#define WD_CD_PORT GPIOA
#define WD_CD_PIN LL_GPIO_PIN_4
#define WD_RST_PORT GPIOA
#define WD_RST_PIN LL_GPIO_PIN_5
#define WD_CS_PORT GPIOA
#define WD_CS_PIN LL_GPIO_PIN_8

#define SPI_WAIT_LIMIT 5000u /*max polls per wait. One byte is about 4 us, so this is far more than enough*/

/*Pin Access (LL, direct BSRR Writing)*/
static inline void WD_CD(bool v)
{
	if (v) { LL_GPIO_SetOutputPin(WD_CD_PORT, WD_CD_PIN); }
	else   { LL_GPIO_ResetOutputPin(WD_CD_PORT, WD_CD_PIN); }
}

static inline void WD_RST(bool v)
{
	if (v) { LL_GPIO_SetOutputPin(WD_RST_PORT, WD_RST_PIN); }
	else   { LL_GPIO_ResetOutputPin(WD_RST_PORT, WD_RST_PIN); }
}

static inline void WD_CS(bool v)
{
	if (v) { LL_GPIO_SetOutputPin(WD_CS_PORT, WD_CS_PIN); }
	else   { LL_GPIO_ResetOutputPin(WD_CS_PORT, WD_CS_PIN); }
}

/*State*/
static uint8_t fb[WaveD_PAGES][WaveD_W]; /*8 pages x 102 columns*/
static SemaphoreHandle_t fb_mutex = NULL; /*stays NULL until WaveD_Init succeeds*/
static TaskHandle_t Wave_Task_Handle = NULL;

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

/*Low-level SPI (mode 3, MSB First, Write-only). Every wait is bounded; false means the SPI hung*/
static bool Spi_Byte(uint8_t b)
{
	uint32_t n = SPI_WAIT_LIMIT;
	while (!LL_SPI_IsActiveFlag_TXE(SPI1))
	{
		n--;
		if (n == 0u) { return false; }
	}

	LL_SPI_TransmitData8(SPI1, b);

	n = SPI_WAIT_LIMIT;
	while (LL_SPI_GetTxFIFOLevel(SPI1) != LL_SPI_TX_FIFO_EMPTY)
	{
		n--;
		if (n == 0u) { return false; }
	}

	n = SPI_WAIT_LIMIT;
	while (LL_SPI_IsActiveFlag_BSY(SPI1))
	{
		n--;
		if (n == 0u) { return false; }
	}
	return true;
}

/*CS is always released, even when the transfer fails*/
static bool Write_CMD(uint8_t c)
{
	WD_CS(false);
	WD_CD(false); /*0 is command*/
	bool ok = Spi_Byte(c);
	WD_CS(true);
	return ok;
}

/*Init: datasheet sequence. False if any command failed*/
static bool Display_Reset_And_Init(void)
{
	/*Orientation values (were the SEG_CMD / COM_CMD macros inside #ifdef)*/
	const uint8_t seg_cmd = WAVED_TOP_VIEW_ENABLED ? 0xA0u : 0xA1u;
	const uint8_t com_cmd = WAVED_TOP_VIEW_ENABLED ? 0xC8u : 0xC0u;

	const uint8_t init_cmds[] = {
		0x40u, seg_cmd, com_cmd, 0xA4u, 0xA6u, 0xA2u, 0x2Fu,
		0x27u, 0x81u, 0x10u, 0xFAu, 0x90u, 0xAFu
	};

	WD_CS(true); /*Idle Levels*/

	WD_RST(false);
	vTaskDelay(pdMS_TO_TICKS(10));
	WD_RST(true);
	vTaskDelay(pdMS_TO_TICKS(10));

	for (size_t i = 0u; i < (sizeof init_cmds / sizeof init_cmds[0]); i++)
	{
		if (!Write_CMD(init_cmds[i]))
		{
			return false;
		}
	}
	return true;
}

/*Send frame buffer. False if the mutex is missing/failed or any SPI transfer hung*/
static bool Display_Send_Frame(void)
{
	const uint8_t col_offset = WAVED_TOP_VIEW_ENABLED ? 30u : 0u;

	if (fb_mutex == NULL)
	{
		return false;
	}
	if (xSemaphoreTake(fb_mutex, portMAX_DELAY) != pdTRUE)
	{
		return false;
	}

	bool ok = true;
	for (uint8_t page = 0u; (page < WaveD_PAGES) && ok; page++)
	{
		ok = Write_CMD((uint8_t)(0xB0u | page));                           /*Page Address*/
		if (ok) { ok = Write_CMD((uint8_t)(0x10u | ((col_offset >> 4) & 0x0Fu))); } /*Column MSB*/
		if (ok) { ok = Write_CMD((uint8_t)(col_offset & 0x0Fu)); }                  /*Column LSB*/

		if (ok)
		{
			WD_CS(false);
			WD_CD(true);
			for (uint8_t x = 0u; (x < WaveD_W) && ok; x++)
			{
				ok = Spi_Byte(fb[page][x]);
			}
			WD_CS(true); /*released even if a byte failed*/
		}
	}

	if (xSemaphoreGive(fb_mutex) != pdTRUE)
	{
		ok = false;
	}
	return ok;
}

/*Public API*/
bool WaveD_Init(void)
{
	/*CS is PA8: make sure it is a driven output, idle high*/
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_GPIO_SetOutputPin(WD_CS_PORT, WD_CS_PIN);
	LL_GPIO_SetPinMode(WD_CS_PORT, WD_CS_PIN, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType(WD_CS_PORT, WD_CS_PIN, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinSpeed(WD_CS_PORT, WD_CS_PIN, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinPull(WD_CS_PORT, WD_CS_PIN, LL_GPIO_PULL_NO);

	WD_SPI_Start();
	(void)memset(fb, 0, sizeof fb);

	fb_mutex = xSemaphoreCreateMutex();
	return (fb_mutex != NULL);
}

void WaveD_Task_Body(void)
{
	Wave_Task_Handle = xTaskGetCurrentTaskHandle();
	if (Wave_Task_Handle == NULL)
	{
		return; /* not running inside a task: nothing safe to do */
	}

	bool ok = Display_Reset_And_Init();
	if (ok)
	{
		ok = Display_Send_Frame();
	}

	/* Rule 2 exception: intentionally non-terminating RTOS task loop.
	 * It blocks on a notification, so it never spins. */
	for (;;)
	{
		if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0u)
		{
			continue;
		}
		if (!ok)
		{
			ok = Display_Reset_And_Init(); /* retry the init after an earlier failure */
		}
		if (ok)
		{
			ok = Display_Send_Frame();
		}
	}
}

bool WaveD_Clear(void)
{
	if (fb_mutex == NULL)
	{
		return false;
	}
	if (xSemaphoreTake(fb_mutex, portMAX_DELAY) != pdTRUE)
	{
		return false;
	}
	(void)memset(fb, 0, sizeof fb);
	return (xSemaphoreGive(fb_mutex) == pdTRUE);
}

bool WaveD_Set_Pixel(uint8_t x, uint8_t y, bool on)
{
	if ((x >= WaveD_W) || (y >= WaveD_H))
	{
		return false;
	}
	if (fb_mutex == NULL)
	{
		return false;
	}
	if (xSemaphoreTake(fb_mutex, portMAX_DELAY) != pdTRUE)
	{
		return false;
	}

	if (on)
	{
		fb[y >> 3][x] |= (uint8_t)(1u << (y & 7u));
	}
	else
	{
		fb[y >> 3][x] &= (uint8_t)~(1u << (y & 7u));
	}

	return (xSemaphoreGive(fb_mutex) == pdTRUE);
}

bool WaveD_Flush(void)
{
	if (Wave_Task_Handle == NULL)
	{
		return false; /*display task has not started yet*/
	}
	(void)xTaskNotifyGive(Wave_Task_Handle); /*macro always returns pdPASS*/
	return true;
}
