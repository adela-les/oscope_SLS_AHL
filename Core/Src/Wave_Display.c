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

static inline void WD_MOSI(int v){ v ? LL_GPIO_SetOutputPin(DOGS_MOSI_GPIO_Port, DOGS_MOSI_Pin): LL_GPIO_ResetOutputPin(DOGS_MOSI_GPIO_Port, DOGS_MOSI_Pin); }
static inline void WD_CLK(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CLK_GPIO_Port, DOGS_CLK_Pin): LL_GPIO_ResetOutputPin(DOGS_CLK_GPIO_Port, DOGS_CLK_Pin); }
static inline void WD_CD(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CD_GPIO_Port, DOGS_CD_Pin): LL_GPIO_ResetOutputPin(DOGS_CD_GPIO_Port, DOGS_CD_Pin); }
static inline void WD_RST(int v){ v ? LL_GPIO_SetOutputPin(DOGS_RST_GPIO_Port, DOGS_RST_Pin): LL_GPIO_ResetOutputPin(DOGS_RST_GPIO_Port, DOGS_RST_Pin); }
static inline void WD_CS(int v){ v ? LL_GPIO_SetOutputPin(DOGS_CS_GPIO_Port, DOGS_CS_Pin): LL_GPIO_ResetOutputPin(DOGS_CS_GPIO_Port, DOGS_CS_Pin); }


