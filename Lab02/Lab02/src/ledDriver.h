/*
 * ledDriver.h
 *
 * Created: 1/25/2022 1:59:09 AM
 *  Author: mbkea
 */ 

#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

void	initializeLEDDriver (void);				// dummy
uint8_t	readLED				(uint8_t);			// returns state of LED referenced by arg
uint8_t	setLED				(uint8_t, uint8_t);	// sets state (arg 2) of LED (arg 1)
uint8_t	toggleLED			(uint8_t);			// toggles state of LED referenced by arg

#endif /* LEDDRIVER_H_ */
