/*
 * animation.h
 *
 *  Created on: Feb 9, 2022
 *      Author: Ethan
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "peripherals.h"
#include "note.h"

unsigned long graphicsTimer;
unsigned long millis;

unsigned int refreshRate; // lower than 15 (ms) is unstable
unsigned int refreshCnt;
unsigned int songLngth;
unsigned int ltstCnt;

bool clearBuffer;
bool flushBuffer;

typedef struct
    {
    Note *array;
    size_t used;
    size_t size;
    } Array;

Array song;

void graphicsInit(Note playSong[], unsigned int refresh);
signed int getTDistance(unsigned int noteCnt);
void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, Note element);
void draw(unsigned int cnt);
void freeArray(Array *a);
void timerInit(void);
void animate(void);

void graphicsInit(Note playSong[], unsigned int refresh)
    {
    songLngth = 0;
    size_t n = sizeof(playSong) / sizeof(playSong[0]);
    freeArray(&song);
    initArray(&song, n);
    unsigned int i;
    for (i = 0; i < (unsigned int) n; i++)
	{
	insertArray(&song, playSong[i]);
	songLngth += song.array[i].duration; //in ms
	}

    millis = 0;
    refreshRate = refresh;
    graphicsTimer = millis;
    clearBuffer = false;
    flushBuffer = false;
    refreshCnt = 0;
    timerInit();
    }

void animate(void)
    {

    draw(0);
//    if ((millis >= (graphicsTimer + refreshRate))
//	    && (millis < (graphicsTimer + (refreshRate * 2))) && flushBuffer)
//	{
//
//	Graphics_flushBuffer(&g_sContext);
//	flushBuffer = false;
//	clearBuffer = true;
//	}
//
//    if ((millis >= (graphicsTimer + (refreshRate * 2)))
//	    && (millis < (graphicsTimer + (refreshRate * 3))) && clearBuffer)
//	{
//
//	graphicsTimer = millis;
//	Graphics_clearDisplay(&g_sContext);
//	refreshCnt++;
//	flushBuffer = true;
//	clearBuffer = false;
//	}
    }

void draw(unsigned int cnt)
    {
    Graphics_clearDisplay(&g_sContext);
    Graphics_Rectangle noteBox =
	{
//	.xMin = 0, .xMax = 90, .yMin = 50, .yMax = 96
	.xMin = ((song.array[cnt].pos + 1) * 6) - 6, .xMax = ((song.array[cnt].pos + 1) * 6), .yMin = (int)(0.1 * (getTDistance(cnt) - song.array[cnt].duration)) - 96, .yMax = (int)(0.1 * (getTDistance(cnt))) - 96
	    };
    Graphics_drawRectangle(&g_sContext, &noteBox);
    Graphics_flushBuffer(&g_sContext);
    }

int getTDistance(unsigned int noteCnt)
    {

    signed int distance;
    unsigned int i;

    for (i = 0; i < noteCnt; i++)
	{

	distance -= song.array[i].duration;
	}

    return distance += millis;
    }

void timerInit(void)
    {

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = 1000 - 1; // 1000 cycles = 1ms at 1 MHz
    TA0CTL = TASSEL_2 + MC_1; // SMCLK, contmode
    _BIS_SR(GIE);
    }

void initArray(Array *a, size_t initialSize)
    {

    a->array = malloc(initialSize * sizeof(Note));
    a->used = 0;
    a->size = initialSize;
    }

void insertArray(Array *a, Note element)
    {

// a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
// Therefore a->used can go up to a->size

    if (a->used == a->size)
	{

	a->size++;
	a->array = realloc(a->array, a->size * sizeof(Note));
	}

    a->array[a->used++] = element;
    }

void freeArray(Array *a)
    {

    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
    }

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
    {

    millis++;
    }

#endif /* ANIMATION_H_ */
