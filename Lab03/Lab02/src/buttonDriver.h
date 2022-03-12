/*
 * buttonDriver.h
 *
 * Created: 1/25/2022 2:48:21 PM
 *  Author: mbkea
 */ 

#ifndef BUTTONDRIVER_H_
#define BUTTONDRIVER_H_

#define FIRST	0
#define SECOND	1
#define THIRD	2

void	initializeButtonDriver (void);	  // dummy
uint8_t readButton			   (uint8_t); // reads the state of the button referenced by arg

#endif /* BUTTONDRIVER_H_ */