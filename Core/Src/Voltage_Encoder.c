/*
 * Voltage_Encoder.c
 *
 *  Created on: Nov 2, 2026
 *      Author: Jagad
 */

#include "Voltage_Encoder.h"
#include "main.h"
#include "app_freertos.h"

volatile uint16_t current = count;

int8_t Read_Input()
{
    static enum btn qB = HIGH;

    uint32_t A = (GPIOB->IDR & LL_GPIO_PIN_2) ? 1 : 0;
    uint32_t B = (GPIOB->IDR & LL_GPIO_PIN_3) ? 1 : 0;

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

uint16_t Raise_Lower_Voltage(void)
{
	 for(uint16_t i = 0; i < 1234; i++)
		      {
		          ;
		      }

	  int8_t input = Read_Input();

	  if((input == 1) && (current<30))
	  {
		  current++;
	  }

	  if((input == -1) && (current>0))
	  {
		  current--;
	  }

	  return current;
}
