/*
 * mytasks.h
 *
 * Created: 1/18/2022 11:36:05 AM
 *  Author: mbkea
 */ 

#ifndef MYTASKS_H_
#define MYTASKS_H_

#define FOREVER		1

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
};

struct UARTData
{	// struct to hold data used by taskUART
	QueueHandle_t uart_tx_queue_handle;
	QueueHandle_t uart_rx_queue_handle;
	QueueHandle_t led_queue_handle;
};

void taskHeartbeat			(void*);
void taskBlinkLED			(void*);
void taskBlinkLED_alt		(void*);
void taskToggleSelectLED	(void*);
void taskSystemControl		(void*);
void taskUART_TX			(void*);
void taskUART_RX			(void*);

#endif /* MYTASKS_H_ */
