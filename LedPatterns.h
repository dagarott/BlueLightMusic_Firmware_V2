/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef LEDSPATTERNS_H__
#define LEDSPATTERNS_H__

#include <stdlib.h>
#include <stdint.h>
#include "project.h"

//User setting 
#define OFFLEDS '0'
#define ONLEDS '1'
#define FADE '2'
#define CYCLON '3'
#define FLASHLED '4'
#define FLASHFADE '5'
#define WIPE '6'
#define RING '7'
 
 
#define DELAYFADE 2     //255_up/down_values)*2*1ms_Systick_time=1020ms
#define DELAYCYCLON 80
#define DELAYFLASHLED 100
#define DELAYFLASHFADE 1
#define DELAYWIPE 5
#define DELAYBOUNCE 10
#define DELAYRING 250
#define DELAYRINGBOUNCE 350



void showStrip();
void OffLeds(void);
void SetAll(uint8_t red, uint8_t green, uint8_t blue);
void SetPixel(uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue);
uint16_t FadeInOut(void);
uint16_t Cyclon(void);
uint16_t Flash(void);
uint16_t FlashFadeInOut(void);
uint16_t Wipe(void);
uint16_t Ring(void);


#endif // LEDSPATTERNS_H

