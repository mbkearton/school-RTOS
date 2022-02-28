/******************************************************************************
Author:			Michael Kearton
File Name:		main.c
Date Created:	01/18/2022
******************************************************************************
Assignment:		Lab01

Overview:		This program starts up a task using the FreeRTOS API. Once the
			board has been configured following startup and the pins are set
			appropriately, the onboar LED will turn on, stay on for half a 
			second, and then turn off and stay off for half a second. This 
			behavior will repeat indefinitely, serving as a sort of "canary
			in the coal mine," effectively providing a simple form of visual
			debugging. This will be beneficial during later assignments 
			requiring more tasks and greater scheduling complexity.
			

Input:		N/A
Output:		Onboard LED will blink on and off, toggling every 0.5 seconds.
******************************************************************************/
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

int main (void)
{
	// Initialize The Board
	prvMiscInitialisation();
	
	// Create a Task to Handle Button Press and Light LED
	xTaskCreate(myButtonTask,					 // Function Called by task
		"My Button Task",                        // Task Name
		configMINIMAL_STACK_SIZE,                // Task Stack Size
		NULL,                                    // Any Paramaters Passed to Task
		1,                                       // Task Priority
		NULL);                                   // Place to store Task Handle
	
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

// initializes and configures various required
// hardware registers and pins
static void prvMiscInitialisation( void )
{
       /* Initialize the SAM system */
       sysclk_init();
       board_init();
       prvInitialiseHeap();
       pmc_enable_periph_clk(ID_PIOA);
       pmc_enable_periph_clk(ID_PIOB);
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
