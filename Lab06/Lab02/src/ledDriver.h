/*
 * ledDriver.h
 *
 * Created: 1/25/2022 1:59:09 AM
 *  Author: mbkea
 */ 

#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include <stdint.h>

void initializeLEDDriver(void);	// dummy
uint8_t readLED(uint8_t uiLedNum);	// returns state of LED referenced by uiLedNum
uint8_t setLED(uint8_t uiLedNum, uint8_t uiLedValue);	// sets state of LED referenced by uiLedNum
uint8_t toggleLED(uint8_t uiLedNum);	// toggles state of LED referenced by uiLedNum

#endif /* LEDDRIVER_H_ */
