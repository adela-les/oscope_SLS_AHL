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
	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1)) {}

	LL_ADC_ClearFlag_ADRDY(ADC1);
	LL_ADC_Enable(ADC1);
	while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {}

	Scope_ADC_SetRate(SCOPE_DEFAULT_RATE);
}

