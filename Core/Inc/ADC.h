/*
 * ADC.h
 *
 *  Created on: Oct 29, 2026
 *      Author: sydney
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdint.h>
#include <stdbool.h>

#define Scope_ADC_Max 1023u /* 10-bit ADC: readings are 0 to 1023 (0v-3.3v)*/

/* Calibrate and enable ADC, set the default sample rate. Returns false on any failure. */
bool Scope_ADC_Init(void);

/* True only after Scope_ADC_Init() succeeded. */
bool Scope_ADC_IsReady(void);

/* Samples per second (1 to 100000, values above the max are clamped).
 * Takes effect at the next capture. Returns false if hz is 0. */
bool Scope_ADC_SetRate(uint32_t hz);

/* One blocking conversion. Writes 0 to 1023 to *out. Returns false on timeout,
 * a NULL pointer, or if the ADC is not ready. */
bool Scope_ADC_ReadOnce(uint16_t *out);

void Scope_Task_Body(void);      /* Call from the scope task. Captures, draws, repeats */
void Scope_ADC_Timer_ISR(void);  /* Call from the sample timer's IRQ handler */

#endif /* INC_ADC_H_ */
