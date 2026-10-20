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

#define A_Up 0x31

/*PINOUT
 * PB0-7 ARE D FOR SENDING BITS
 * PB8 IS E, OR CLOCK
 * PB9 is R/W, OR COMMAND OR WHATEVER
 * PB10 is RS OR idk what this does yet
 */



#endif /*INC_CHARACTER_DISPLAY_H_*/
