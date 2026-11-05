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
		for(volatile uint16_t n = 0; n < 1000; n++)
		{
			;
		}
	}

void Character_Display_Command(uint8_t i)
{

	LL_GPIO_ResetOutputPin(GPIOC, LCD_DATA_MASK | LCD_RS_PIN);
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
	LL_GPIO_SetOutputPin(GPIOC, ((uint32_t)i << LCD_DATA_SHIFT) & LCD_DATA_MASK);


	DelayLoop();                               // setup time before E rises
	LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);    // E high
	DelayLoop();                               // pulse width
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);  // E low: data latched
	DelayLoop();                               // let the command execute
}

void Character_Display_Write(uint8_t i)
{
	LL_GPIO_ResetOutputPin(GPIOC, LCD_DATA_MASK | LCD_RS_PIN);
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
	LL_GPIO_SetOutputPin(GPIOC, (((uint32_t)i << LCD_DATA_SHIFT) & LCD_DATA_MASK) | LCD_RS_PIN);

	DelayLoop();
	LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);
	DelayLoop();
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);
	DelayLoop();
}

void Character_Display_Init(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);

	for(volatile uint32_t n = 0; n < 1000000; n++)   // >40 ms after power-up
	{
		;
	}

	Character_Display_Command(0x30);    // wake up #1
	for(volatile uint32_t n = 0; n < 100000; n++)    // >4.1 ms
	{
		;
	}
	Character_Display_Command(0x30);    // wake up #2
	for(volatile uint32_t n = 0; n < 100000; n++)
	{
		;
	}
	Character_Display_Command(0x30);    // wake up #3
	DelayLoop();

	Character_Display_Command(0x38);    // 8-bit, 2 lines, 5x8
	Character_Display_Command(0x08);    // display off
	Character_Display_Command(0x01);    // clear display
	for(volatile uint32_t n = 0; n < 50000; n++)
	{
		;
	}
	Character_Display_Command(0x0C);    // display on, cursor off
	Character_Display_Command(0x06);    // entry mode: increment

}

void Character_Display_ShowZero(void)
{
	Character_Display_Command(0x80);    // cursor to row 0, col 0
	Character_Display_Write('A');
}



