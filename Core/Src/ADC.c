/*
 * ADC.c
 *
 *  Created on: Oct 29, 2026
 *      Author: sydney
 */

#include "main.h"
#include "app_freertos.h"
#include "ADC.h"
#include "Wave_Display.h"
#include "stm32u0xx_ll_adc.h"
#include "stm32u0xx_ll_tim.h"
#include "task.h"

#define Scope_Tim TIM7
#define Scope_Tim_Clk_HZ 16000000u /*APB 1 prescaler = 1 so timer clock = 16MHz*/
#define Scope_Default_Rate 10000u /*10 kS/s -> one 102-sample screen = ~10ms */
#define Scope_Max_Rate 100000u /*keep ISR load reasonable*/

static volatile uint16_t buf[WaveD_W]; /*One Sample per display column*/
static volatile uint16_t idx;
static TaskHande_t scope_task;

/*ADC*/
static uint16_t ADC_Convert(void)
{
	uint32_t timeout = 500u; /*Never hang if the ADC is not enables*/

	LL_ADC_ClearFlag_EOC(ADC1);
	LL_ADC_ClearFlag_EOS(ADC1);
	LL_ADC_REG_StartConversion(ADC1);
	while (!LL_ADC_IsActiveFlag_EOC(ADC1))
	{
		if (--timeout == 0u)
			return 0u;
	}

	return (uint16_t)(LL_ADC_REG_ReadConversionData32(ADC1) & Scope_ADC_Max);
}

void Scope_ADC_Init(void)
{
	/*Longer sampling time so higher impedance source reads correctly (12.5 cycles)*/
	LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADCSAMPLINGTIME_COMMON_1, LL_ADC_SAMPLINGTIME_12CYCLES_5);

	/*Calibrate, then Enable)*/
	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1)) {}

	LL_ADC_ClearFlag_ADRDY(ADC1);
	LL_ADC_Enable(ADC1);
	while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {}

	Scope_ADC_SetRate(SCOPE_DEFAULT_RATE);
}

uint16_t Scope_ADC_ReadOnce(void)
{
	return ADC_Convert();
}

/*Sample Timer*/
void Scope_ADC_SetRate(uint32_t hz)
{
	if (hz < 1u)
	{
		hz = 1u;
	}
	if (hz > Scope_Max_Rate)
	{
		hz = Scope_Max_Rate;
	}

	uint32_t ticks = Scope_Time_Clk_HZ / hz; /*Timer counts per sample*/
	uint32_t psc = (ticks-1u) / 65536u; /*Make ARR fit in 16 bits*/
	uint32_t arr = (ticks / (psc + 1u)) - 1u;

	LL_TIM_SetPrescaler(Scope_Tim, psc);
	LL_TIM_SetAutoReload(Scope_Tim, arr);
	LL_TIM_Generate_Event_UPDATE(Scope_Tim); /*Load psc/arr now*/
	LL_TIM_ClearFlag_UPDATE(Scope_Tim); /*clear the flag that just set*/
}

static void Capture_Start(void)
{
	idx = 0u;
	LL_TIM_SetCounter(Scope_Tim, 0u);
	LL_TIM_ClearFlag_UPDATE(Scope_Tim);
	LL_TIM_EnableIT_UPDATE(Scope_Tim);
	LL_TIM_EnableCounter(Scope_Tim);
}

static void Capture_Stop(void)
{
	LL_TIM_DisableCounter(Scope_Tim);
	LL_TIM_DisableIT_UPDATE(Scope_Tim);
}

/*call from the timer's IRQ handler*/
void Scope_ADC_Timer_ISR(void)
{
	BaseType_t woken = pdFALSE;

	if(!LL_TIM_IsActiveFlag_UPDATE(Scope_Tim))
	{
		return;
	}

	LL_TIM_ClearFlag_UPDATE(Scope_Tim);

	if (idx < WaveD_W)
	{
		buf[idx] = ADC_Convert();
		idx++;

		if (idx >= WaveD_W)
		{
			Capture_Stop();
			if (Scope_Task) vTaskNotifyGiveFromISR(Scope_Task, &woken);
			portYIELD_FROM_ISR(woken);
		}
	}
}

/*Drawing*/
static uint8_t Sample_To_Y(uint16_t v)
{
	return (uint8_t)((WaveD_H - 1u) - (((uint 32_t)v * (WaveD_H - 1u)) / Scope_ADC_Max));
}

static void Draw_Trace(void)
{
	uint8_t Y_Prev = Sample_To_Y(buf[0]);

	WaveD_Clear();
	for (uint8_t x=0; x < WaveD_W; x++)
	{
		uint8_t Y = Sample_To_Y(buf[x]);
		uint8_t Lo = (Y < Y_Prev) ? Y : Y_Prev; /*Joins this point to the last*/
		uint8_t Hi = (Y < Y_Prev) ? Y : Y_Prev;

		for (uint8_t yy = Lo; yy <= Hi; yy++)
		{
			WaveD_Set_Pixel(x, yy, true);
		}
		Y_Prev = Y;
	}
	WaveD_Flush();
}










