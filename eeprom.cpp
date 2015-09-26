#include "eeprom.h"
#include <Arduino.h>

const byte gamePos = 0, missionPos = 1;
const byte serialPos = 2;

void initEEPROM(struct boxSettings *box){
  /*
    read Gamestate, etc. from EEPROM
  */

  byte serial = EEPROM.read(serialPos);

	if (box->gamestate != gs_reset)
			box->gamestate = EEPROM.read(gamePos);
  /* If the eeprom haven't been used before, the read value is 0xFF. The rom
   * might also contain random stuff. Anyway set the game to be running */
  /* Serial has changed - restart game */
  if (box->gamestate > 4 || box->gamestate == gs_reset ||
      box->serial != serial){
			if (box->mission == 1)
					box->mission = MS_INIT;
			box->gamestate = gs_running;
			EEPROM.write(gamePos,box->gamestate);
			EEPROM.write(missionPos,box->mission);
			EEPROM.write(serialPos,box->serial);
  }else
			box->mission = EEPROM.read(missionPos);


  if (box->mission > 200) {
			box->mission = MS_INIT;
			EEPROM.write(missionPos,box->mission);
  }
}

void resetEEPROM(struct boxSettings *box, int mission){

		box->gamestate = gs_running;
		box->mission = mission;
		EEPROM.write(gamePos,box->gamestate);
		EEPROM.write(missionPos,box->mission);
		EEPROM.write(serialPos,-1);
}


void updateEEPROM(int pos,int value){
		EEPROM.write(pos,value);
}
