/*
 * commands.c
 *
 * Created: 3/11/2022 12:20:27 AM
 *  Author: mbkea
 */ 
#include "commands.h"
#include "ledDriver.h"

#include <stdio.h>

/*
	The parameters will be returned in ASCII and must be converted to the
	appropriate type by your command processing function
		Parameters extracted from command with:
		 -- FreeRTOS_CLIGetParameter()
		Converted to appropriate type with:
		 -- atoi()
*/

/*	
	Four commands to be processed, with variable number of parameters:
	*	task-stats		(void)
	*	start-led		(1) LED_int, (2) DELAY_ms; else print "LED already started"
	*	stop-led		(1) LED_int; else print "LED already stopped"
	*	change-LED		(1) LED_int, (2) DELAY_ms; else print "LED is stopped"
*/

static BaseType_t prvTaskStatsCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
static BaseType_t prvStartLEDCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
static BaseType_t prvStopLEDCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
static BaseType_t prvChangeLEDCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);

const char names_LEDs[3][14] =
{
	"TaskBlinkLED1",
	"TaskBlinkLED2",
	"TaskBlinkLED3"
};

//=============================================================================
// task-stats
// no parameters	
//=============================================================================
/* 
 * Note: To use the VTaskList Command you must modify the FreeRTOSConfig.h
 * to change: 
 * #define configUSE_TRACE_FACILITY                1
 * #define configUSE_STATS_FORMATTING_FUNCTIONS    1 
 */
static const char taskListHdr[] = "Name\t\t\tStat\tPri\tS/Space\tTCB\r\n\0";

static const xCommandLineInput xTaskStatsCommand = {"task-stats\0",
            "task-stats: Displays a table of task state information\r\n\0",
            &prvTaskStatsCommand,
            0};

			
// Note: You should check to make sure everything fits in the buffer you are 
// sending back. This example does not...
static BaseType_t prvTaskStatsCommand(char* pcWriteBuffer, 
                                  size_t xWriteBufferLen,
                                  const char* pcCommandString)
{
    snprintf(pcWriteBuffer, xWriteBufferLen, taskListHdr);
    
	pcWriteBuffer += strlen(taskListHdr);

    vTaskList(pcWriteBuffer);
	
	return pdFALSE;
}

//=============================================================================
// start-led
// two parameters: (1) LED_int, (2) DELAY_ms
//=============================================================================
//
static const xCommandLineInput xStartLEDCommand = 
{
	"start-led\0",
	"start-led LED_int DELAY_ms: Starts blinking LED specified by LED_int for DELAY_ms\r\n\0",
	&prvStartLEDCommand,
	2
};

//
static BaseType_t prvStartLEDCommand(char* pcWriteBuffer,
								  size_t xWriteBufferLen,
								  const char* pcCommandString)
{
	char *LED_int, *DELAY_ms;
	BaseType_t strlenLED_int, strlenDELAY_ms;
	
	LED_int = FreeRTOS_CLIGetParameter(pcCommandString, 1, &strlenLED_int);
	DELAY_ms = FreeRTOS_CLIGetParameter(pcCommandString, 2, &strlenDELAY_ms);
	
	LED_int[strlenLED_int] = '\0';
	DELAY_ms[strlenDELAY_ms] = '\0';
	
	//
	int LED_num = atoi(LED_int);
	if (thLEDs[LED_num - 1] != NULL)
	{	// if this numbered LED already has an associated task handle, 
		// then alert the user and return
		snprintf(pcWriteBuffer, xWriteBufferLen, "LED already started\n\0");
	}
	else // this LED blinking task has yet to be created
	{
		int DELAY_num = atoi(DELAY_ms);
		data_LEDs[LED_num - 1].delay = DELAY_num / portTICK_PERIOD_MS;
			
		xTaskCreate(taskBlinkLED,
					names_LEDs[LED_num - 1],
					configMINIMAL_STACK_SIZE,
					(void *) &data_LEDs[LED_num - 1],
					2,
					&thLEDs[LED_num - 1]);
	}
	
	return pdFALSE;
}

//=============================================================================
// stop-led
// one parameter: (1) LED_int
//=============================================================================
//
static const xCommandLineInput xStopLEDCommand =
{
	"stop-led\0",
	"stop-led LED_int: Stops blinking LED specified by LED_int\r\n\0",
	&prvStopLEDCommand,
	1
};

//
static BaseType_t prvStopLEDCommand(char* pcWriteBuffer,
								size_t xWriteBufferLen,
								const char* pcCommandString)
{
	char* LED_int;
	BaseType_t strlenLED_int;
	
	LED_int = FreeRTOS_CLIGetParameter(pcCommandString, 1, &strlenLED_int);
	LED_int[strlenLED_int] = '\0';
	
	int LED_num = atoi(LED_int);
	if (thLEDs[LED_num - 1] == NULL)
	{	// if this numbered LED does NOT have an associated task handle, 
		// then it does not need to be stopped.
		strcpy(pcWriteBuffer, "LED already stopped\0");
	}
	else // this LED blinking task needs to be deleted
	{	
		setLED(LED_num, 0); // first, set the LED state to zero
		
		// once LED is no longer lit up, delete the task
		vTaskDelete(thLEDs[LED_num - 1]);
		thLEDs[LED_num - 1] = NULL;
	}
	
	return pdFALSE;
}

//=============================================================================
// change-led
// one parameters: (1) LED_int, (2) DELAY_ms
//=============================================================================
//
static const xCommandLineInput xChangeLEDCommand =
{
	"change-led\0",
	"change-led LED_int DELAY_ms: Changes delay used by LED specified by LED_int to DELAY_ms\r\n\0",
	&prvChangeLEDCommand,
	2
};

//
static BaseType_t prvChangeLEDCommand(char* pcWriteBuffer,
								size_t xWriteBufferLen,
								const char* pcCommandString)
{
	char *LED_int, *DELAY_ms;
	BaseType_t strlenLED_int, strlenDELAY_ms;
	
	LED_int = FreeRTOS_CLIGetParameter(pcCommandString, 1, &strlenLED_int);
	DELAY_ms = FreeRTOS_CLIGetParameter(pcCommandString, 2, &strlenDELAY_ms);
	
	LED_int[strlenLED_int] = '\0';
	DELAY_ms[strlenDELAY_ms] = '\0';
	
	//
	int LED_num = atoi(LED_int);
	if (thLEDs[LED_num - 1] == NULL)
	{	// if this numbered LED does NOT have an associated task handle,
		// then there is no delay to be changed.
		strcpy(pcWriteBuffer, "LED is stopped\0");
	}
	else // LED blinking task needs to be changed
	{
		int  DELAY_num = atoi(DELAY_ms);
		
		xQueueSendToBack(
			data_LEDs[LED_num - 1].led_queue_handle, 
			(void*) &DELAY_num, 
			(TickType_t) 10
		);
	}
	
	return pdFALSE;
}

void register_commands(void)
{
	FreeRTOS_CLIRegisterCommand(&xTaskStatsCommand);
	FreeRTOS_CLIRegisterCommand(&xStartLEDCommand);
	FreeRTOS_CLIRegisterCommand(&xStopLEDCommand);
	FreeRTOS_CLIRegisterCommand(&xChangeLEDCommand);
}
