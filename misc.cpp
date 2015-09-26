// -*- coding: utf-8 -*-


#include "misc.h"
#include "TheBox.h"
#include "pinSetup.h"

void backdoorListen(boxSettings *box, LiquidCrystal *lcd){
#ifdef DEBUG
  const unsigned int BDTIMER1 = 1000;
  const unsigned int BDTIMER2 = 5000;
  const unsigned long BDTIMER3 = 10000;
  const unsigned int DELAY_OPEN_CLOSE = 5000;
#else
  const unsigned int BDTIMER1 = 2000;
  const unsigned int BDTIMER2 = 15000;
  const unsigned long BDTIMER3 = 20000;
  const unsigned int DELAY_OPEN_CLOSE = 5000;
#endif
  //const float scalea = 0.99;
  //const float scaleb = 0.01;
  static float val = 1.0;
  static bool BDTIMER1_RUN = false;
  static bool BDTIMER2_RUN = false;
  static bool BDTIMER3_RUN = false;

  // listen for REED switch(backdoor)
  int sensorVal = digitalRead(BACKDOORPIN);
  //  Serial.print("sensorVal: ");
  //  Serial.println(sensorVal);

  val = sensorVal;
  // val = scalea*val + scaleb*sensorVal;
  // Serial.print("Val: ");
  // Serial.println(val);
  // start timer if backdoor-pin is activated
  // disable timer if val isn't under threshold
  if (val <= 0.1 && box->bdrunning == false) {
    box->bdtimer = millis();
    box->bdrunning = true;
#ifdef DEBUG
    Serial.println(F("backdoor timer started"));
#endif
  } else if (val >= 0.1) {
    box->bdrunning = false;
    BDTIMER1_RUN = false;
    BDTIMER2_RUN = false;
    BDTIMER3_RUN = false;
  }

  // test against timers
  if (box->bdrunning && (millis()-box->bdtimer >= BDTIMER1) && 
      !BDTIMER1_RUN) {
    // open box
#ifdef DEBUG
    Serial.println(F("First backdoor timer(open & close box)"));
#endif
    lcd->clear();
    lcd->print(F("First backdoor timer(open & close box)"));
    toogleBox(box, OPEN);
    smartDelay(DELAY_OPEN_CLOSE);
    toogleBox(box, CLOSE);
    BDTIMER1_RUN = true;

    box->time = millis();
    box->first_time = true;
    box->turnoff = false;
    box->gamestate = gs_running;
    box->gpsPrintVarning = true;
    /* reset activity on box so we get to see the missions again */
  }
  else if (box->bdrunning && (millis()-box->bdtimer >= BDTIMER2) && 
      !BDTIMER2_RUN){
#ifdef DEBUG
    Serial.println(F("Second backdoor timer(goto next mission)"));
#endif
    lcd->clear();
    lcd->print(F("Second backdoor timer(goto next mission)"));
    // goto next mission
    missionCompleted(box);
    BDTIMER2_RUN = true;
  }
  else if (box->bdrunning && (millis()-box->bdtimer >= BDTIMER3) && 
      !BDTIMER3_RUN) {
    // reset game
    box->gamestate = gs_reset;
    BDTIMER3_RUN = true;
#ifdef DEBUG
    Serial.println(F("Third backdoor timer(reset box)"));
#endif
  }
}
