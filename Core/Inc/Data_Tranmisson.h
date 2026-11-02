/*
 * Data_Tranmisson.h
 *
 *  Created on: Oct 15, 2026
 *      Author: Jagad
 */

#ifndef INC_DATA_TRANMISSON_H_
#define INC_DATA_TRANMISSON_H_
/*void Character_Display_Command(uint8_t i)
{

LL_GPIO_ResetOutputPin(GPIOB, (1<<9));
LL_GPIO_ResetOutputPin(GPIOB, (1<<8));

LL_GPIO_SetOutputPin(GPIOB,0x0);
 uint8_t P1 = i; //put data on output Port
 LL_GPIO_ResetOutputPin(GPIOB,(1<<10)); //D/I=LOW : send instruction
 LL_GPIO_ResetOutputPin(GPIOB,(1<<9)); //R/W=LOW : Write
 LL_GPIO_SetOutputPin(GPIOB,(1<<8)); //Set Clock Pin High

 LL_GPIO_ResetOutputPin(GPIOB, 0x00FF | (1 << 8) | (1 << 10));

 if(P1 == 0x30)
 {
 LL_GPIO_SetOutputPin(GPIOB,(1<<2)); //Hard code setting 2
 LL_GPIO_SetOutputPin(GPIOB,(1<<3)); //Hard Code setting 3
 osDelay(1); //enable pulse width >= 300ns

}


 else if(P1 == 0x10)
 {
	 LL_GPIO_SetOutputPin(GPIOB,(1<<4)); //Hard code setting

 }

 else if(P1 == 0x0C)
 {
	 LL_GPIO_SetOutputPin(GPIOB,(1<<2)); //Hard code setting
	 LL_GPIO_SetOutputPin(GPIOB,(1<<1)); //Hard code setting
 }

 else if(P1 == 0x06)
 {
	 LL_GPIO_SetOutputPin(GPIOB,(1<<0)); //Hard code setting
	 LL_GPIO_SetOutputPin(GPIOB,(1<<1)); //Hard code setting
	 LL_GPIO_SetOutputPin(GPIOB,(1<<2)); //Hard code setting
 }

 else //0x38
 {
	 LL_GPIO_SetOutputPin(GPIOB,(1<<2)); //Hard code setting
	 LL_GPIO_SetOutputPin(GPIOB,(1<<3)); //Hard code setting
	 LL_GPIO_SetOutputPin(GPIOB,(1<<5)); //Hard code setting

 }

 LL_GPIO_ResetOutputPin(GPIOB,(1<<8)); //Set Clock Pin Low

}

void Character_Display_Write(uint8_t i)
{

 uint8_t P1 = i; //put data on output Port

 LL_GPIO_ResetOutputPin(GPIOB, 0x00FF | (1 << 8) | (1 << 10));

 LL_GPIO_ResetOutputPin(GPIOB, (1<<9)); //reset
 LL_GPIO_ResetOutputPin(GPIOB, (1<<8));

 LL_GPIO_ResetOutputPin(GPIOB,(1<<10)); //D/I=HIGH : send data
 LL_GPIO_SetOutputPin(GPIOB,(1<<9)); //R/W=LOW : Write
 LL_GPIO_SetOutputPin(GPIOB,(1<<8)); //Set Clock Pin High
 osDelay(1); //enable pulse width >= 300ns
 LL_GPIO_ResetOutputPin(GPIOB,(1<<8)); //Set Clock Pin Low

 //at some point, 0 is 1<<0 and 1<<1

}


void Character_Display_Init(void)
{
 LL_GPIO_ResetOutputPin(GPIOB,(0<<8)); //Set Clock Pin Low
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
*/



#endif /* INC_DATA_TRANMISSON_H_ */
