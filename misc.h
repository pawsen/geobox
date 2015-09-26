// -*- coding: utf-8 -*-

#ifndef MISC_H
#define MISC_H
#include <Arduino.h>
#include <LiquidCrystal.h>

#define TEXTLENGTH 80


struct boxSettings{

  // set to 1 when the box is open
  bool open;
  // set to 1 when the backdoor timer reaches backdoor timer2 and this is still 0 - is then being set to 1 to trigger the actual reset
  bool gamereset;
  unsigned long time;
  unsigned long log_time;

  // to prevent usage of the old Servo library, I am instead making
  // sure, that I only talk to one of Software Serial or the Servo at
  // a time. These two flags keep check of that.  only one can be
  // 'attached' at the time
  bool servoattached;
  bool gpsattached;

  bool gpsSignal;
  bool gpsPrintVarning;

  /* declare gamestate and mission */
  byte gamestate; // RUNNING, etc.
  byte gamestate_old;
  byte mission;
  int nMission;

	int timezone;

  /* First time: show mission
     Second time: Show remaining dist, time etc. */
  bool first_time;
  bool second_time;

  bool turnoff;      /* Has the box turned off */
  bool timeout;       /* Can the box auto-turnoff */
  bool msCompleted;  /* is the mission completed */
	bool posCompleted;
	bool timeCompleted;
	bool acceptCompleted;

  /* REED switch */
  bool bdrunning;
  unsigned long bdtimer;

  /* DC-card */
  bool sdPresent;

  /* SERIAL */
  unsigned long serial;
  unsigned int delay;

  /* msg */
		char init[TEXTLENGTH], welcome[TEXTLENGTH], completed[TEXTLENGTH], finish[TEXTLENGTH];

};

struct menuSettings{
  volatile int id;
  volatile bool changed;
};


struct gpsTargetStruct{
  const float flat, flon;
  const unsigned int treshold;
};


// extern boxSettings box;


enum{gs_reset=0,gs_running,gs_finish,gs_showMission,gs_removePower};
/* Note that order matters! They have to be listed as they will be completed */
enum{MS_INIT=99};

/* for opening/closing the box */
const bool OPEN = true;
const bool CLOSE = false;


// void backdoorListen(boxSettings *box);
void backdoorListen(boxSettings *box, LiquidCrystal *lcd);

#endif
