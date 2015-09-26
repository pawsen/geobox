
#ifndef PINSETUP_H
#define PINSETUP_H


/*
 * LCD RS pin to A1
 * LCD Enable pin to A0
 * LCD Vo to digital pin 3 (no - resistor to vcc instead)
 * LCD D4 pin to digital pin 12
 * LCD D5 pin to digital pin 11
 * LCD D6 pin to digital pin 10
 * LCD D7 pin to digital pin 9
 * LCD R/W pin to ground
 */

/* const byte LCD_RS = A1, LCD_E  = A0, LCD_D4 = 12, LCD_D5 = 11, LCD_D6 = 10, LCD_D7 = 9; */
/* const byte GPS_RX_PIN = 7 , GPS_TX_PIN = 6; */

static const byte LCD_RS = A0, LCD_E  = A1, LCD_D4 = A2, LCD_D5 = A3, LCD_D6 = A4, LCD_D7 = A5;
/* White: RX, yellow: TX */
/* Not all pins on the Mega and Mega 2560 support change interrupts, so only the
 * following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8
 * (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
 * http://arduino.cc/en/Reference/SoftwareSerial */
/* BOARD_TAG is defined in the makefile */


static const byte GPS_RX_PIN = 4 , GPS_TX_PIN = 5;

/* #if BOARD_TAG == mega */
/* const byte GPS_RX_PIN = A9 , GPS_TX_PIN = A8; */
/* #else  /\* uno, duo etc. *\/ */
/* const byte GPS_RX_PIN = 4 , GPS_TX_PIN = 5; */
/* #endif */

static const byte SERVOPIN = 9;
static const byte POLULUPIN = 8;
// reed switch == back door
static const byte BACKDOORPIN = 7;


static const byte BUTTON_LPUSHPIN = 2;
static const byte BUTTON_RPUSHPIN = 3;

/* Make sure the default hardware chip-select pin is set as output
   Even if it's not used, otherwise the SD library will not work.
   CS pin: (10 on most Arduino boards, 53 on the Mega)
   */

/* The communication between the microcontroller and the SD card uses SPI, which
 * takes place on digital pins 11, 12, and 13 (on most Arduino boards) or 50,
 * 51, and 52 (Arduino Mega). Additionally, another pin must be used to select
 * the SD card. This can be the hardware SS pin - pin 10 (on most Arduino
 * boards) or pin 53 (on the Mega) - or another pin specified in the call to
 * SD.begin(). Note that even if you don't use the hardware SS pin, it must be
 * left as an output or the SD library won't work. */

/* These cannot be changed - and are not set! */
/* BOARD:                    UNO                    MEGA */
/* SS_PIN                     10                     53 */
/* MOSI_PIN                   11                     51 */
/* MISO_PIN                   12                     50 */
/* SCK_PIN                    13                     52 */

#endif
