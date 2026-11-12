/*
 * ADC.c
 *
 *  Created on: Oct 29, 2026
 *      Author: sydney
 *
 *  Scope controls (set by the encoder/button tasks):
 *    mode        0 = trigger on falling edge, 1 = trigger on rising edge
 *    start_point trigger level, 0..30 = 0.0 V .. 3.0 V (tenths of a volt)
 *    current     screen height in volts, 1..30 = 0.1 V .. 3.0 V tall (tenths of a volt)
 *    time        time across the screen, in units of 0.1 ms (310 = 31.0 ms)
 */

#include "main.h"
#include "app_freertos.h"
#include "ADC.h"
#include "Wave_Display.h"
#include "stm32u0xx_ll_adc.h"
#include "stm32u0xx_ll_tim.h"
#include "task.h"

/*Controls defined in the encoder / button files*/
extern volatile uint16_t current;
extern volatile uint16_t time;
extern volatile uint8_t  mode;
extern volatile uint16_t start_point;

#define Scope_Tim TIM7
#define SCOPE_TIM_CLK_HZ 16000000u /*APB 1 prescaler = 1 so timer clock = 16MHz*/
#define SCOPE_DEFAULT_RATE 10000u /*10 kS/s until the first capture sets the real rate*/
#define Scope_Max_Rate 100000u /*keep ISR load reasonable*/

#define SCOPE_VREF_TENTHS 33u /*3.3 V reference, in tenths of a volt*/
#define SCOPE_DIVS 1u /*1 = "time" is the whole screen width. Set to 10 if "time" means per division of 10*/

#define SCOPE_CAP_LEN (2u * WaveD_W) /*capture two screens so a trigger can be found in the first one*/

static volatile uint16_t buf[SCOPE_CAP_LEN];
static volatile uint16_t idx;
static TaskHandle_t Scope_Task;
volatile uint8_t Scope_ADC_Ready = 0;

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
	LL_ADC_InitTypeDef     ADC_InitStruct     = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
	LL_GPIO_InitTypeDef    GPIO_InitStruct    = {0};
	uint32_t t;

	Scope_ADC_Ready = 0;
	/*Clocks and the analog input pin (PA0 = ADC1_IN4)*/
	LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_SYSCLK);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

	GPIO_InitStruct.Pin  = LL_GPIO_PIN_0;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*ADC settings (same as CubeMX: 10-bit, right aligned, software trigger, single conversion)*/
	ADC_InitStruct.Clock         = LL_ADC_CLOCK_SYNC_PCLK_DIV1;
	ADC_InitStruct.Resolution    = LL_ADC_RESOLUTION_10B;
	ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADC_InitStruct.LowPowerMode  = LL_ADC_LP_MODE_NONE;
	LL_ADC_Init(ADC1, &ADC_InitStruct);

	LL_ADC_REG_SetSequencerConfigurable(ADC1, LL_ADC_REG_SEQ_FIXED);

	ADC_REG_InitStruct.TriggerSource    = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength  = LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_1RANK;
	ADC_REG_InitStruct.ContinuousMode   = LL_ADC_REG_CONV_SINGLE;
	ADC_REG_InitStruct.DMATransfer      = LL_ADC_REG_DMA_TRANSFER_NONE;
	ADC_REG_InitStruct.Overrun          = LL_ADC_REG_OVR_DATA_PRESERVED;
	LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

	LL_ADC_REG_SetSequencerScanDirection(ADC1, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);
	LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);
	LL_ADC_SetTriggerFrequencyMode(ADC1, LL_ADC_TRIGGER_FREQ_HIGH);
	LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_4);
	/*Longer sampling time so a pot / higher impedance source reads correctly (12.5 cycles)*/
	LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_COMMON_1, LL_ADC_SAMPLINGTIME_12CYCLES_5);
	LL_ADC_DisableIT_EOC(ADC1);
	LL_ADC_DisableIT_EOS(ADC1);
	/*Voltage regulator on, then give it time to settle (about 1 ms here, plenty)*/
	LL_ADC_EnableInternalRegulator(ADC1);
	for (volatile uint32_t i = 0; i < 20000u; i++) {}
	/*Calibrate (limited wait)*/
	LL_ADC_StartCalibration(ADC1);
	t = 1000000u;
	while (LL_ADC_IsCalibrationOnGoing(ADC1) && (--t != 0u)) {}
	if (t == 0u) return;                          /* calibration never finished: Scope_ADC_Ready stays 0 */
	/*Enable (limited wait)*/
	LL_ADC_ClearFlag_ADRDY(ADC1);
	LL_ADC_Enable(ADC1);
	t = 1000000u;
	while (!LL_ADC_IsActiveFlag_ADRDY(ADC1) && (--t != 0u)) {}
	if (t == 0u) return;                          /* ADC never became ready: Scope_ADC_Ready stays 0 */
	Scope_ADC_Ready = 1;
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

	uint32_t ticks = SCOPE_TIM_CLK_HZ / hz; /*Timer counts per sample*/
	uint32_t psc = (ticks-1u) / 65536u; /*Make ARR fit in 16 bits*/
	uint32_t arr = (ticks / (psc + 1u)) - 1u;

	LL_TIM_SetPrescaler(Scope_Tim, psc);
	LL_TIM_SetAutoReload(Scope_Tim, arr);
	LL_TIM_GenerateEvent_UPDATE(Scope_Tim); /*Load psc/arr now*/
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

	if (idx < SCOPE_CAP_LEN)
	{
		buf[idx] = ADC_Convert();
		idx++;

		if (idx >= SCOPE_CAP_LEN)
		{
			Capture_Stop();
			if (Scope_Task) vTaskNotifyGiveFromISR(Scope_Task, &woken);
			portYIELD_FROM_ISR(woken);
		}
	}
}

/*Convert tenths of a volt (0..30) to ADC counts (0..1023)*/
static uint32_t Tenths_To_Counts(uint16_t tenths)
{
	return ((uint32_t)tenths * Scope_ADC_Max) / SCOPE_VREF_TENTHS;
}

/*Find the first point where the signal crosses the trigger level in the chosen direction.
 *Returns the index to start drawing from, or 0 if no crossing was found (auto: draw from the start)*/
static uint16_t Find_Trigger(uint16_t level, uint8_t rising)
{
	for (uint16_t i = 1u; i <= WaveD_W; i++)
	{
		uint16_t a = buf[i - 1u];
		uint16_t b = buf[i];

		if (rising && (a < level) && (b >= level))
		{
			return i;
		}
		if (!rising && (a > level) && (b <= level))
		{
			return i;
		}
	}
	return 0u;
}

/*Drawing: full_counts is the ADC value that reaches the top of the screen*/
static uint8_t Sample_To_Y(uint16_t v, uint32_t full_counts)
{
	if ((uint32_t)v >= full_counts)
	{
		return 0u;
	}
	return (uint8_t)((WaveD_H - 1u) - (((uint32_t)v * (WaveD_H - 1u)) / full_counts));
}

static void Draw_Trace(uint16_t first, uint32_t full_counts)
{
	uint8_t Y_Prev = Sample_To_Y(buf[first], full_counts);

	WaveD_Clear();
	for (uint8_t x=0; x < WaveD_W; x++)
	{
		uint8_t Y = Sample_To_Y(buf[first + x], full_counts);
		uint8_t Lo = (Y < Y_Prev) ? Y : Y_Prev; /*Joins this point to the last*/
		uint8_t Hi = (Y < Y_Prev) ? Y_Prev : Y;

		for (uint8_t yy = Lo; yy <= Hi; yy++)
		{
			WaveD_Set_Pixel(x, yy, true);
		}
		Y_Prev = Y;
	}
	WaveD_Flush();
}

/*Scope Task*/
void Scope_Task_Body(void)
{
	Scope_Task = xTaskGetCurrentTaskHandle();
	osDelay(200); /*Allows display to finish init*/

	for (;;)
	{
		/*Read the controls once per frame*/
		uint16_t t      = (time < 1u) ? 1u : time;                 /*0.1 ms units, screen width*/
		uint16_t cur    = (current < 1u) ? 1u : current;           /*tenths of a volt, screen height*/
		uint16_t level  = (uint16_t)Tenths_To_Counts(start_point); /*trigger level in ADC counts*/
		uint8_t  rising = (mode != 0u);                            /*1 = rise, 0 = fall*/
		uint32_t full   = Tenths_To_Counts(cur);                   /*ADC counts for a full-height screen*/

		/*WaveD_W samples across t x 0.1 ms  ->  rate = WaveD_W * 10000 / (t * SCOPE_DIVS)*/
		Scope_ADC_SetRate(((uint32_t)WaveD_W * 10000u) / ((uint32_t)t * SCOPE_DIVS));

		Capture_Start();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*sleep until ISR says buffer is full*/

		Draw_Trace(Find_Trigger(level, rising), full);
		osDelay(50);
	}
}
