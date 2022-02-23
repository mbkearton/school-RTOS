#include <asf.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>

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
	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const struct DataBlinkLED data = *(struct DataBlinkLED*) pvParameters;

	char message = '\0';
	uint8_t led_num;

	while(FOREVER)
	{
		if (uxQueueMessagesWaiting(data.led_queue_handle))
		{			
			// retrieving message from mailbox
			xQueueReceive(data.led_queue_handle, &message, 0);
			led_num = getLED(message);
			toggleLED(led_num);				// toggle LED state
		}
		vTaskDelay(xDelay);
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
	
	char rx_message;
	char tx_message[50];
	
	while (FOREVER)
	{
		if (xQueueReceive(data.uart_rx_queue_handle, &rx_message, portMAX_DELAY) == pdTRUE)
		{
			switch (rx_message)
			{
				case '\r':
				tx_message[0] = '\r';
				tx_message[1] = '\n';
				tx_message[2] = '\0';
				break;
				case '1':
				case '2':
				case '3':
				tx_message[0] = rx_message;
				tx_message[1] = '\0';
				xQueueSendToBack(data.led_queue_handle, (void*) &rx_message, (TickType_t) 10);
				break;
				case 'u':
				strcpy(tx_message, "Michael Kearton");
				break;
				default:
				tx_message[0] = rx_message;
				tx_message[1] = '\0';
				break;
			}
			
			xQueueSendToBack(data.uart_tx_queue_handle, (void*) &tx_message, (TickType_t) 10);
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
