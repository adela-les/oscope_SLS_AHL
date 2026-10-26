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
#ifdef WaveD_Top_View
#define Seg_CMD


