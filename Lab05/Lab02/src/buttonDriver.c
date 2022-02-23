/*
 * buttonDriver.c
 *
 * Created: 1/25/2022 2:48:36 PM
 *  Author: mbkea
 */ 
#include <asf.h>
#include <stdint.h>

#include "buttonDriver.h"
#include "OITExpansionBoardDefines.h"

uint8_t decSW [] =
{   // a "decoder" used to dereference button GPIO pins
	// from the "encoded" shorthand values 0-3
	EXT1_PUSH_BUTTON_6,
	EXT1_PUSH_BUTTON_7,
	EXT1_PUSH_BUTTON_8
};

void initializeButtonDriver(void)
{
	// dummy ; init completed in OITExpansionBoardInit()
}

uint8_t readButton(uint8_t uiButNum)
{
	// 0 == button NOT pressed
	// 1 == button pressed
	uint8_t butState;
	
	// capturing the inverted pin level (buttons are pulled-up)
	butState = !ioport_get_pin_level(decSW[uiButNum]);

	return butState;
}
