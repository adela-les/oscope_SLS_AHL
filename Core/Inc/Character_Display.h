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
void DelayLoop(void);

#define A_Up 0x31
#define LCD_DATA_MASK  0x00FF      // PB0-7  -> DB0-DB7
#define LCD_E_PIN      (1 << 8)    // PB8    -> E
#define LCD_RW_PIN     (1 << 9)    // PB9    -> R/W
#define LCD_RS_PIN     (1 << 10)   // PB10   -> RS

/*PINOUT
 * PB0-7 ARE D FOR SENDING BITS
 * PC8 IS E, OR CLOCK
 * PC9 is R/W, OR COMMAND OR WHATEVER
 * PB10 is RS OR idk what this does yet
 */



#endif /*INC_CHARACTER_DISPLAY_H_*/
