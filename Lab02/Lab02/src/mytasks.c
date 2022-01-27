#include <asf.h>

#include <FreeRTOS.h>
#include <task.h>

#include "mytasks.h"
#include "buttonDriver.h"
#include "ledDriver.h"
#include "FreeRTOSConfig.h"

// helper functions
void processEXT_SW1 (void);
void processEXT_SW2 (void);
void processEXT_SW3 (void);

// task variables
uint8_t ledCount = 0; // number of active heartbeat tasks (not including main)
uint8_t suspendStatus = 0; // flag referenced by SW_8 (button 3)
TaskHandle_t thLED1Heartbeat = NULL; // task handles referenced for task control
TaskHandle_t thLED2Heartbeat = NULL; //	used to delete, suspend and resume respective
TaskHandle_t thLED3Heartbeat = NULL; //	heartbeat tasks
uint8_t led1 = 1; // shorthand for extension board LEDs
uint8_t led2 = 2;
uint8_t led3 = 3;

// toggles an LED on and off every 0.5 second
// heartbeat task created up to four times: 
//     *    1 onboard LED uncontrolled
//     *	3 extension board LEDs controlled by button functions
void taskHeartbeat( void * pvParameters )
{
	/* Block for 500ms. */
	const TickType_t xDelay = 500 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const uint8_t* ledNum = (uint8_t*) pvParameters;

	while(FOREVER)
	{	// actual task
		toggleLED(*ledNum);		// toggle LED state
		vTaskDelay( xDelay );	// then wait 0.5 

	}
}

//
//
//
void taskSystemControl(void * pvParameters )
{
	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;

	while (FOREVER)
	{	// actual task
		if (readButton(FIRST))
		{
			processEXT_SW1();
		}
		
		if (readButton(SECOND))
		{
			processEXT_SW2();
		}
		
		if (readButton(THIRD))
		{
			processEXT_SW3();
		}
		vTaskDelay(xDelay);
	}
}

// processes SW1 button press and takes appropriate action;
// I realize that "process____" probably isn't the best
// naming convention
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

//
//
//
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
	if (ledCount) ledCount -= 1;
}

//
//
//
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
