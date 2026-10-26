/*
 * Wave_Display.h
 *
 * Created on: Oct 22, 2026
 *      Author: Sydney
 */

#ifndef INC_WAVE_DISPLAY_H_
#define INC_WAVE_DISPLAY_H_

#define WaveD_W 102;
#define WaveD_H 64;

void WaveD_Init(void);  /*Reset, Turn Display On, Intialize*/
void Wave_Task_Body(void);
void WaveD_Clear(void);  /*Clear the frame Buffer*/
void WaveD_Set_Pixel(int x, int y);  /*Turn on one frame*/
void WaveD_Flush(void);  /*Send the frame*/


#endif /*INC_WAVE_DISPLAY_H_*/
