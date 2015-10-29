// -*- coding: utf-8 -*-

#ifndef THEBOX_V2_H
#define THEBOX_V2_H

#include "misc.h"
#include <Flash.h>

#define F2(str) (_FLASH_STRING(PSTR(str)))
#define byte uint8_t
//#define DEBUG
#define _SD


/* set the display time on the LCD*/
#ifdef DEBUG
const int DELAY1 = 2000;  /* For showing missions etc */
const int DELAY2 = 2000;  /* For turnoff */
const int DELAY_LCD_PRINT = 1;
const unsigned long SHUTDOWN_TIME = 10000;
const unsigned long GPS_TIMEOUT = 10000;
const unsigned long GPS_SHOW_WARNING = 8000;
#else
const int DELAY1 = 10000;
const int DELAY2 = 5000;
const int DELAY_LCD_PRINT = 100;
const unsigned long SHUTDOWN_TIME = 180000;  /* 3 min */
const unsigned long GPS_TIMEOUT = 180000;
const unsigned long GPS_SHOW_WARNING = 15000;
#endif


/* Servo settings */
const int SERVO_DELAY = 15;
const int SERVO_MAX = 175; //maximum angle
const int SERVO_MIN = 90;  //minimum angle
const int SERVO_STEPSIZE = 1; //degrees between steps

/* interval to log GPS data to SD-card */
const unsigned long LOG_INTERVAL = 5000;

void checkButtons(boxSettings *box);

void printGameState(const char *);
void printRemainDist(unsigned long dist);
void check_inactivity(boxSettings *box);
void redisplayMission(boxSettings *box);
void missionCompleted(boxSettings *box);
void boxActivity(boxSettings *box);

void stringToLCD_flash(const _FLASH_STRING &, bool w_delay=true);
void stringToLCD(const char *stringIn, bool w_delay = true);

void smartDelay(unsigned long ms);

void toogleBox(boxSettings *box, const bool open);
void turnOff(const int del);

#endif
