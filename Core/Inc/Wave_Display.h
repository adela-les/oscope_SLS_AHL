/*
 * Wave_Display.h
 *
 * Created on: Oct 22, 2026
 *      Author: Sydney
 */

#ifndef INC_WAVE_DISPLAY_H_
#define INC_WAVE_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#define WaveD_W 102u /*Width*/
#define WaveD_H 64u /*Height*/
#define WaveD_Pages (WaveD_H / 8u) /*8 pages of 8 rows*/

void WaveD_Init(void);  /*Reset, Turn Display On, Intialize*/
void Wave_Task_Body(void); /*Call the CubeMX display task*/
void WaveD_Clear(void);  /*Clear the frame Buffer*/
void WaveD_Set_Pixel(uint8_t x, uint8_t y, bool on);  /*Turn on one frame*/
void WaveD_Flush(void);  /*Send the frame*/


#endif /*INC_WAVE_DISPLAY_H_*/
