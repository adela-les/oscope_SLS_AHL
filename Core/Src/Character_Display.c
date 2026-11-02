/*
 * Character_Display.c
 *
 *  Created on: Oct 20, 2026
 *      Author: Jagad
 */

#include "main.h"
#include "app_freertos.h"
#include "Character_Display.h"


void DelayLoop(void)
{
	for(uint16_t n = 0; n<100;n++)
	{
		;
	}
}


void Character_Display_Command(uint8_t i)
{
	LL_GPIO_ResetOutputPin(GPIOB, LCD_DATA_MASK | LCD_RS_PIN);
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
    LL_GPIO_SetOutputPin(GPIOB, i);            // data on PB0-7 (RS=0, RW=0)

    DelayLoop();                                // setup time before E rises
    LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);    // E high
    DelayLoop();                                 // pulse width
    LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);  // E low: data latched
    DelayLoop();                                // let the command execute
}

void Character_Display_Write(uint8_t i)
{
    LL_GPIO_ResetOutputPin(GPIOB, LCD_DATA_MASK | LCD_RS_PIN);
    LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
    LL_GPIO_SetOutputPin(GPIOB, i | LCD_RS_PIN);   // data + RS=1 (RW stays 0)

    DelayLoop();
    LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);
    DelayLoop();
    LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);
    DelayLoop();
}

void Character_Display_Init(void)
{
    LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);
    for(uint32_t n = 0; n<300000;n++)
    	{
    		;
    	}                        // >40 ms after power-up

    Character_Display_Command(0x30);    // wake up #1
    for(uint32_t n = 0; n<10000;n++)
    	{
    		;
    	}
    Character_Display_Command(0x30);    // wake up #2
    for(uint32_t n = 0; n<10000;n++)
    	{
    		;
    	}
    Character_Display_Command(0x30);    // wake up #3
    DelayLoop();

    Character_Display_Command(0x38);    // 8-bit, 2 lines
    Character_Display_Command(0x10);    // display off
    Character_Display_Command(0x01);    // clear display
    DelayLoop();                          // clear needs ~1.5 ms
    Character_Display_Command(0x0C);    // entry mode: increment
    Character_Display_Command(0x06);    // display on CHANGED
}

void Character_Display_ShowZero(void)
{
    //Character_Display_Command(0x80);    // cursor to row 0, col 0
    Character_Display_Write('A');       // 0x30
}



