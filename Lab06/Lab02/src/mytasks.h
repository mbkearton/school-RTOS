/*
 * mytasks.h
 *
 * Created: 1/18/2022 11:36:05 AM
 *  Author: mbkea
 */ 

#ifndef MYTASKS_H_
#define MYTASKS_H_

#include "queue.h"
#include <stdint.h>
#include <FreeRTOS_CLI.h>

#define FOREVER				1
#define OUTPUT_LENGTH		configCOMMAND_INT_MAX_OUTPUT_SIZE
#define INPUT_LENGTH		50

enum ledQueueMessage
{	//
	DECREMENT = 1,
	INCREMENT = 2
};

enum ledSelect
{
	LED1 = 1, 
	LED2 = 2, 
	LED3 = 3	
};

struct DataBlinkLED
{	// struct to hold data used by taskBlinkLED
	QueueHandle_t	led_queue_handle;
	uint8_t			led_num;
	TickType_t		delay;
};

struct UARTData
{	// struct to hold data used by taskUART
	QueueHandle_t uart_tx_queue_handle;
	QueueHandle_t uart_rx_queue_handle;
	QueueHandle_t led_queue_handle;
};

void taskHeartbeat			(void*);
void taskBlinkLED			(void*);
void taskUART_TX			(void*);
void taskUART_RX			(void*);

#endif /* MYTASKS_H_ */
