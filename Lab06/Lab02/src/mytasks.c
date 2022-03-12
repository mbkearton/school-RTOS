#include <asf.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>
#include <FreeRTOS_CLI.h>

#include "buttonDriver.h"
#include "ledDriver.h"
#include "uartdrv.h"
#include "mytasks.h"
#include "FreeRTOSConfig.h"

#define MAXDELAY	1000	// maximum LED blink delay, in ms
#define MINDELAY	200		// minimum LED blink delay, in ms
#define DELTADELAY	50		// the absolute value of the minimum change in delay

// helper function prototype
uint8_t getLED (char);

// toggles an LED on and off every 0.5 second
// heartbeat task created up to four times: 
//     *    1 onboard LED uncontrolled
//     *	3 extension board LEDs controlled by button functions
void 
taskHeartbeat (void* pvParameters)
{
	/* Block for 500ms. */
	const TickType_t xDelay = 500 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const uint8_t* ledNum = (uint8_t*) pvParameters;
	const uint8_t led = *ledNum;

	while(FOREVER)
	{	// actual task
		toggleLED(led);		// toggle LED state
		vTaskDelay( xDelay );	// then wait 0.5 
	}
}


// blinks target LED at the rate specified by the delay;
// delay is determined from message received from current system control task
// delay is bounded by min and max values
void 
taskBlinkLED (void* pvParameters)
{
	// casting sent parameter back to what it was originally
	struct DataBlinkLED data = *(struct DataBlinkLED*) pvParameters;

	int message;

	while(FOREVER)
	{
		if (uxQueueMessagesWaiting(data.led_queue_handle))
		{			
			// retrieving message from mailbox
			xQueueReceive(data.led_queue_handle, &message, 0);
			data.delay = message / portTICK_PERIOD_MS;
		}
		toggleLED(data.led_num);	// toggle LED state
		vTaskDelay(data.delay);
	}
}

//
void 
taskUART_TX (void* pvParameters)
{
	char					message[50];
	const struct UARTData	data = *(struct UARTData*) pvParameters;
	
	while (FOREVER)
	{	
		if (xQueueReceive(data.uart_tx_queue_handle, &message, portMAX_DELAY) == pdTRUE)
		{
			UARTPutStr(EDBG_UART, message, strlen(message));
		}
	}
}

//
void 
taskUART_RX (void* pvParameters)
{
	const struct UARTData data = *(struct UARTData*) pvParameters;
	
	BaseType_t more_data;
	
	static char rx_message = '\0';
	static char  tx_message[INPUT_LENGTH] = "";
	static char  output_message[OUTPUT_LENGTH] = "";
	
	static int8_t cli_cmd_index = 0;
	static char  cli_command[INPUT_LENGTH] = "";
	
	while (FOREVER)
	{
		if (xQueueReceive(data.uart_rx_queue_handle, &rx_message, portMAX_DELAY) == pdTRUE)
		{			
			if (rx_message == '\r')
			{
				cli_command[cli_cmd_index] = '\0';
				
				strcpy(tx_message, "\r\n\0");
				xQueueSendToBack(data.uart_tx_queue_handle, (void*) &tx_message, (TickType_t) 10);
				
				do 
				{
					more_data = FreeRTOS_CLIProcessCommand(cli_command, output_message, OUTPUT_LENGTH);
					
					while (snprintf(tx_message, INPUT_LENGTH - 1, output_message) > 0) 
					{
						strcpy(output_message, &output_message[strlen(tx_message)]);
						xQueueSendToBack(data.uart_tx_queue_handle, (void*) &tx_message, (TickType_t) 10);
						
					};
					
				} while (more_data != pdFALSE);
				
				cli_cmd_index = 0;
				memset(cli_command, 0x00, INPUT_LENGTH);
			}
			else if (rx_message == '\n')
			{
					// do nothing
			}
			else if (rx_message == '\b')
			{ 
				if (cli_cmd_index > 0)
				{
					--cli_cmd_index;
					cli_command[cli_cmd_index] = '\0';
				}
			}
			else
			{
				if (cli_cmd_index < INPUT_LENGTH)
				{
					cli_command[cli_cmd_index] = rx_message;
					++cli_cmd_index;
					
					strcpy(tx_message, &rx_message);
					xQueueSendToBack(data.uart_tx_queue_handle, (void*) &tx_message, (TickType_t) 10);
				}
			}
		}
	}
}

//
uint8_t
getLED (char led)
{
	uint8_t led_num = 0;
	
	// returns next LED given current, rotating through
	switch (led)
	{
		case '1':
			led_num = 1;
			break;
		case '2':
			led_num = 2;
			break;
		case '3':
			led_num = 3;
			break;
		default:
			break;
	}
	return led_num;
}
