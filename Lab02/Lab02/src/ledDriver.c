/*
 * ledDriver.c
 *
 * Created: 1/25/2022 2:00:08 AM
 *  Author: mbkea
 */ 
#include <asf.h>
#include <stdint.h>

#include "ledDriver.h"
#include "OITExpansionBoardDefines.h"

uint8_t SUCCESS = 1;
uint8_t FAILURE = 0;

uint8_t decLED [] =
{	// a "decoder" used to reference LED GPIO pins
	LED_0_PIN,
	EXT1_LED_1_PIN,
	EXT1_LED_2_PIN,
	EXT1_LED_3_PIN
};

/************************************************************
	LED Driver Functions	

*************************************************************/
void initializeLEDDriver(void)
{
	// dummy ; init completed in OITExpansionBoardInit()
}

// returns state of given LED
uint8_t readLED(uint8_t uiLedNum)
{
	// 0 == button NOT pressed
	// 1 == button pressed
	uint8_t ledState;
	
	ledState = ioport_get_pin_level(decLED[uiLedNum]);
	
	return ledState;
}

// sets given LED (arg1) to given value (arg2)
uint8_t setLED(uint8_t uiLedNum, uint8_t uiLedValue)
{
	ioport_set_pin_level(decLED[uiLedNum], uiLedValue);
	
	return SUCCESS; // dummy return
}

// 
uint8_t toggleLED(uint8_t uiLedNum)
{
	ioport_set_pin_level(decLED[uiLedNum], !ioport_get_pin_level(decLED[uiLedNum]));

	return SUCCESS; // dummy return
}
