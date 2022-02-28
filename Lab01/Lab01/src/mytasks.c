/******************************************************************************
Author:			Michael Kearton
File Name:		mytasks.c
Date Created:	01/18/2022
******************************************************************************/
// Atmel-provided software framework containing APIs used to call various 
// system utilities including interrupts, power management, clock control, etc.
#include <asf.h>

// contains defines/definitions required to make use of scheduling API 
// used to manage tasks
#include "FreeRTOS.h"
#include "task.h"

// task definition(s) containing heartbeat task
#include "mytasks.h"
// configuration file used to manage and alter some aspects of scheduling
// behavior 
#include "FreeRTOSConfig.h"

// helper function to blink onboard LED
void vToggleLED (void);

// primary task which will function as the standard heartbeat referenced during
// program operation and debugging. 
// simply toggles the state of the onboard LED and then waits for half of a 
// second, repeating indefinitely.
 void myButtonTask( void * pvParameters )
 {
	 /* Block for 500ms. */
	 const TickType_t xDelay = 500 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	 while (FOREVER)
	 {
		 /* Simply toggle the LED every 500ms, blocking between each toggle. */
		 vToggleLED();
		 vTaskDelay( xDelay );
	 }
 }

// simple helper function toggling the voltage of the pin used to control the 
// onboard LED
void vToggleLED(void)
{
	ioport_set_pin_level(LED_0_PIN, !ioport_get_pin_level(LED_0_PIN));
}
