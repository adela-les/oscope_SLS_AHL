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
#define WaveD_PAGES (WaveD_H / 8u) /*8 pages of 8 rows*/

bool WaveD_Init(void);  /*Set up pins, SPI, frame buffer and mutex. False if the mutex could not be created*/
void WaveD_Task_Body(void); /*Call from the CubeMX display task only. Never returns*/
bool WaveD_Clear(void);  /*Clear the frame buffer. False if not initialized or the mutex failed*/
bool WaveD_Set_Pixel(uint8_t x, uint8_t y, bool on);  /*Set or clear one pixel. False if out of range or the mutex failed*/
bool WaveD_Flush(void);  /*Ask the display task to send the frame. False if the task has not started*/

#endif /*INC_WAVE_DISPLAY_H_*/
