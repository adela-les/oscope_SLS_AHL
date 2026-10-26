/*
 * Character_Display.c
 *
 *  Created on: Oct 20, 2026
 *      Author: Jagad
 */

#include "main.h"
#include "app_freertos.h"
#include "Character_Display.h"


void Character_Display_Command(uint8_t i)
{

 uint8_t P1 = i; //put data on output Port
 LL_GPIO_SetOutputPin(GPIOB,(0<<10)); //D/I=LOW : send instruction
 LL_GPIO_SetOutputPin(GPIOB,(1<<9)); //R/W=LOW : Write
 LL_GPIO_SetOutputPin(GPIOB,(1<<8)); //Set Clock Pin High
 osDelay(1); //enable pulse width >= 300ns
 LL_GPIO_SetOutputPin(GPIOB,(0<<8)); //Set Clock Pin Low

}


void Character_Display_Write(uint8_t i)
{

 uint8_t P1 = i; //put data on output Port
 LL_GPIO_SetOutputPin(GPIOB,(0<<10)); //D/I=HIGH : send data
 LL_GPIO_SetOutputPin(GPIOB,(1<<9)); //R/W=LOW : Write
 LL_GPIO_SetOutputPin(GPIOB,(1<<8)); //Set Clock Pin High
 osDelay(1); //enable pulse width >= 300ns
 LL_GPIO_SetOutputPin(GPIOB,(0<<8)); //Set Clock Pin Low

 //at some point, 0 is 1<<0 and 1<<1

}


void Character_Display_Init(void)
{
 LL_GPIO_SetOutputPin(GPIOB,(0<<8)); //Set Clock Pin Low
 osDelay(100); //Wait >40 msec after power is applied
 Character_Display_Command(0x30); //command 0x30 = Wake up
 osDelay(30); //must wait 5ms, busy flag not available
 Character_Display_Command(0x30); //command 0x30 = Wake up #2
 osDelay(10); //must wait 160us, busy flag not available
 Character_Display_Command(0x30); //command 0x30 = Wake up #3
 osDelay(10); //must wait 160us, busy flag not available
 Character_Display_Command(0x38); //Function set: 8-bit/2-line
 Character_Display_Command(0x10); //Set cursor
 Character_Display_Command(0x0c); //Display ON; Cursor ON
 Character_Display_Command(0x06); //Entry mode set
}
