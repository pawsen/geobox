#ifndef MY_EEPROM_H
#define MY_EEPROM_H

#include <EEPROM.h>
#include <Arduino.h>
#include "misc.h"

void initEEPROM(struct boxSettings *box);
void resetEEPROM(struct boxSettings *box, int mission);
void updateEEPROM(int pos,int value);

extern const byte gamePos, missionPos;

#endif
