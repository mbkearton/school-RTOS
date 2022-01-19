#include <asf.h>

#include "FreeRTOS.h"
#include "task.h"

#include "mytasks.h"
#include "FreeRTOSConfig.h"

 void myButtonTask( void * pvParameters )
 {
	 /* Block for 500ms. */
	 const TickType_t xDelay = 500 / portTICK_PERIOD_MS;	// SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ = 4MHz

	 for( ;; )
	 {
		 /* Simply toggle the LED every 500ms, blocking between each toggle. */
		 vToggleLED();
		 vTaskDelay( xDelay );
	 }
 }

void vToggleLED(void)
{
	ioport_set_pin_level(LED_0_PIN, !ioport_get_pin_level(LED_0_PIN));
}


//void myButtonTask(void * pvParamaters)
//{
	//while (true)
	//{
		///* Is button pressed? */
		//if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE)
		//{
			///* Yes, so turn LED on. */
			//ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
		//}
		//else
		//{
			///* No, so turn LED off. */
			//ioport_set_pin_level(LED_0_PIN, !LED_0_ACTIVE);
		//}
	//}
//}