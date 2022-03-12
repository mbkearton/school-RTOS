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
	uint8_t onboard_LED = 0; // shorthand for onboard LED referenced by taskHeartbeat
	
	QueueHandle_t qhLED = NULL;		// LED Blink Queue Handle
	QueueHandle_t qhUART_TX = NULL; // UART TX Queue Handle
	//QueueHandle_t qhUART_RX = NULL; // UART RX Queue Handle
	
	// Data for LED Blink task
	struct DataBlinkLED led_blink_data = {NULL};
	struct UARTData uart_data_tx = {NULL, NULL, NULL}; // data for UART TX task
	struct UARTData uart_data_rx = {NULL, NULL, NULL}; // data for UART RX task
	struct DataButtons buttons_data = {NULL};
	
	//=========================================================================
	// BLINKING TASKS -- creation of all tasks responsible for blinking 
	// an LED.
	//=========================================================================
	// Establish heartbeat task for onboard LED
	xTaskCreate(taskHeartbeat,
		"TaskHeartbeat",
		configMINIMAL_STACK_SIZE,
		(void *) &onboard_LED,			// reference to onboard LED shorthand
		1,
		NULL);						// no task handle necessary, should always run
		
	//=========================================================================
	//	QUEUES -- Creation of queues, and assignment of previously null 
	//	references to queue handles (used to pass messages between tasks).
	//=========================================================================
	// starting led queue
	qhLED = xQueueCreate(5, 1);
	// must ensure both data sets refer to their respective LED's queue
	led_blink_data.led_queue_handle = qhLED;
	
	// starting UART queues
	qhUART_TX = xQueueCreate(20, 50);
	qhUART_RX = xQueueCreate(20, 1);
	// TX Task only need TX Queue
	uart_data_tx.uart_tx_queue_handle = qhUART_TX;
	// RX Task needs TX, RX, and LED queues
	uart_data_rx.uart_tx_queue_handle = qhUART_TX;
	uart_data_rx.uart_rx_queue_handle = qhUART_RX;
	uart_data_rx.led_queue_handle = qhLED;
	// Buttons task only needs TX Queue
	buttons_data.uart_tx_queue_handle = qhUART_TX;

	//=========================================================================
	// SYSTEM CONTROL TASKS -- Creation of all main control tasks
	//=========================================================================
	xTaskCreate(taskUART_TX,
		"TaskUART_TX",
		configMINIMAL_STACK_SIZE,
		(void*) &uart_data_tx,
		3,
		NULL);
		
	//
	xTaskCreate(taskUART_RX,
		"TaskUART_RX",
		configMINIMAL_STACK_SIZE,
		(void*) &uart_data_rx,
		4,
		NULL);
	
	//
	xTaskCreate(taskBlinkLED,
		"TaskBlinkLED1",
		configMINIMAL_STACK_SIZE,
		(void *) &led_blink_data,
		2,
		NULL);
		
	//
	xTaskCreate(taskButtons,
		"TaskButtons",
		configMINIMAL_STACK_SIZE,
		(void *) &buttons_data,
		2,							
		&thButtons);
	
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
	   initializeButtonDriver();
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
