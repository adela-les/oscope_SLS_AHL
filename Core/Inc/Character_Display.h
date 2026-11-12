/*
 * Character_Display.h
 *
 *  Created on: Oct 15, 2026
 *      Author: Jagad
 */

#ifndef INC_CHARACTER_DISPLAY_H_
#define INC_CHARACTER_DISPLAY_H_

void Character_Display_Command(uint8_t a);
void Character_Display_Write(uint8_t b);
void Character_Display_Init(void);
void Character_Display_ShowZero(void);
static void LCD_Delay_E(void);
static void LCD_Delay_Cmd(void);
void Character_Display_SetCursor(uint8_t row, uint8_t col);
void Update_Character_Display_Voltage(uint16_t new_voltage);
void Update_Character_Display_Trigger(uint8_t trigger);
void Update_Character_Display_Time(uint16_t new_time);
void Update_Character_Display_Volt_Start(uint16_t new_start);

#define A_Up 0x31
#define LCD_DATA_MASK  0x03FC      // PB0-7  -> DB0-DB7
#define LCD_E_PIN      (1 << 10)    // PC8    -> E
#define LCD_RW_PIN     (1 << 11)    // PC9    -> R/W
#define LCD_RS_PIN     (1 << 12)   // PB10   -> RS
#define LCD_DATA_SHIFT  2

/*PINOUT
 * PB0-7 ARE D FOR SENDING BITS
 * PC8 IS E, OR CLOCK
 * PC9 is R/W, OR COMMAND OR WHATEVER
 * PB10 is RS OR idk what this does yet
 */



#endif /*INC_CHARACTER_DISPLAY_H_*/
