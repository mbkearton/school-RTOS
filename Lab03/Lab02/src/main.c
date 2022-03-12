//system includes
#include <asf.h>

// FreeRTOS Includes
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>

// My Includes
#include "FreeRTOSConfig.h"
#include "mytasks.h"
#include "OITExpansionBoard.h"
#include "uartdrv.h"
#include "ledDriver.h"
#include "buttonDriver.h"

// Defines
#if( BOARD == SAM4E_XPLAINED_PRO )       // Used to place the heap
#define mainHEAP_START            _estack
#define mainRAM_LENGTH            0x00020000    /* 128 KB of internal SRAM. */
#define mainRAM_START             0x20000000    /* at 0x20000000 */
/* The SAM4E Xplained board has 2 banks of external SRAM, each one 512KB. */
#define mainEXTERNAL_SRAM_SIZE           ( 512ul * 1024ul )
#else
#error Define memory regions here.
#endif /* SAM4E_XPLAINED_PRO */

// Function Prototypes
static void prvMiscInitialisation( void );
static void prvInitialiseHeap( void );
void vApplicationMallocFailedHook( void );

// Used to place the heap
extern char _estack;

// helper functions
void startBlinkingLEDs(void);
void startQueues(void);

/************************************************************
	main: LED Control Application	

*************************************************************/
int main (void)
{
	// Initialize The Board
	prvMiscInitialisation();
	
	//=========================================================================
	// VARIABLES -- Declaration and initialization of task and queue handles 
	// referenced by system control tasks, LED blinking tasks, and UART task
	//=========================================================================
	// shorthand for onboard LED referenced by taskHeartbeat
	uint8_t onboard_LED = 0;
	
	// Task Handles
	TaskHandle_t thLED1SystemControl = NULL;	// System Control tasks poll all buttons
	TaskHandle_t thLED2SystemControl = NULL;	// and ensure only that only one LED is
	TaskHandle_t thLED3SystemControl = NULL;	// in control at a time
	
	// LED Queue Handles
	QueueHandle_t qhLED1 = NULL;	// queue handles referenced for task control
	QueueHandle_t qhLED2 = NULL;	// used to increase/decrease the delay for
	QueueHandle_t qhLED3 = NULL;	// each respective LED task
	// UART Queue Handle
	QueueHandle_t qhUART = NULL;
	
	// Data per each LED SystemControl task
	struct DataSystemControl led1_control_data = {NULL, NULL, NULL, "LED 2 IS NOW ACTIVE\n"};
	struct DataSystemControl led2_control_data = {NULL, NULL, NULL, "LED 3 IS NOW ACTIVE\n"};
	struct DataSystemControl led3_control_data = {NULL, NULL, NULL, "LED 1 IS NOW ACTIVE\n"};
	struct DataBlinkLED led1_blink_data = {1, NULL, NULL};
	struct DataBlinkLED led2_blink_data = {2, NULL, NULL};
	struct DataBlinkLED led3_blink_data = {3, NULL, NULL};
	// data for UART task
	struct UARTData uart_data = {NULL};
	
	//=========================================================================
	// BLINKING TASKS -- creation of all tasks responsible for blinking 
	// and LED.
	//=========================================================================
	// Establishe heartbeat task for onboard LED;
	// NOT controlled by taskSystemControl below
	xTaskCreate(taskHeartbeat,
		"TaskHeartbeat",
		configMINIMAL_STACK_SIZE,
		(void *) &onboard_LED,			// reference to onboard LED shorthand
		1,
		NULL);						// no task handle necessary, should always run
		
	// Begin blinking LED tasks 
	// tasks communicate with system control tasks found below
	// found below
	xTaskCreate(taskBlinkLED,
		"TaskBlinkLED1",
		configMINIMAL_STACK_SIZE,
		(void *) &led1_blink_data,
		1,
		NULL);
		
	xTaskCreate(taskBlinkLED,
		"TaskBlinkLED2",
		configMINIMAL_STACK_SIZE,
		(void *) &led2_blink_data,
		1,
		NULL);
		
	xTaskCreate(taskBlinkLED,
		"TaskBlinkLED1",
		configMINIMAL_STACK_SIZE,
		(void *) &led3_blink_data,
		1,
		NULL);
		
	//=========================================================================
	//	QUEUES -- Creation of queues, and assignment of previously null 
	//	references to queue handles (used to pass messages between tasks).
	//=========================================================================
	// starting led queues
	qhLED1 = xQueueCreate(5, sizeof(enum ledQueueMessage));
	qhLED2 = xQueueCreate(5, sizeof(enum ledQueueMessage));
	qhLED3 = xQueueCreate(5, sizeof(enum ledQueueMessage));
	// must ensure both data sets refer to their respective LED's queue
	led1_blink_data.led_queue_handle = qhLED1;
	led2_blink_data.led_queue_handle = qhLED2;
	led3_blink_data.led_queue_handle = qhLED3;
	led1_control_data.led_queue_handle = qhLED1;
	led2_control_data.led_queue_handle = qhLED2;
	led3_control_data.led_queue_handle = qhLED3;
	
	// starting UART queue
	qhUART = xQueueCreate(5, sizeof(char) * 50);
	uart_data.uart_queue_handle = qhUART;
	// must ensure both data sets refer to same UART queue
	led1_blink_data.uart_queue_handle = qhUART;
	led2_blink_data.uart_queue_handle = qhUART;
	led3_blink_data.uart_queue_handle = qhUART;
	led1_control_data.uart_queue_handle = qhUART;
	led2_control_data.uart_queue_handle = qhUART;
	led3_control_data.uart_queue_handle = qhUART;

	//=========================================================================
	// SYSTEM CONTROL TASKS -- Creation of all three main control tasks;
	// tasks two (2) and three (3) are suspended immediately following  their
	// creation to ensure only one (the first) system control task is
	// running upon startup.
	//=========================================================================
	// Creating UART task before control tasks so it is ready to respond to
	// system control tasks
	xTaskCreate(taskUART,
		"TaskUART",
		configMINIMAL_STACK_SIZE,
		(void*) &uart_data,
		1,
		NULL);
	
	// LED TWO (2) taskSystemControl created first
	xTaskCreate(taskSystemControl,
		"TaskSystemControlLED2",
		configMINIMAL_STACK_SIZE,
		(void*) &led2_control_data,
		1,
		&thLED2SystemControl);
	vTaskSuspend(thLED2SystemControl);	// suspend immediately, only ONE to run
		
	// LED THREE (3) taskSystemControl
	xTaskCreate(taskSystemControl,
		"TaskSystemControlLED3",
		configMINIMAL_STACK_SIZE,
		(void*) &led3_control_data,
		1,
		&thLED3SystemControl);
	vTaskSuspend(thLED3SystemControl); // suspend immediately, only ONE to run
		
	// LED ONE (1) taskSystemControl
	xTaskCreate(taskSystemControl,		// LED1 control task created last
		"TaskSystemControlLED1",		// and NOT SUSPENDED, to ensure
		configMINIMAL_STACK_SIZE,		// it is the first controlled LED
		(void*) &led1_control_data,
		1,
		&thLED1SystemControl);

	// must ensure control_data structs point to the new , non-null address
	// of the next LED's system control task
	led1_control_data.next_task_handle = thLED2SystemControl;
	led2_control_data.next_task_handle = thLED3SystemControl;
	led3_control_data.next_task_handle = thLED1SystemControl;
	
	// print currently controlled LED via UART
	UARTPutStr(EDBG_UART, led3_control_data.uart_next_led_message, 50);
	
	// Start The Scheduler
	vTaskStartScheduler();

	while(true)
	{
		
	}
}

static void prvInitialiseHeap( )
{
       uint32_t ulHeapSize;
       uint8_t *pucHeapStart, *pucHeapEnd;

       pucHeapStart = ( uint8_t * ) ( ( ( ( uint32_t ) &mainHEAP_START ) + 7 ) & ~0x07ul );
       pucHeapEnd = ( uint8_t * ) ( mainRAM_START + mainRAM_LENGTH );
       ulHeapSize = ( uint32_t ) ( ( uint32_t ) pucHeapEnd - ( uint32_t ) &mainHEAP_START );
       ulHeapSize &= ~0x07ul;
       ulHeapSize -= 1024;
       HeapRegion_t xHeapRegions[] = {
              { ( unsigned char *) pucHeapStart, ulHeapSize },
              { ( unsigned char *) SRAM_BASE_ADDRESS, mainEXTERNAL_SRAM_SIZE },
              { ( unsigned char *) SRAM_BASE_ADDRESS_2ND, mainEXTERNAL_SRAM_SIZE },
              { NULL, 0 }
       };

       vPortDefineHeapRegions( xHeapRegions );
}

static void prvMiscInitialisation( void )
{
       /* Initialize the SAM system */
       sysclk_init();
       board_init();
       prvInitialiseHeap();
       pmc_enable_periph_clk(ID_PIOA);
       pmc_enable_periph_clk(ID_PIOB);
	   OITExpansionBoardInit();
	   initUART(EDBG_UART);
}

void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
       volatile uint32_t ulBlockVariable = 0UL;

       /* Setting ulBlockVariable to a non-zero value in the debugger will allow
       this function to be exited. */
       taskDISABLE_INTERRUPTS();
       {
              while( ulBlockVariable == 0UL )
              {
                     __asm volatile( "NOP" );
              }
       }
       taskENABLE_INTERRUPTS();
}

void vApplicationMallocFailedHook( void )
{
       /* Called if a call to pvPortMalloc() fails because there is insufficient
       free memory available in the FreeRTOS heap.  pvPortMalloc() is called
       internally by FreeRTOS API functions that create tasks, queues, software
       timers, and semaphores.  The size of the FreeRTOS heap is set by the
       configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
       vAssertCalled( __FILE__, __LINE__ );
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
       ( void ) pcTaskName;
       ( void ) pxTask;

       /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
       taskDISABLE_INTERRUPTS();

       for( ;; );
}
