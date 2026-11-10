/*
 * Trigger_Encoder.c
 *
 *  Created on: Nov 5, 2026
 *      Author: Jagad
 */

#include "Trigger_Encoder.h"

volatile uint16_t start_point = initial;

int8_t read_encoder()
{
    static enum btn qB = HIGH;

    uint32_t A = (GPIOB->IDR & LL_GPIO_PIN_6) ? 1 : 0;
    uint32_t B = (GPIOB->IDR & LL_GPIO_PIN_7) ? 1 : 0;

    int8_t input = 0;

    switch(qB)
    {
    case HIGH:
        if(B == 0)
        {
            qB = H_BOUNCE_DELAY;
        }
        break;

    case H_BOUNCE_DELAY:
        if(B == 0)
        {
            qB = LOW;
        }
        else
        {
            qB = HIGH;
        }
        break;

    case LOW:
        if(B == 1)
        {
            qB = L_BOUNCE_DELAY;
        }
        break;

    case L_BOUNCE_DELAY:
        if(B == 1)
        {
            qB = RISE;

            if(A == 1)
            {
                input = 1;
            }
            else
            {
                input = -1;
            }
        }
        else
        {
            qB = LOW;
        }
        break;

    case RISE:
        qB = HIGH;
        break;

    case FALL:
        qB = LOW;
        break;
    }

    return input;
}

uint16_t Raise_Lower_Start(void)
{
	for(uint16_t i = 0; i < 1234; i++)
			  {
				  ;
			  }

	  int8_t input = read_encoder();

	  if((input == 1) && (start_point<30))
	  {
		  start_point++;
	  }

	  if((input == -1) && (start_point>0))
	  {
		  start_point--;
	  }


	  return start_point;
}

