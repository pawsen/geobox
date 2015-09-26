// -*- coding: utf-8 -*-

/* TheBox uses around 180mA */

#include "pinSetup.h"
#include "eeprom.h"
#include "TheBox.h"
#include "processSD.h"

#include <Servo.h>
#include <LiquidCrystal.h>
#include <gSoftSerial.h>
#include <Time.h>
#include <MemoryFree.h>
#include <Flash.h>
#include <TinyGPS++.h>

#define F2(str) (_FLASH_STRING(PSTR(str)))
void stringToLCD_flash(const _FLASH_STRING &, bool w_delay=true);
void stringToLCD(const char *stringIn, bool w_delay = true);

void dateFromSecs(const time_t sec,byte &Day,byte &Hours,byte &Mins,byte &Secs){
  /* calculate time from seconds */

  Day = int(sec/(86400));      /* days - remember to round!  24*60*60 = 86400(to prevent overflow) */
  Hours = ((sec/(60*60))%24);  /* hours */
  Mins = ((sec/60)%60);        /* mins */
  Secs = (sec%60);             /* secs */

  return;
}


static LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
static Servo myservo;
static const unsigned long GPS_BAUD = 9600;
TinyGPSPlus gps;
// rx = yellow, tx = white, vcc = black, gnd = green
gSoftSerial ss(GPS_RX_PIN,GPS_TX_PIN);


struct boxSettings box;
struct menuSettings menu;
struct missionStruct ms;

const char SDfilename[] = "gpsdata.txt";

static const char SETTINGS[] = "set.txt";
static const char MISSIONS[] = "mission.txt";

void setup()
{
  // initiate SoftwareSerial, which we use to talk to the GPS
  ss.begin(GPS_BAUD);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  pinMode(BACKDOORPIN,INPUT);
  digitalWrite(BACKDOORPIN,HIGH);
  /* polulu-pin - pull this one high, and the battery power is cut - has no effect on external power */
  pinMode(POLULUPIN,OUTPUT);
  digitalWrite(POLULUPIN,LOW);

  /* Buttons for menu */
  pinMode(BUTTON_LPUSHPIN,INPUT);
  digitalWrite(BUTTON_LPUSHPIN,HIGH);
  pinMode(BUTTON_RPUSHPIN,INPUT);
  digitalWrite(BUTTON_RPUSHPIN,HIGH);
	//attachInterrupt(0,checkButtons,LOW);
	//attachInterrupt(1,checkButtons,LOW);

  /* Petit FatFs SD card library  */
  spi_init();
  PFFS.begin(2, rx, tx);

#ifdef DEBUG
  Serial.begin(9600);
  // forced to be compiled into and read from Flash
  Serial.print(F("Free RAM = "));
  Serial.println(freeMemory(), DEC);
#endif

	// Set standard UTC_offset/timezone. Can also be set in settings file on SD card.
	box.timezone = 2;
	// cast char const* into a char*
  settingsRead(PFFS.open_file(const_cast<char*>(SETTINGS)),SETTINGS,&box);
	box.gamestate = gs_running;
  initEEPROM(&box);
	//box.mission = 2;
  if (box.gamestate == gs_running) {
			if (box.mission == MS_INIT){
					/* make sure the box is closed */
					toogleBox(&box, CLOSE);
					stringToLCD(box.init);
					box.mission = 1;
					updateEEPROM(missionPos,box.mission);
			}else{
					stringToLCD(box.welcome);
			}
  } else if (box.gamestate == gs_finish){
			stringToLCD(box.finish);
	}
	smartDelay(DELAY2);

  // mastertimer starts here
  box.time = millis();
  box.first_time = true;
  box.second_time = true;
  box.turnoff = false;
	box.posCompleted = false;
	box.timeCompleted = false;
	box.acceptCompleted = false;
  box.msCompleted = false;
  box.gpsPrintVarning = true;
  box.timeout = true; // false
  menu.id = 0; menu.changed = false;
  box.log_time = millis();

  fileRead(PFFS.open_file(const_cast<char*>(MISSIONS)),box.mission,&ms);
}
void loop() {
		backdoorListen(&box, &lcd);
		if (box.timeout)
				check_inactivity(&box);

		unsigned long start = millis();
		do{
				/* buttons for menu */
				checkButtons(&box);
				/* Feed gps */
				smartDelay(0);
		} while (millis() - start < 1000);

#ifdef DEBUG
		if (box.time > 5000 && gps.charsProcessed() < 10)
				Serial.println(F("No connection to GPS!"));
#endif


		/* set time - Notice that day() etc. are functions. Thus we them capitalized */
		char dateTime[20];
		const time_t *t = NULL;  /* pointer to const value */
		int Year;
		byte Month, Day, Hour, Minute, Second;
		if(gps.date.isValid()){
				Month = gps.date.month(), Day = gps.date.day(), Year = gps.date.year();
				Hour = gps.time.hour(), Minute =  gps.time.minute(), Second =  gps.time.second();
				setTime(Hour, Minute, Second, Day, Month, Year);
				adjustTime(box.timezone * SECS_PER_HOUR);
				/* write time to string */
				sprintf(dateTime, "%02d-%02d-%02d,%02d:%02d:%02d",
								Year, Month, Day, Hour + box.timezone, Minute, Second);
		}
		/* returns the current time in second since 1970(UNIX time)... */
		time_t time = now();

		double dist = 0;
		const char *enhed;

		switch(box.gamestate) {
    case gs_reset:
				// game has just been reset - lock the box and shut down
				resetEEPROM(&box, MS_INIT);
#ifdef DEBUG
				printGameState("gs_reset");
#endif
				stringToLCD_flash(F2("box locked and reset"));
				delay(2000);
				stringToLCD_flash(F2("Good Luck!"));
				box.turnoff = false;
				turnOff(2000);
				break;

    case gs_running:
        /* Ensure satellites are found if needed */
        if((ms.requirePos || ms.requireTime) &&
           (!gps.location.isValid() && !box.first_time)){
						if(box.gpsPrintVarning && (millis() - box.time > GPS_SHOW_WARNING)){
								stringToLCD_flash(F2("...Venter paa          satellitter..."));
								if (ms.requireTime){
										t = &ms.time;
										lcd.setCursor(0,2);
										lcd.print(F("Mission gennemfoeres"));
										sprintf(dateTime, "%02d/%02d/%02d %02d:%02d:%02d  ",
														day(*t),month(*t), year(*t), hour(*t), minute(*t), second(*t));
										lcd.setCursor(0,3), lcd.print(dateTime);
								}
								box.gpsPrintVarning = false;
						}
						smartDelay(1000);
						/* turnoff when no satellites can be found */
						if ((millis() - box.time > GPS_TIMEOUT) && box.timeout){
								if(!box.turnoff)
										turnOff(DELAY2);
						}
						break;
        }
        // the game is running. Show mission even if there's no GPS signal
        if(box.first_time){
						stringToLCD(ms.text);
						smartDelay(DELAY1);
						break;
        }

				// All missions of the same kind is grouped, eg
				if (ms.requirePos && !box.posCompleted){
						/* All missions only requiring position to be satisfied: */
						dist =  /* distance in meters */
								TinyGPSPlus::distanceBetween(
										gps.location.lat(),
										gps.location.lng(),
										ms.flat,
										ms.flon);
						if (dist < ms.treshold) {
								box.posCompleted = true;
						} else {
								/* print remaining distance to mission */
								if(box.second_time){
										box.second_time = false;
										stringToLCD_flash(F2("Afstand til mission-en: "));
								}
								if( dist < 1000)
										enhed = " m";
								else{
										dist /= 1000;
										enhed = " km";
								}
								lcd.setCursor(1,3), lcd.print(F("                  "));  /* clear line */
								lcd.setCursor(1,3), lcd.print(dist), lcd.print(enhed);
						}
				} else if (ms.requireTime && !box.timeCompleted){
						/* missioner der kun kræver tiden er opfyldt */
						if(time > ms.time){
								box.timeCompleted = true;
						} else {
								// printRemainTime(dist);
								/* pointer has to be set here, because it's initialized inside main-loop */
								t = &ms.time;
								if(box.second_time){
										box.second_time = false;
										stringToLCD_flash(F2("Mission gennemfoeres"));
										sprintf(dateTime, "%02d/%02d/%02d %02d:%02d:%02d  ",
														day(*t),month(*t), year(*t), hour(*t), minute(*t), second(*t));
										lcd.setCursor(0,1), lcd.print(dateTime);
										lcd.setCursor(0,2), lcd.print(F("Der mangler: "));
								}
								/* get remaining time until mission is completed */
								dateFromSecs(*t-time, Day, Hour, Minute, Second);
								sprintf(dateTime, "d: %02d, t: %02d:%02d:%02d",
												Day, Hour, Minute, Second);
								lcd.setCursor(0,3), lcd.print(dateTime);
						}
				} else if (ms.requireAccept){
						/* missions requiring that I accept them, eg. use the backdoor */

				}
				/* check if all requirements are completed */
				if ((ms.requirePos + ms.requireTime + ms.requireAccept) ==
						(box.posCompleted + box.timeCompleted + box.acceptCompleted)){
						box.msCompleted = true;
				}

				break;
    case gs_finish:
				/* the game is over - time to open the box */
				if(box.first_time){
						box.first_time = false;
						stringToLCD(ms.completed);
						smartDelay(DELAY2);
#ifdef DEBUG
						printGameState("gs_finish");
#endif
						toogleBox(&box, OPEN);
						if (box.timeout)
								turnOff(3000);
				}
				break;
    case gs_showMission:
				/* show:
					 current location and time
					 current mission + dist to go
					 completed missions
				*/
				if(menu.changed || menu.id == 0){
						if (menu.changed){
								lcd.clear();
								menu.changed = false;
						}
						if (menu.id == 0){
								/* show current pos and time */
								lcd.setCursor(0,0);
								if (gps.location.isValid() || gps.location.isUpdated())
										lcd.print(gps.location.lat(),6), lcd.print(","), lcd.print( gps.location.lng(),6);
								else
										lcd.print(F("pos not valid"));
								lcd.setCursor(0,1);
								if (gps.time.isValid() || gps.time.isUpdated())
										lcd.print(dateTime);
								else
										lcd.print(F("tid not valid"));
//								if (gps.satellites.isUpdated()){
								lcd.setCursor(0,2), lcd.print(F("#sat: "));
								lcd.print(gps.satellites.value());
								lcd.print(F(", H: "));
//								}

								/* clear line */
								lcd.setCursor(11,2), lcd.print(F("         "));
								lcd.setCursor(11,2);
								if(gps.altitude.isValid() || gps.altitude.isUpdated())
										lcd.print(gps.altitude.meters(),2), lcd.print("m");
								else
										lcd.print(F("not valid"));
								lcd.setCursor(0,3);
								if (gps.speed.isValid() || gps.speed.isUpdated()){
										lcd.print("Fart: ");
										lcd.setCursor(6,3), lcd.print(F("             "));
										lcd.setCursor(6,3);
										lcd.print(gps.speed.kmph()),lcd.print(" km/h");
								}else
										lcd.print(F("Fart not valid"));
#ifdef DEBUG
								Serial.print(F("pos: "));
								Serial.print(gps.location.lat(),6), Serial.print(","), Serial.println( gps.location.lng(),6);
								Serial.print(F("Hoejde: ")),Serial.print(gps.altitude.meters());
								Serial.print(F(", #sat: ")), Serial.println(gps.satellites.value());
								Serial.print(F("tid: ")), Serial.println(dateTime);
								Serial.print(F("Fart: ")),Serial.println(gps.speed.kmph());
#endif

						}else{ /* menu.id > 0 */
								/* toggle throug completed missions */
								/* There's not enough space on the arduino to store all missions
								 * in ram, so we read them one at a time from the memory-card.
								 * Remember to reread the current mission, when exiting the
								 * showMission
								 */
								fileRead(PFFS.open_file(const_cast<char*>(MISSIONS)),menu.id,&ms);
								stringToLCD(ms.text,false);
#ifdef DEBUG
								Serial.println(F("Completed missions:"));
								Serial.println(ms.text);
#endif
						}
				} /* if menu.changed */
				break;
    case gs_removePower:
				// we're waiting for power to be removed
				if (!box.turnoff){
						box.turnoff = true;
						lcd.clear();
						lcd.setCursor(0,1);
						lcd.print("please remove power");
#ifdef DEBUG
						printGameState("gs_remove power");
#endif
				}

				break;
		}// end of gamestate
		box.first_time = false;

		/* missionComplete have to be called after box.first_time is set to false.
		 * This is because box.first_time is set to true in missionComplete.
		 */
		if(box.msCompleted){
				/* Write the mission as a waypoint to SD card */
/*
#ifdef _SD
				if (box.sdPresent){
						dataFile = SD.open(SDfilename,FILE_WRITE);
						writeSD(dataFile,"W",dateTime,gps,msHeader[box.mission]);
				}
#endif
*/
				smartDelay(DELAY2);
				missionCompleted(&box);
		}
}



void stringToLCD(const char *stringIn, bool w_delay /* = true */) {

  byte lineCount = 0;
  byte lineNumber = 0;
  bool stillProcessing = true;
  byte charCount = 1;
  lcd.clear();
  lcd.setCursor(0,0);

  while(stillProcessing) {
    if (++lineCount > 20) {    // have we printed 20 characters yet (+1 for the logic)
      lineNumber++;
      lcd.setCursor(0,lineNumber);   // move cursor down
      lineCount = 1;
    }

    lcd.print(stringIn[charCount - 1]);

    if (!stringIn[charCount]) {   // no more chars to process?
      stillProcessing = false;
    }
    charCount++;
    if(w_delay)
      delay(DELAY_LCD_PRINT);
  }
}


void stringToLCD_flash(const _FLASH_STRING &stringIn, bool w_delay /* = true */) {
  // void stringToLCD(const char *stringIn, bool w_delay /* = true */) {

  byte lineCount = 0;
  byte lineNumber = 0;
  byte stillProcessing = 1;
  byte charCount = 1;
  lcd.clear();
  lcd.setCursor(0,0);

  while(stillProcessing) {
    if (++lineCount > 20) {    // have we printed 20 characters yet (+1 for the logic)
      lineNumber += 1;
      lcd.setCursor(0,lineNumber);   // move cursor down
      lineCount = 1;
    }

    lcd.print(stringIn[charCount - 1]);

    if (!stringIn[charCount]) {   // no more chars to process?
      stillProcessing = 0;
    }
    charCount += 1;
    if(w_delay)
      delay(DELAY_LCD_PRINT);
  }
}



void toogleBox(boxSettings *box, const bool open_box) {
  /* open or close box */

  delay(250);
  myservo.attach(SERVOPIN);

  if(open_box){
    for(int pos = SERVO_MAX; pos>=SERVO_MIN; pos-=SERVO_STEPSIZE) {
      myservo.write(pos);
      delay(SERVO_DELAY);
    }
  } else{
    for(int pos = SERVO_MIN; pos < SERVO_MAX; pos += SERVO_STEPSIZE) {
      myservo.write(pos);
      delay(SERVO_DELAY);
    }
  }


  delay(250);
  myservo.detach();

#ifdef DEBUG
  if(open_box)
    Serial.println(F("box opened"));
  else
    Serial.println(F("box closed"));
#endif
}


void printGameState(const char *called_from){
  // copy code from robot
  Serial.print(called_from);
  Serial.print(F(", GameState: "));
  Serial.print(box.gamestate);
  Serial.print(F(", Current mission: "));
  Serial.println(box.mission);

}



void missionCompleted(boxSettings *box){

  /* update to next mission */
#ifdef DEBUG
  printGameState("mission completed");
#endif

  box->time = millis();
  box->first_time = true;
  box->second_time = true;
  box->turnoff = false;
	box->posCompleted = false;
	box->timeCompleted = false;
	box->acceptCompleted = false;
	box->msCompleted = false;


  if(box->mission >= box->nMission){
			/* All missions done. Game finished' */
			//stringToLCD(box->finish);
			box->gamestate = gs_finish;
			EEPROM.write(gamePos,box->gamestate);
			return;
  }
	stringToLCD(ms.completed);
	//stringToLCD(box->completed);

  box->mission++;
  box->gamestate = gs_running;
  updateEEPROM(missionPos,box->mission);
	fileRead(PFFS.open_file(const_cast<char*>(MISSIONS)),box->mission,&ms);

  smartDelay(DELAY1);
  return;
}


void smartDelay(unsigned long ms){
		/* This custom version of delay() ensures that the gps object
		 * is being "fed".
		 */

		unsigned long start = millis();
		do {
				while (ss.available()){
						gps.encode(ss.read());
						if(menu.changed)
								return;
				}
		} while (millis() - start < ms);
}


void check_inactivity(boxSettings *box){
  if ( (millis()-box->time > SHUTDOWN_TIME) && !box->turnoff){
    stringToLCD_flash(F2("Inaktiv for laenge. Farvel"));
    turnOff(DELAY2);
#ifdef DEBUG
    Serial.println(F("check_inak"));
#endif
  }
}


void turnOff(const int del){
  if(!box.turnoff){
    delay(del);
    stringToLCD_flash(F2("Slukker"));
    digitalWrite(POLULUPIN,HIGH);
    delay(2000);
    // switch to message if running on external power
    box.gamestate = gs_removePower;
#ifdef DEBUG
    Serial.println("turnOff");
#endif
  }
}

void checkButtons(boxSettings *box){
  /*
    Push button: (Start with)
    Completed missions:
    Press <,> to cycle through missions
    Show mission number in lower right corner. Dont need rotary encoder.
    Just three ordinary buttons. Maybe just two buttons <,>
    and then a long click to activate/deactivate.
    Push again to cancel menu. Have a timer to turn off again.
  */

  int button_delay = 0;
  bool first_entry = true;
  bool second_entry = true;
  while( digitalRead(BUTTON_RPUSHPIN) == LOW ){
    if( box->gamestate == gs_showMission && menu.id < box->mission &&
        !menu.changed){
      menu.id++;
      menu.changed = true;
#ifdef DEBUG
      Serial.println(F("changed menu id ++"));
#endif
    }

    button_delay++;
    delay(100); //debounce
    if (button_delay > 5 && first_entry){
      first_entry = false;
      menu.changed = true;
      menu.id = 0;
			// swap gamestate, eg if showMission then return to game and visa versa
      if( box->gamestate == gs_showMission){
					/* old gamestate can be both running & finish */
					box->gamestate = gs_running;//box->gamestate_old;
					fileRead(PFFS.open_file(const_cast<char*>(MISSIONS)),box->mission,&ms);
      }else{
        box->gamestate = gs_showMission;
      }
    }
    if (button_delay > 10 && second_entry){
      second_entry = false;
      lcd.clear();
			stringToLCD_flash(F2("du kan slippe knappen nu :)"),false);
    }
  }

  /* LEFT button */
  button_delay = 0;
  first_entry = true;
  second_entry = true;
  while( digitalRead(BUTTON_LPUSHPIN) == LOW ){

    if( box->gamestate == gs_showMission && menu.id > 0 &&
        !menu.changed){
      menu.id--;
      menu.changed = true;
#ifdef DEBUG
      Serial.println(F("changed menu id--"));
#endif
    }

    button_delay++;
    delay(100); //debounce
    if (button_delay > 5 && first_entry){
      first_entry = false;
      menu.changed = true;
      box->timeout = !box->timeout;
      if(box->timeout)
					stringToLCD_flash(F2("Timeout enabled"),false);
      else
					stringToLCD_flash(F2("Timeout disabled"),false);
      delay(100);
    }
  }

  /* reset activity when looking at the menu */
  if(menu.changed){
			if (box->gamestate != gs_showMission)
					menu.changed = false;
			box->time = millis();
			box->first_time = true;
			box->second_time = true;
			box->turnoff = false;
			box->gpsPrintVarning = true;
  }
}
