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
void taskBlinkLED( void * pvParameters )
{
	/* Block for 500ms. */
	TickType_t xDelay = 500;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	// casting sent parameter back to what it was originally
	const struct DataBlinkLED data = *(struct DataBlinkLED*) pvParameters;

	enum ledQueueMessage message = 0;	// preparing mailbox...

	while(FOREVER)
	{	// actual task
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
		vTaskDelay( xDelay / portTICK_PERIOD_MS);	// then wait 0.5
	}
}

//
//
//
void taskSystemControl(void * pvParameters )
{
	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
	const TickType_t xDebounceDelay = 15 / portTICK_PERIOD_MS;
	
	const struct DataSystemControl data = *(struct DataSystemControl*) pvParameters;

	char uart_message[50]; 
	enum ledQueueMessage message = 0;

	while (FOREVER)
	{
		if (readButton(FIRST))							// poll button
		{
			vTaskDelay(xDebounceDelay);					// wait a moment
			if (readButton(FIRST))						// take action if valid
			{
				message = DECREMENT;
				xQueueSendToBack(data.led_queue_handle, (void*) &message, (TickType_t) 10);
			}
		}												
		
		if (readButton(SECOND))
		{	// debounce
			vTaskDelay(xDebounceDelay);
			if (readButton(SECOND))
			{
				message = INCREMENT;
				xQueueSendToBack(data.led_queue_handle, (void*) &message, (TickType_t) 10);
			}
		}
		
		if (readButton(THIRD))
		{	// debounce
			vTaskDelay(xDebounceDelay);
			if (readButton(THIRD))	// if valid, change control to next LED taskSystemControl
			{	
				// sending message to UART indicating which LED will be in control following 
				// vTaskResume below
				strcpy(uart_message , data.uart_next_led_message);
				xQueueSendToBack(data.uart_queue_handle, (void*) &uart_message, (TickType_t) 10);
				
				// resuming next LED control task and immediately suspending self
				vTaskResume(data.next_task_handle);
				vTaskSuspend(NULL);
			}
		}
		vTaskDelay(xDelay);
	}
}

void taskUART(void* pvParameters)
{
	const struct UARTData data = *(struct UARTData*) pvParameters;
	
	char message[50];
	
	while (FOREVER)
	{	
		if (xQueueReceive(data.uart_queue_handle, &message, portMAX_DELAY ) == pdTRUE)
		{
			UARTPutStr(EDBG_UART, message, strlen(message));
		}
	}
}
