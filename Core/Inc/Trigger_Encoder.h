/*
 * Trigger_Encoder.h
 *
 *  Created on: Nov 5, 2026
 *      Author: Jagad
 */

#ifndef SRC_TRIGGER_ENCODER_H_
#define SRC_TRIGGER_ENCODER_H_

#include "Voltage_Encoder.h"
#include "main.h"
#include "TimeBase.h"

uint16_t Raise_Lower_Start(void);
int8_t read_encoder(void);

#define initial 15



#endif /* SRC_TRIGGER_ENCODER_H_ */
