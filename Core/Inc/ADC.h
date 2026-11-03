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

void Scope_ADC_Init(void);

void Scope_ADC_SetRate(uint32_t hz);

uint16_t Scope_Task_Body(void);

void Scope_Task_Body(void);

void Scope_ADC_Timer_ISR(void);


#endif /* INC_ADC_H_ */
