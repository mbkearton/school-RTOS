/******************************************************************************
Author:			Michael Kearton
File Name:		mytasks.c
Date Created:	01/18/2022
Modified:		01/25/2022 -- altered to facilitate Lab02
******************************************************************************/
#include <asf.h>

#include <FreeRTOS.h>
#include <task.h>

#include "mytasks.h"
#include "buttonDriver.h"
#include "ledDriver.h"
#include "FreeRTOSConfig.h"

/************************************************************
	Task Helper Function Declarations
*************************************************************/
// helper functions process each button pressed and respond appropriately
void processEXT_SW1 (void);
void processEXT_SW2 (void);
void processEXT_SW3 (void);

/************************************************************
	Task Variables
*************************************************************/
uint8_t ledCount = 0; // number of active heartbeat tasks (not including onboard LED)
uint8_t suspendStatus = 0; // flag referenced by SW_8 (button 3)
TaskHandle_t thLED1Heartbeat = NULL; // task handles referenced for task control
TaskHandle_t thLED2Heartbeat = NULL; //	used to delete, suspend and resume respective
TaskHandle_t thLED3Heartbeat = NULL; //	heartbeat tasks
uint8_t led1 = 1; // shorthand for extension board LEDs
uint8_t led2 = 2;
uint8_t led3 = 3;

/************************************************************
	Task Function Definitions
*************************************************************/
// (1) One argument: references LED to be toggled
// periodically toggles LED referenced by argument passed in, specifying which
// LED should be blinking. a unique heartbeat task is created for each LED.
void taskHeartbeat (void* pvParameters)
{
	/* Block for 500ms. */
	const TickType_t xDelay = 500 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const uint8_t* ledNum = (uint8_t*) pvParameters;

	while(FOREVER)
	{	// actual task
		toggleLED(*ledNum);		// toggle state of LED referenced by ledNum
		vTaskDelay( xDelay );	// then wait 0.5 
	}
}

// (0) Zero Arguments.
// polls the state of the three buttons and call appropriate helper function
// to respond with the necessary system call:
// 1.	add a blinking LED to the board (up to three)
// 2.	remove a blinking LED from the board
// 3.	suspend the state of all currently active blinking LEDs
void taskSystemControl (void* pvParameters)
{
	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;

	while (FOREVER)
	{	// actual task
		// NOTE: no debouncing implemented here
		if (readButton(FIRST))
		{
			processEXT_SW1();	// add LED
		}
		
		if (readButton(SECOND))
		{
			processEXT_SW2();	// remove LED
		}
		
		if (readButton(THIRD))
		{
			processEXT_SW3();	// suspend LEDs
		}
		vTaskDelay(xDelay);
	}
}

/************************************************************
	Task Helper Function Definitions
*************************************************************/
// processes SW1 button press and creates a heartbeat task for the next 
// LED, up to three. Only creates the task if it doesn't already exist.
// NOTE: I realize that "process____" probably isn't the best
// naming convention to be used here
void processEXT_SW1 (void)
{
	// only increase number of heart beat tasks if
	// there are FEWER than three tasks already created
	if (ledCount < 3) ledCount += 1;
	
	// determine if number of tasks to be be created has changed
	switch (ledCount)
	{
		case 1:
			// only create a task if the relevant
			// task handle has NOT yet been allocated
			if (!thLED1Heartbeat)
				xTaskCreate(taskHeartbeat,
					"LED1Heartbeat",
					configMINIMAL_STACK_SIZE,
					(void *) &led1,				// reference to LED1 shorthand
					1,
					&thLED1Heartbeat);			// reference to LED1 heartbeat task handle
			break;
		case 2:
			if (!thLED2Heartbeat)
				xTaskCreate(taskHeartbeat,
					"LED2Heartbeat",
					configMINIMAL_STACK_SIZE,
					(void *) &led2,				// reference to LED2 shorthand
					1,
					&thLED2Heartbeat);			// reference to LED2 heartbeat task handle
			break;
		case 3:
			if (!thLED3Heartbeat)
				xTaskCreate(taskHeartbeat,
					"LED3Heartbeat",
					configMINIMAL_STACK_SIZE,
					(void *) &led3,				// reference to LED3 shorthand
					1,
					&thLED3Heartbeat);			// reference to LED3 heartbeat task handle
			break;
		default:
			break;
	}
}

// process SW2 button press an deletes the highest numbered expansion board LED 
// heartbeat task, never deleting the onboard heartbeat task. must also set the
// relevant LED state to zero
void processEXT_SW2 (void)
{
	switch (ledCount)
	{
		case 3:
			setLED(ledCount, 0);
			vTaskDelete(thLED3Heartbeat);
			thLED3Heartbeat = NULL;
			break;
		case 2:
			setLED(ledCount, 0);
			vTaskDelete(thLED2Heartbeat);
			thLED2Heartbeat = NULL;
			break;
		case 1:
			setLED(ledCount, 0);
			vTaskDelete(thLED1Heartbeat);
			thLED1Heartbeat = NULL;
			break;
		default:
			break;
	}
	// decrement the LED count prior to exit, only if there is at least one LED
	if (ledCount) ledCount -= 1;
}

// suspends all heartbeat tasks -- except for the onboard heartbeat task.
// only suspends a task it actually exists.
// if tasks have already been suspended, then this function will resume
// all of those heartbeat tasks so long as they exist and the suspendStatus
// flag high.
void processEXT_SW3 (void)
{
	if (suspendStatus == 0)
	{
		if (thLED1Heartbeat)
			vTaskSuspend(thLED1Heartbeat);
		if (thLED2Heartbeat)
			vTaskSuspend(thLED2Heartbeat);
		if (thLED3Heartbeat)
			vTaskSuspend(thLED3Heartbeat);
		suspendStatus = 1;
	}
	else //  if suspendStatus == 1, then resume tasks and remove suspend state
	{
		if (thLED1Heartbeat)
			vTaskResume(thLED1Heartbeat);
		if (thLED2Heartbeat)
			vTaskResume(thLED2Heartbeat);
		if (thLED3Heartbeat)
			vTaskResume(thLED3Heartbeat);
		suspendStatus = 0;
	}

}
