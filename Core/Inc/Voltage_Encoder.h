/*
 * Voltage_Encoder.h
 *
 *  Created on: Nov 2, 2026
 *      Author: Jagad
 */

#ifndef INC_VOLTAGE_ENCODER_H_
#define INC_VOLTAGE_ENCODER_H_

#include "main.h"

int8_t Raise_Lower_Voltage(void);
int8_t Read_Input(void);

static enum btn {RISE, HIGH, H_BOUNCE_DELAY, FALL, LOW, L_BOUNCE_DELAY};
#define count 15



#endif /* INC_VOLTAGE_ENCODER_H_ */
