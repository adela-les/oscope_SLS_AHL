/*
 * Character_Display.c
 *
 *  Created on: Oct 20, 2026
 *      Author: Jagad
 */

#include "main.h"
#include "app_freertos.h"
#include "Character_Display.h"


void Character_Display_Command(uint8_t a)
{
	/*
 P1 = i; //put data on output Port
 D_I =0; //D/I=LOW : send instruction
 R_W =0; //R/W=LOW : Write
 E = 1;
 Delay(1); //enable pulse width >= 300ns
 E = 0; //Clock enable: falling edge
 TODO
 */
}


void Character_Display_Write(uint8_t b)
{
	/*
 P1 = i; //put data on output Port
 D_I =1; //D/I=HIGH : send data
 R_W =0; //R/W=LOW : Write
 E = 1;
 Delay(1); //enable pulse width >= 300ns
 E = 0; //Clock enable: falling edge
 TODO
 */
}


void Character_Display_Init(void)
{
 /*E = 0; CLOCK PIN ZERO
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
 Character_Display_Command(0x06); //Entry mode set TODO*/
}
