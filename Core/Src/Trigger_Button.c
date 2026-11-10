/*
 * Trigger_Button.c
 *
 *  Created on: Nov 4, 2026
 *      Author: Jagad
 */

#include "Trigger_Button.h"
#include "main.h"
#include "app_freertos.h"

uint16_t setting = count;
volatile uint8_t mode = 0;

int8_t Button_Read()
{
	static enum btn qIncrement = HIGH;

	uint32_t incrementG = (GPIOC->IDR & LL_GPIO_PIN_13) ? 1:0;

	switch(qIncrement)
	{
	case HIGH:
		if(incrementG==0){
			qIncrement = H_BOUNCE_DELAY;
			break;
			}
		else break;
	case H_BOUNCE_DELAY:

		if(incrementG==0){
			qIncrement = FALL;
			break;
		}
		else{
			qIncrement = HIGH;
			break;
		}
	case FALL:
		qIncrement = LOW;
		return 1;
		break;
	case LOW:
		if(incrementG==1){
			qIncrement = L_BOUNCE_DELAY;
			break;
		}
		else break;
	case L_BOUNCE_DELAY:

		if(incrementG==1){
			qIncrement = RISE;
			break;
		}
		else{
			qIncrement = LOW;
			break;
		}
	case RISE:
		qIncrement = HIGH;
		break;


	}
	return 0;
}


int8_t Rise_Or_Fall(void)
{
	 for(uint16_t i = 0; i < 1234; i++)
		      {
		          ;
		      }

	  int8_t input = Button_Read();

	  if((input == 1) && (mode == 1))
	  {
		  mode = 0;
	  }

	  else if ((input==1) && (mode == 0))
	  {
		  mode = 1;
	  }

	  return mode;
}
