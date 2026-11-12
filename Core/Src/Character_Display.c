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

	/*for(volatile uint32_t n = 0; n < 1000000; n++)   // >40 ms after power-up
	{
		;
	}
	*/
	osDelay(50);

	Character_Display_Command(0x30);    // wake up #1
	/*for(volatile uint32_t n = 0; n < 100000; n++)    // >4.1 ms
	{
		;
	}*/
	osDelay(6);
	Character_Display_Command(0x30);    // wake up #2
	/*for(volatile uint32_t n = 0; n < 100000; n++)
	{
		;
	}*/
	osDelay(2);
	Character_Display_Command(0x30);    // wake up #3

	Character_Display_Command(0x38);    // 8-bit, 2 lines, 5x8
	Character_Display_Command(0x08);    // display off
	Character_Display_Command(0x01);    // clear display
	/*for(volatile uint32_t n = 0; n < 50000; n++)
	{
		;
	}*/
	osDelay(3);

	Character_Display_Command(0x0C);    // display on, cursor off
	Character_Display_Command(0x06);    // entry mode: increment

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
	uint8_t tens_place = new_voltage / 10;
	uint8_t ones_place = new_voltage - (tens_place * 10);

	Character_Display_Command(0x80);    // cursor to row 0, col 0
	Character_Display_Write(tens_place + '0');
	Character_Display_SetCursor(0, 1);
	Character_Display_Write('.');
	Character_Display_SetCursor(0, 2);
	Character_Display_Write(ones_place + '0');
}

void Update_Character_Display_Trigger(uint8_t trigger)
{
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
	uint8_t thou_place = new_time / 1000;
	uint8_t hund_place = (new_time - (thou_place * 1000)) / 100;
	uint8_t tens_place = (new_time - (thou_place * 1000) - (hund_place * 100)) / 10;
	uint8_t ones_place = (new_time - (thou_place * 1000) - (hund_place * 100) - (tens_place * 10));

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
	uint8_t tens_place = new_start / 10;
	uint8_t ones_place = new_start - (tens_place * 10);

	Character_Display_SetCursor(0, 11);
	Character_Display_Write(tens_place + '0');
	Character_Display_SetCursor(0, 12);
	Character_Display_Write('.');
	Character_Display_SetCursor(0, 13);
	Character_Display_Write(ones_place + '0');

}



