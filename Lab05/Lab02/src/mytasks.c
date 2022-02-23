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
enum ledSelect getNextLED (enum ledSelect);

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
	/* Block for 500ms. */
	TickType_t xDelay = 500;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const struct DataBlinkLED data = *(struct DataBlinkLED*) pvParameters;

	char uart_message[50] = "\0";
	enum ledQueueMessage message = 0;	// preparing mailbox...

	while(FOREVER)
	{
		//
		strcpy(uart_message, data.uart_led_message[0]);
		xQueueSendToBack(data.uart_queue_handle, (void*) &uart_message, (TickType_t) 10);
		
		if (uxQueueMessagesWaiting(data.led_queue_handle))
		{			
			// retrieving message from mailbox
			xQueueReceive(data.led_queue_handle, &message, 0);
			
			// setting delay value appropriately
			// depending on message and current delay value
			if (message == INCREMENT && xDelay < MAXDELAY)
			{
				xDelay += DELTADELAY;
			}
			else if (message == DECREMENT && xDelay > MINDELAY)
			{
				xDelay -= DELTADELAY;
			}
		}
		// toggle LED and wait for delay period
		toggleLED(data.led_number);				// toggle LED state
		
		//
		strcpy(uart_message, data.uart_led_message[1]);
		xQueueSendToBack(data.uart_queue_handle, (void*) &uart_message, (TickType_t) 10);
		
		//
		vTaskDelay( xDelay / portTICK_PERIOD_MS);	// then wait 0.5
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
		// 
		if (xQueueReceive(data.uart_queue_handle, &message, portMAX_DELAY) == pdTRUE)
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
	
	while (FOREVER)
	{
		// 
		if (xQueueReceive(data.uart_queue_handle, &rx_message, portMAX_DELAY) == pdTRUE)
		{
			switch (rx_message)
			{
				case '1':
				case '2':
				case '3':
				case 'u':
					xQueueSendToBack(data.led_queue_handle, (void*) &rx_message, (TickType_t) 10);
					break;
				default:
				
					break;
			}
			if
			xQueueSendToBack();
		}
	}
}

//
enum ledSelect
getNextLED (enum ledSelect current)
{
	enum ledSelect next = 0;
	
	// returns next LED given current, rotating through
	switch (current)
	{
		case LED1:
			next = LED2;
			break;
		case LED2:
			next = LED3;
			break;
		case LED3:
			next = LED1;
			break;
		default:
			break;
	}
	return next;
}
