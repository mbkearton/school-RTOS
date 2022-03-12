/******************************************************************************
Author:			Michael Kearton
File Name:		mytasks.h
Date Created:	01/18/2022
Modified:		01/25/2022 -- renamed myButtonTask, added taskSystemControl
******************************************************************************/

#ifndef MYTASKS_H_
#define MYTASKS_H_

#define FOREVER		1

/************************************************************
	Task Function Declarations
*************************************************************/
enum ledQueueMessage
{	// Message from queue will always be to  either shorten the period
	// to toggle or to lengthen the period to toggle.
	DECREMENT = 1,
	INCREMENT = 2
};

struct DataSystemControl
{	// struct to hold data used by taskSystemControl
	TaskHandle_t	next_task_handle;
	QueueHandle_t	led_queue_handle;
	QueueHandle_t	uart_queue_handle;
	char			uart_next_led_message[50];
};

struct DataBlinkLED
{	// struct to hold data used by taskBlinkLED
	uint8_t			led_number;
	QueueHandle_t	led_queue_handle;
	QueueHandle_t	uart_queue_handle;
};

struct UARTData
{	// struct to hold data used by taskUART
	QueueHandle_t uart_queue_handle;
};

/************************************************************
	Task Function Declarations
*************************************************************/
// polls buttons, managing the creation, deletion, and suspension of heartbeat
// tasks responsible for controlling expansion board LEDs
void taskSystemControl	(void *);	// takes struct data argument
void taskHeartbeat		(void *);	// toggles LED referenced by arg every 0.5s
void taskBlinkLED		(void *);   // 
void taskUART			(void *);

#endif /* MYTASKS_H_ */
