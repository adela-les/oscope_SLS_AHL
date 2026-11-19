/*
 * Character_Display.c
 *
 *  Created on: Oct 20, 2026
 *      Author: Jagad
 */

#include "main.h"
#include "app_freertos.h"
#include "Character_Display.h"



static void LCD_Delay_E(void)
{
	for (volatile uint16_t n = 0; n < 10; n++) { ; }
}

// Time for the LCD to execute an instruction (needs ~40 us)
static void LCD_Delay_Cmd(void)
{
	for (volatile uint16_t n = 0; n < 400; n++) { ; }
}

void Character_Display_Command(uint8_t i)
{

	LL_GPIO_ResetOutputPin(GPIOC, LCD_DATA_MASK | LCD_RS_PIN);
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
	LL_GPIO_SetOutputPin(GPIOC, ((uint32_t)i << LCD_DATA_SHIFT) & LCD_DATA_MASK);


	LCD_Delay_E();                               // setup time before E rises
	LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);    // E high
	LCD_Delay_E();                              // pulse width
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);  // E low: data latched
	LCD_Delay_Cmd();                               // let the command execute
}

void Character_Display_Write(uint8_t i)
{
	LL_GPIO_ResetOutputPin(GPIOC, LCD_DATA_MASK | LCD_RS_PIN);
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN | LCD_RW_PIN);
	LL_GPIO_SetOutputPin(GPIOC, (((uint32_t)i << LCD_DATA_SHIFT) & LCD_DATA_MASK) | LCD_RS_PIN);

	LCD_Delay_E();
	LL_GPIO_SetOutputPin(GPIOC, LCD_E_PIN);
	LCD_Delay_E();
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);
	LCD_Delay_Cmd();
}

void Character_Display_SetCursor(uint8_t row, uint8_t col)
{
	if (row > 1U)  { return; } /*ASSERTING THAT VALUES ARE IN RANGE*/
	if (col > 15U) { return; }

    uint8_t addr = col;

    if (row == 1)
    {
        addr = 0x40 + col;
    }

    Character_Display_Command(0x80 | addr);
}

void Character_Display_Init(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LCD_E_PIN);

	osDelay(50);

	Character_Display_Command(0x30);    // wake up #1
	osDelay(6);
	Character_Display_Command(0x30);    // wake up #2
	osDelay(2);
	Character_Display_Command(0x30);    // wake up #3

	Character_Display_Command(0x38);    // 8-bit, 2 lines, 5x8
	Character_Display_Command(0x08);    /* display off */
	Character_Display_Command(0x01);    /* clear display*/
	osDelay(3);

	Character_Display_Command(0x0C);
	Character_Display_Command(0x06);

}

void Character_Display_ShowZero(void)
{

	Character_Display_SetCursor(0, 3);
	Character_Display_Write('V');
	Character_Display_Write(' ');
	Character_Display_Write('T');
	Character_Display_Write('a');
	Character_Display_Write('l');
	Character_Display_Write('l');

	Character_Display_SetCursor(1, 3);
	Character_Display_Write('.');
	Character_Display_SetCursor(1, 6);
	Character_Display_Write('m');
	Character_Display_Write('s');

	Character_Display_SetCursor(1, 12);
	Character_Display_Write('F');
	Character_Display_Write('a');
	Character_Display_Write('l');
	Character_Display_Write('l');

	Character_Display_SetCursor(0, 14);
	Character_Display_Write('V');
	Character_Display_Write('t');

}

void Update_Character_Display_Voltage(uint16_t new_voltage)
{
	if (new_voltage > 31) { return; } /*Asserting voltage is no more than 30*/

	uint16_t tens_place = new_voltage / 10;
	uint16_t ones_place = (uint16_t)new_voltage - (uint16_t)(tens_place * 10);

	Character_Display_Command(0x80);    // cursor to row 0, col 0
	Character_Display_Write(tens_place + '0');
	Character_Display_SetCursor(0, 1);
	Character_Display_Write('.');
	Character_Display_SetCursor(0, 2);
	Character_Display_Write(ones_place + '0');
}

void Update_Character_Display_Trigger(uint16_t trigger)
{
	if (trigger > 1U) { return; } /*Trigger should only ever be 1 or 0*/

	if(trigger==0)
	{
		Character_Display_SetCursor(1, 12);
		Character_Display_Write('F');
		Character_Display_SetCursor(1, 13);
		Character_Display_Write('a');
		Character_Display_SetCursor(1, 14);
		Character_Display_Write('l');
		Character_Display_SetCursor(1, 15);
		Character_Display_Write('l');
	}

	else
	{
		Character_Display_SetCursor(1, 12);
		Character_Display_Write('R');
		Character_Display_SetCursor(1, 13);
		Character_Display_Write('i');
		Character_Display_SetCursor(1, 14);
		Character_Display_Write('s');
		Character_Display_SetCursor(1, 15);
		Character_Display_Write('e');
	}
}

void Update_Character_Display_Time(uint16_t new_time)
{
	if (new_time > 1001U) { return; } /*Time should never be greater than 100ms*/

	uint16_t thou_place = new_time / 1000;
	uint16_t hund_place = (uint16_t)(new_time - (thou_place * 1000)) / 100;
	uint16_t tens_place = (uint16_t)(new_time - (thou_place * 1000) - (hund_place * 100)) / 10;
	uint16_t ones_place = (uint16_t)(new_time - (thou_place * 1000) - (hund_place * 100) - (tens_place * 10));

	Character_Display_SetCursor(1, 0);    // cursor to row 0, col 0
	Character_Display_Write(thou_place + '0');
	Character_Display_SetCursor(1, 1);
	Character_Display_Write(hund_place + '0');
	Character_Display_SetCursor(1, 2);    // cursor to row 0, col 0
	Character_Display_Write(tens_place + '0');
	Character_Display_SetCursor(1, 4);
	Character_Display_Write(ones_place + '0');

}

void Update_Character_Display_Volt_Start(uint16_t new_start)
{
	if (new_start > 31U) { return; } /* Should never be greater than 30*/

	uint16_t tens_place = new_start / 10;
	uint16_t ones_place = (uint16_t)new_start - (uint16_t)(tens_place * 10);

	Character_Display_SetCursor(0, 11);
	Character_Display_Write(tens_place + '0');
	Character_Display_SetCursor(0, 12);
	Character_Display_Write('.');
	Character_Display_SetCursor(0, 13);
	Character_Display_Write(ones_place + '0');

}



