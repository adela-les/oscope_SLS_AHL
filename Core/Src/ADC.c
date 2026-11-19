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
#include "cmsis_os2.h"
#include "stm32u0xx_ll_adc.h"
#include "stm32u0xx_ll_tim.h"
#include "stm32u0xx_ll_gpio.h"
#include "stm32u0xx_ll_rcc.h"
#include "stm32u0xx_ll_bus.h"
#include "task.h"
#include <stddef.h>

/*Controls defined in the encoder / button files (owned by the encoder code, left as is)*/
extern volatile uint16_t current;
extern volatile uint16_t time;
extern volatile uint8_t  mode;
extern volatile uint16_t start_point;

#define Scope_Tim TIM7
#define SCOPE_TIM_CLK_HZ 16000000u /*APB 1 prescaler = 1 so timer clock = 16MHz*/
#define SCOPE_DEFAULT_RATE 10000u /*10 kS/s until the first capture sets the real rate*/
#define Scope_Max_Rate 100000u /*keep ISR load reasonable*/

#define SCOPE_VREF_TENTHS 33u /*3.3 V reference, in tenths of a volt*/
#define SCOPE_TENTHS_MAX 30u /*highest valid tenths value (3.0 V)*/
#define SCOPE_DIVS 1u /*1 = "time" is the whole screen width. Set to 10 if "time" means per division of 10*/

#define SCOPE_CAP_LEN (2u * WaveD_W) /*capture two screens so a trigger can be found in the first one*/

#define SCOPE_CONV_TIMEOUT 500u /*max polls waiting for one conversion*/
#define SCOPE_REG_SETTLE 20000u /*regulator settling delay, loop count*/
#define SCOPE_WAIT_LIMIT 1000000u /*max polls for calibration / ADRDY*/

static volatile uint16_t buf[SCOPE_CAP_LEN];
static volatile uint16_t idx;
static volatile bool capture_failed;   /* set by the ISR if any conversion timed out */
static volatile bool adc_ready = false; /* was Scope_ADC_Ready (non-static global) */
static TaskHandle_t Scope_Task;

/*ADC: one blocking conversion. Returns false on timeout so a failure is not mistaken for 0 V*/
static bool ADC_Convert(uint16_t *out)
{
	if (out == NULL)
	{
		return false;
	}

	uint32_t timeout = SCOPE_CONV_TIMEOUT; /*Never hang if the ADC is not enabled*/

	LL_ADC_ClearFlag_EOC(ADC1);
	LL_ADC_ClearFlag_EOS(ADC1);
	LL_ADC_REG_StartConversion(ADC1);
	while (!LL_ADC_IsActiveFlag_EOC(ADC1))
	{
		timeout--;
		if (timeout == 0u)
		{
			return false;
		}
	}

	*out = (uint16_t)(LL_ADC_REG_ReadConversionData32(ADC1) & Scope_ADC_Max);
	return true;
}

bool Scope_ADC_Init(void)
{
	LL_ADC_InitTypeDef     ADC_InitStruct     = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
	LL_GPIO_InitTypeDef    GPIO_InitStruct    = {0};

	adc_ready = false;

	/*Clocks and the analog input pin (PA0 = ADC1_IN4)*/
	LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_SYSCLK);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

	GPIO_InitStruct.Pin  = LL_GPIO_PIN_0;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	if (LL_GPIO_Init(GPIOA, &GPIO_InitStruct) != SUCCESS)
	{
		return false;
	}

	/*ADC settings (same as CubeMX: 10-bit, right aligned, software trigger, single conversion)*/
	ADC_InitStruct.Clock         = LL_ADC_CLOCK_SYNC_PCLK_DIV1;
	ADC_InitStruct.Resolution    = LL_ADC_RESOLUTION_10B;
	ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADC_InitStruct.LowPowerMode  = LL_ADC_LP_MODE_NONE;
	if (LL_ADC_Init(ADC1, &ADC_InitStruct) != SUCCESS)
	{
		return false;
	}

	LL_ADC_REG_SetSequencerConfigurable(ADC1, LL_ADC_REG_SEQ_FIXED);

	ADC_REG_InitStruct.TriggerSource    = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength  = LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_1RANK;
	ADC_REG_InitStruct.ContinuousMode   = LL_ADC_REG_CONV_SINGLE;
	ADC_REG_InitStruct.DMATransfer      = LL_ADC_REG_DMA_TRANSFER_NONE;
	ADC_REG_InitStruct.Overrun          = LL_ADC_REG_OVR_DATA_PRESERVED;
	if (LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct) != SUCCESS)
	{
		return false;
	}

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
	for (volatile uint32_t i = 0u; i < SCOPE_REG_SETTLE; i++) {}

	/*Calibrate (limited wait)*/
	LL_ADC_StartCalibration(ADC1);
	uint32_t wait = SCOPE_WAIT_LIMIT;
	while (LL_ADC_IsCalibrationOnGoing(ADC1) && (--wait != 0u)) {}
	if (wait == 0u)
	{
		return false; /* calibration never finished */
	}

	/*Enable (limited wait)*/
	LL_ADC_ClearFlag_ADRDY(ADC1);
	LL_ADC_Enable(ADC1);
	wait = SCOPE_WAIT_LIMIT;
	while (!LL_ADC_IsActiveFlag_ADRDY(ADC1) && (--wait != 0u)) {}
	if (wait == 0u)
	{
		return false; /* ADC never became ready */
	}

	adc_ready = true;
	return Scope_ADC_SetRate(SCOPE_DEFAULT_RATE);
}

bool Scope_ADC_IsReady(void)
{
	return adc_ready;
}

bool Scope_ADC_ReadOnce(uint16_t *out)
{
	if ((out == NULL) || !adc_ready)
	{
		return false;
	}
	return ADC_Convert(out);
}

/*Sample Timer*/
bool Scope_ADC_SetRate(uint32_t hz)
{
	if (hz < 1u)
	{
		return false; /* invalid: would divide by zero */
	}
	if (hz > Scope_Max_Rate)
	{
		hz = Scope_Max_Rate; /* clamp, as documented in ADC.h */
	}

	uint32_t ticks = SCOPE_TIM_CLK_HZ / hz; /*Timer counts per sample*/
	uint32_t psc = (ticks - 1u) / 65536u; /*Make ARR fit in 16 bits*/
	uint32_t arr = (ticks / (psc + 1u)) - 1u;

	LL_TIM_SetPrescaler(Scope_Tim, psc);
	LL_TIM_SetAutoReload(Scope_Tim, arr);
	LL_TIM_GenerateEvent_UPDATE(Scope_Tim); /*Load psc/arr now*/
	LL_TIM_ClearFlag_UPDATE(Scope_Tim); /*clear the flag that just set*/
	return true;
}

static void Capture_Start(void)
{
	idx = 0u;
	capture_failed = false;
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

	if (!LL_TIM_IsActiveFlag_UPDATE(Scope_Tim))
	{
		return;
	}

	LL_TIM_ClearFlag_UPDATE(Scope_Tim);

	if (idx < SCOPE_CAP_LEN)
	{
		uint16_t sample = 0u;

		if (!ADC_Convert(&sample))
		{
			capture_failed = true; /* the task discards this frame */
			sample = 0u;
		}
		buf[idx] = sample;
		idx = (uint16_t)(idx + 1u);

		if (idx >= SCOPE_CAP_LEN)
		{
			Capture_Stop();
			if (Scope_Task != NULL)
			{
				vTaskNotifyGiveFromISR(Scope_Task, &woken);
			}
			portYIELD_FROM_ISR(woken);
		}
	}
}

/*Convert tenths of a volt (0..30) to ADC counts (0..1023). Out-of-range input is clamped*/
static uint32_t Tenths_To_Counts(uint16_t tenths)
{
	if (tenths > SCOPE_TENTHS_MAX)
	{
		tenths = SCOPE_TENTHS_MAX;
	}
	return ((uint32_t)tenths * Scope_ADC_Max) / SCOPE_VREF_TENTHS;
}

/*Find the first point where the signal crosses the trigger level in the chosen direction.
 *Returns the index to start drawing from (0..WaveD_W), or 0 if no crossing was found (auto: draw from the start)*/
static uint16_t Find_Trigger(uint16_t level, bool rising)
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

/*Drawing: full_counts is the ADC value that reaches the top of the screen.
 *full_counts == 0 is safe: every sample is >= 0, so the first branch returns the top row*/
static uint8_t Sample_To_Y(uint16_t v, uint32_t full_counts)
{
	if ((uint32_t)v >= full_counts)
	{
		return 0u;
	}
	return (uint8_t)((WaveD_H - 1u) - (((uint32_t)v * (WaveD_H - 1u)) / full_counts));
}

/*Returns false (and draws nothing) if the arguments would read past the capture buffer*/
static bool Draw_Trace(uint16_t first, uint32_t full_counts)
{
	if ((first > WaveD_W) || (full_counts == 0u))
	{
		return false;
	}

	uint8_t Y_Prev = Sample_To_Y(buf[first], full_counts);

	WaveD_Clear();
	for (uint8_t x = 0u; x < WaveD_W; x++)
	{
		uint8_t Y = Sample_To_Y(buf[first + x], full_counts);
		uint8_t Lo = (uint8_t)((Y < Y_Prev) ? Y : Y_Prev); /*Joins this point to the last*/
		uint8_t Hi = (uint8_t)((Y < Y_Prev) ? Y_Prev : Y);

		for (uint8_t yy = Lo; yy <= Hi; yy++)
		{
			WaveD_Set_Pixel(x, yy, true);
		}
		Y_Prev = Y;
	}
	WaveD_Flush();
	return true;
}

/*Scope Task*/
void Scope_Task_Body(void)
{
	Scope_Task = xTaskGetCurrentTaskHandle();
	if (Scope_Task == NULL)
	{
		return; /* no task context: nothing safe to do */
	}

	/* osDelay can only fail outside a running task, which cannot happen here */
	(void)osDelay(200u); /*Allows display to finish init*/

	/* Rule 2 exception: intentionally non-terminating RTOS task loop. */
	for (;;)
	{
		if (!adc_ready)
		{
			(void)osDelay(100u); /* ADC init failed: do not capture garbage */
			continue;
		}

		/*Read the volatile controls once per frame*/
		uint16_t time_now  = time;
		uint16_t cur_now   = current;
		uint16_t start_now = start_point;
		uint8_t  mode_now  = mode;

		uint16_t t      = (uint16_t)((time_now < 1u) ? 1u : time_now);  /*0.1 ms units, screen width*/
		uint16_t cur    = (uint16_t)((cur_now < 1u) ? 1u : cur_now);    /*tenths of a volt, screen height*/
		uint16_t level  = (uint16_t)Tenths_To_Counts(start_now);        /*trigger level in ADC counts*/
		bool     rising = (mode_now != 0u);                             /*true = rise, false = fall*/
		uint32_t full   = Tenths_To_Counts(cur);                        /*ADC counts for a full-height screen*/

		/*WaveD_W samples across t x 0.1 ms  ->  rate = WaveD_W * 10000 / (t * SCOPE_DIVS)*/
		uint32_t rate = ((uint32_t)WaveD_W * 10000u) / ((uint32_t)t * SCOPE_DIVS);
		if (!Scope_ADC_SetRate(rate))
		{
			(void)osDelay(50u);
			continue;
		}

		Capture_Start();
		if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0u) /*sleep until ISR says buffer is full*/
		{
			continue; /* woke without a notification: start a new capture */
		}

		if (!capture_failed)
		{
			if (!Draw_Trace(Find_Trigger(level, rising), full))
			{
				capture_failed = true; /* nothing drawn this frame; retry next loop */
			}
		}
		(void)osDelay(50u);
	}
}
