/* -*- coding: utf-8 -*- */

#ifndef MY_PROCESSSD
#define MY_PROCESSSD


#include <petit_fatfs.h>
#include <MemoryFree.h>
#include "TheBox.h"

#include "misc.h"
#include <Time.h>

#define TEXTLENGTH 80

struct missionStruct{
  /* gps */
  /* floats only have 6-7 decimal digits of precision
     (total number of digits) Double is the same as floats */
  float flat, flon;
  unsigned int treshold;
  /*  */
  char text[TEXTLENGTH], completed[TEXTLENGTH];
  /* time_t = unsigned long */
  time_t time;
  bool requirePos, requireTime, requireAccept;
};


/* Setup for petit FatFs wrapper  */
void spi_init();
void tx(byte d);
byte rx();


void fileRead(int err, const char *file, int nMission, struct missionStruct *ms);
void settingsRead(int err, const char *file, struct boxSettings *box);
//void settingsRead(int err, struct boxSettings *box);
void processChunk(char * str, byte readPart, struct missionStruct *ms);


#endif
