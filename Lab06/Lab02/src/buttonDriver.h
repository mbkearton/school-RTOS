/*
 * buttonDriver.h
 *
 * Created: 1/25/2022 2:48:21 PM
 *  Author: mbkea
 */ 

#ifndef BUTTONDRIVER_H_
#define BUTTONDRIVER_H_

// for BTN6 == EXT_SW1
#define extPIN_PUSHBUTTON_1_MASK  PIO_PA16
#define extPIN_PUSHBUTTON_1_PIO   PIOA
#define extPIN_PUSHBUTTON_1_ID    ID_PIOA
#define extPIN_PUSHBUTTON_1_TYPE  PIO_INPUT
#define extPIN_PUSHBUTTON_1_ATTR  (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)
#define extPIN_PUSHBUTTON_1_IRQn  PIOA_IRQn

// for BTN7 == EXT_SW2
#define extPIN_PUSHBUTTON_2_MASK  PIO_PA11
#define extPIN_PUSHBUTTON_2_PIO   PIOA
#define extPIN_PUSHBUTTON_2_ID    ID_PIOA
#define extPIN_PUSHBUTTON_2_TYPE  PIO_INPUT
#define extPIN_PUSHBUTTON_2_ATTR  (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)
#define extPIN_PUSHBUTTON_2_IRQn  PIOA_IRQn

// for BTN8 == EXT_SW3
#define extPIN_PUSHBUTTON_3_MASK  PIO_PD25
#define extPIN_PUSHBUTTON_3_PIO   PIOD
#define extPIN_PUSHBUTTON_3_ID    ID_PIOD
#define extPIN_PUSHBUTTON_3_TYPE  PIO_INPUT
#define extPIN_PUSHBUTTON_3_ATTR  (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)
#define extPIN_PUSHBUTTON_3_IRQn  PIOD_IRQn

#include <FreeRTOS.h>
#include <task.h>

//
TaskHandle_t thButtons;

//
enum Button_Pressed 
{
	EXT_SW1,
	EXT_SW2,
	EXT_SW3
} button_pressed;

void		initializeButtonDriver (void);
uint8_t		readButton (uint8_t uiButNum);
void		button_handler (uint32_t id, uint32_t mask);

#endif /* BUTTONDRIVER_H_ */