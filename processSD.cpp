/* -*- coding: utf-8 -*- */

/*
  (setq-default indent-tabs-mode t)
  (setq-default tab-width 4) ; Assuming you want your tabs to be four spaces wide
  (defvaralias 'c-basic-offset 'tab-width)
  (setq-default c-basic-offset 'tab-width)
*/

#include "processSD.h"


byte rx()
{
	SPDR = 0xFF;
	loop_until_bit_is_set(SPSR, SPIF);
    return SPDR;
}

void tx(byte d)
{
	SPDR = d;
	loop_until_bit_is_set(SPSR, SPIF);
}

void spi_init()
{
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(12, INPUT);
	pinMode(13, OUTPUT);
	// Master mode, SPI enable, clock speed MCU_XTAL/4
	SPCR = _BV(MSTR) | _BV(SPE);
}


void settingsRead(int err, const char *file, struct boxSettings *box){

	if (err != 0){
			Serial.print(F("Fejlkode ")), Serial.print(err),
					Serial.print(F(" ved åbning af fil ")), Serial.println(file);
		return;
	}

	char str[TEXTLENGTH];
	const int READ_STRING_SIZE = sizeof(str) - 1;
	char par[15];
	int readIndex = 0;
	char c;
	int bytes_read;
	bool comment = false;

	Serial.print(("Free RAM = "));
	Serial.println(freeMemory(), DEC);

	do
	{
		/* bytes_read is a pointer to a integer which will contain the
		 * number of bytes actually read. Eq if bytes_read < 1 ==
		 * EOF */
		PFFS.read_file(&c, 1, &bytes_read);

		if (readIndex < READ_STRING_SIZE) {
			str[readIndex++] = c;
		}else{
			// reset the buffer if the incoming is too long
			memset(&str[0], 0, sizeof(str));
			readIndex = 0;
		}
		if (c == '='){
			/* The parameter */

			// Terminate the read string
			str[readIndex-1] = '\0';
			// Prepare for next reading
			readIndex = 0;

			// convert str to all uppercase
			for (int i = 0; i < strlen(str); i++){
				str[i] = toupper(str[i]);
			}
			strcpy(par,str);
			memset(&str[0], 0, sizeof(str));
		}else if (c == '/' && str[readIndex - 1] == '/' ){
			/* comment - skip line */
			comment = true;
		}else if (c == '\n') {
			/* The argument */

			str[readIndex-1] = '\0';
			readIndex = 0;
			if (comment){
				comment = false;
				continue;
			}

			if (strcmp(par,("SERIAL"))==0 ){
				box->serial = atol(str);
			}else if (strcmp(par,("START_FROM"))==0 ){
				box->mission = atoi(str);
			}else if (strcmp(par,("DELAY"))==0 ){
				box->delay = atoi(str);
			}else if (strcmp(par,("INIT"))==0 ){
				strcpy(box->init,str);
			}else if (strcmp(par,("WELCOME"))==0 ){
				strcpy(box->welcome,str);
			}else if (strcmp(par,("COMPLETED"))==0 ){
					strcpy(box->completed,str);
			}else if (strcmp(par,("FINISH"))==0 ){
					strcpy(box->finish,str);
			}else if (strcmp(par,("NMISSION"))==0 ){
					box->nMission = atoi(str);
			}else if (strcmp(par,("TIMEZONE"))==0 ){
					box->timezone = atoi(str);
			}else{
				Serial.print(F("Unknown setting: ")), Serial.println(par);
			}
			memset(&str[0], 0, sizeof(str));
			memset(&par[0], 0, sizeof(par));

		}
    }while (bytes_read == 1);

}


void fileRead(int err, int nMission, struct missionStruct *ms)
{
	if (err == 0)
	{

		char str[TEXTLENGTH];
		const int READ_STRING_SIZE = sizeof(str) - 1;
		int readIndex = 0, readPart = 0, nLine = 1;
		char c;
		int bytes_read;
		bool comment = false;

		Serial.print(("Free RAM = "));
		Serial.println(freeMemory(), DEC);

		do
		{
			/* bytes_read is a pointer to a integer which will contain the number of
			 * bytes actually read. Eq if bytes_read < 1 == EOF */
			PFFS.read_file(&c, 1, &bytes_read);


			if (readIndex < READ_STRING_SIZE) {
				str[readIndex++] = c;
			}else{
				// reset the buffer if the incoming is too long
				memset(&str[0], 0, sizeof(str));
				readIndex = 0;
			}

			if (c == '/' && str[readIndex - 2] == '/' ){
				/* comment - skip line */
				comment = true;

				/* FOR MISSIONS */
			}else if(c == '\n' || c == '^') {
				// end of line/mission
				readPart = 0;
				if (comment){
					comment = false;
					continue;
				}

				nLine++;
				if (nLine > nMission)
					break;
				// Prepare for next reading
				memset(&str[0], 0, sizeof(str));
				readIndex = 0;
			}else if (c == '|' && nLine == nMission) {
				// Terminate the read string
				str[readIndex-1] = '\0';

				if (strlen(str) != 0){
					// value given
					processChunk(str, readPart, ms);
				}
				// Prepare for next reading
				memset(&str[0], 0, sizeof(str));
				readIndex = 0;
				// keep track of which part is being read
				readPart++;
			}
		}while (bytes_read == 1);


	}
	else
	{
		Serial.print("Error code "); Serial.print(err); Serial.println(" while opening file");
	}
}



void processChunk(char * str, byte readPart, struct missionStruct *ms){

	// process data
	switch(readPart){
	case 1: /* mission text  */
		// strcpy adds '\0'
		strcpy(ms->text, str);
		break;
	case 2: /* mission completed text  */
		strcpy(ms->completed, str);
		break;
	case 3: /* latitude of mission  */
			ms->flat = (float)atof(str);
		break;
	case 4: /* longitude of mission  */
		ms->flon = (float)atof(str);
		break;
	case 5: /* threshold of mission  */
		ms->treshold = atoi(str);
		break;
	case 6: /* gps required  */
			ms->requirePos = (atoi(str) != 0);
			break;
	case 7: /* time */
			Serial.println(str);
		tmElements_t tm;
		char * pch;
		pch = strtok (str,"/.:");
		tm.Day = atoi(pch);//strtoul(pch, NULL, 0);
		pch = strtok (NULL,"/.:");
		tm.Month = atoi(pch);//strtoul(pch, NULL, 0);
		pch = strtok (NULL,"/.:");
		tm.Year = strtoul(pch, NULL, 0) - 1970;
		tm.Year = atol(pch) - 1970;

		tm.Second = 0;
		pch = strtok (NULL,"/.:");
		tm.Hour = atoi(pch);//strtoul(pch, NULL, 0);;
		pch = strtok (NULL,"/.:");
		tm.Minute = atoi(pch);//strtoul(pch, NULL, 0);;
		tm.Wday = 0;

		ms->time = makeTime(tm);
		break;
	case 8: /* time required  */
			ms->requireTime = (atoi(str) != 0);
			break;
	case 9: /* accept required (reed switch) */
			ms->requireAccept = (atoi(str) != 0);
			break;
	}
}
