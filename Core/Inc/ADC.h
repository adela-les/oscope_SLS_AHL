/*
 * ADC.h
 *
 *  Created on: Oct 29, 2026
 *      Author: sydney
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdint.h>

#define Scope_ADC_Max 1023u /* 10-bit ADC: readings are 0 to 1023 (0v-3.3v)*/

void Scope_ADC_Init(void); /*Calibrate and enable ADC, set the default sample rate*/

void Scope_ADC_SetRate(uint32_t hz); /*Samples per second (1 to 100000). Take effect at the next capture*/

uint16_t Scope_ADC_ReadOnce(void); /*One blocking conversion, 0 to 1023 (for bring-up tests)*/

void Scope_Task_Body(void); /*Call from the scope task. Captures, draws, repeats*/

void Scope_ADC_Timer_ISR(void); /*Call from the sample timer's IRQ handler*/


#endif /* INC_ADC_H_ */
