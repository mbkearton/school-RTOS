/*
 * buttonDriver.c
 *
 * Created: 1/25/2022 2:48:36 PM
 *  Author: mbkea
 */ 
#include <asf.h>
#include <stdint.h>

#include "buttonDriver.h"
#include "OITExpansionBoardDefines.h"

uint8_t decSW [] =
{   // a "decoder" used to dereference button GPIO pins
	// from the "encoded" shorthand values 0-3
	EXT1_PUSH_BUTTON_6,
	EXT1_PUSH_BUTTON_7,
	EXT1_PUSH_BUTTON_8
};

void initializeButtonDriver(void)
{
	// for onboard button
	pio_configure(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_TYPE, PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR);
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID, PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR, button_handler);
	pio_handler_set_priority(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_IRQn, 5);
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);
	// for BTN6 == EXT_SW1
	pio_configure(extPIN_PUSHBUTTON_1_PIO, extPIN_PUSHBUTTON_1_TYPE, extPIN_PUSHBUTTON_1_MASK, extPIN_PUSHBUTTON_1_ATTR);
	pio_set_debounce_filter(extPIN_PUSHBUTTON_1_PIO, extPIN_PUSHBUTTON_1_MASK, 10);
	pio_handler_set(extPIN_PUSHBUTTON_1_PIO, extPIN_PUSHBUTTON_1_ID, extPIN_PUSHBUTTON_1_MASK, extPIN_PUSHBUTTON_1_ATTR, button_handler);
	pio_handler_set_priority(extPIN_PUSHBUTTON_1_PIO, extPIN_PUSHBUTTON_1_IRQn, 5);
	pio_enable_interrupt(extPIN_PUSHBUTTON_1_PIO, extPIN_PUSHBUTTON_1_MASK);
	// for BTN7 == EXT_SW2
	pio_configure(extPIN_PUSHBUTTON_2_PIO, extPIN_PUSHBUTTON_2_TYPE, extPIN_PUSHBUTTON_2_MASK, extPIN_PUSHBUTTON_2_ATTR);
	pio_set_debounce_filter(extPIN_PUSHBUTTON_2_PIO, extPIN_PUSHBUTTON_2_MASK, 10);
	pio_handler_set(extPIN_PUSHBUTTON_2_PIO, extPIN_PUSHBUTTON_2_ID, extPIN_PUSHBUTTON_2_MASK, extPIN_PUSHBUTTON_2_ATTR, button_handler);
	pio_handler_set_priority(extPIN_PUSHBUTTON_2_PIO, extPIN_PUSHBUTTON_2_IRQn, 5);
	pio_enable_interrupt(extPIN_PUSHBUTTON_2_PIO, extPIN_PUSHBUTTON_2_MASK);
	// for BTN8 == EXT_SW3
	pio_configure(extPIN_PUSHBUTTON_3_PIO, extPIN_PUSHBUTTON_3_TYPE, extPIN_PUSHBUTTON_3_MASK, extPIN_PUSHBUTTON_3_ATTR);
	pio_set_debounce_filter(extPIN_PUSHBUTTON_3_PIO, extPIN_PUSHBUTTON_3_MASK, 10);
	pio_handler_set(extPIN_PUSHBUTTON_3_PIO, extPIN_PUSHBUTTON_3_ID, extPIN_PUSHBUTTON_3_MASK, extPIN_PUSHBUTTON_3_ATTR, button_handler);
	pio_handler_set_priority(extPIN_PUSHBUTTON_3_PIO, extPIN_PUSHBUTTON_3_IRQn, 5);
	pio_enable_interrupt(extPIN_PUSHBUTTON_3_PIO, extPIN_PUSHBUTTON_3_MASK);
}

uint8_t readButton(uint8_t uiButNum)
{
	// 0 == button NOT pressed
	// 1 == button pressed
	uint8_t butState;
	
	// capturing the inverted pin level (buttons are pulled-up)
	butState = !ioport_get_pin_level(decSW[uiButNum]);

	return butState;
}

void button_handler(uint32_t id, uint32_t mask)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if((PIN_PUSHBUTTON_1_ID == id) && (PIN_PUSHBUTTON_1_MASK == mask))
	{
		// Set a global variable to signify which button (IE onboard in this case)

		// Then wake up the button Task to process.
		vTaskNotifyGiveFromISR(thButtons, &xHigherPriorityTaskWoken);
	}
	else if ((extPIN_PUSHBUTTON_1_ID == id) && (extPIN_PUSHBUTTON_1_MASK == mask))
	{
		button_pressed = EXT_SW1;
		vTaskNotifyGiveFromISR(thButtons, &xHigherPriorityTaskWoken);
	}
	else if ((extPIN_PUSHBUTTON_2_ID == id) && (extPIN_PUSHBUTTON_2_MASK == mask))
	{
		button_pressed = EXT_SW2;
		vTaskNotifyGiveFromISR(thButtons, &xHigherPriorityTaskWoken);
	}
	else if ((extPIN_PUSHBUTTON_3_ID == id) && (extPIN_PUSHBUTTON_3_MASK == mask))
	{
		button_pressed = EXT_SW3;
		vTaskNotifyGiveFromISR(thButtons, &xHigherPriorityTaskWoken);
	}
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
