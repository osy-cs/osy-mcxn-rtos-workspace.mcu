// **************************************************************************
//
//                  FreeRTOS demo program for OSY labs
//
// Subject:         Operating systems
// Topic:           FreeRTOS
// Author:          Petr Olivka, petr.olivka@vsb.cz, 11/2026
// Organization:    Department of Computer Science, FEECS,
//                  VSB-Technical University of Ostrava, CZ
//
// File:            Semaphores and Interrupt
//
// **************************************************************************

/**
 * @file    main-task.cpp
 * @brief   Application entry point.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <functional>
#include <algorithm>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#include "mcxn-kit.h"
#include "gpio_interrupts.h"

// **************************************************************************
//! System initialization. Do not modify it!!!
void _mcu_initialization() __attribute__(( constructor( 0x100 ) ));

void _mcu_initialization()
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();
    CLOCK_EnableClock( kCLOCK_Gpio0 );
    CLOCK_EnableClock( kCLOCK_Gpio1 );
    CLOCK_EnableClock( kCLOCK_Gpio2 );
    CLOCK_EnableClock( kCLOCK_Gpio3 );
    CLOCK_EnableClock( kCLOCK_Gpio4 );
}

// **************************************************************************
// MCXN-Kit Pin Assignments Section
//
// Button pin assignments. See MCXN-Kit documentation (APPS)
//
// Port Pin     Pin Name
// ---------------------
// P3   18      P3_18
// P3   19      P3_19
// P3   20      P3_20
// P3   21      P3_21
//
// Kit LED pin assignments:
//
// Port Pin     Pin Name
// ---------------------
// P3   16      P3_16
// P3   17      P3_17
//
// Red LED pin assignments:
//
// Port Pin     Pin Name
// ---------------------
// P4    0      P4_00
// P4    1      P4_01
// P4    2      P4_02
// P4    3      P4_00
// P4   12      P4_12
// P4   13      P4_13
// P4   16      P4_16
// P4   20      P4_20
//
// RGB LED pin assignments:
//
// Port Pin     Pin Name
// ---------------------_
// P0   14      P0_14    \.
// P0   15      P0_15     } RGB0
// P0   22      P0_22   _/
// P0   24      P0_24    \.
// P0   25      P0_25     } RGB1
// P0   26      P0_26   _/
// P0   28      P0_28    \.
// P0   29      P0_29     } RGB2
// P0   30      P0_30   _/


// **************************************************************************
//! Global Data Section

#define KIT_LED_NUM                 2
#define RED_LED_NUM                 8
#define RGB_LED_NUM                 9

//! Buttons on MCXN-KIT - instances of class DigitalIn
DigitalIn g_but_P3_18( P3_18 );
DigitalIn g_but_P3_19( P3_19 );
DigitalIn g_but_P3_20( P3_20 );
DigitalIn g_but_P3_21( P3_21 );

// all P3_xx LEDs in array
DigitalOut g_kit_led[ KIT_LED_NUM ] =
{
        P3_16,
        P3_17,
};

// all P4_xx LEDs in array
DigitalOut g_red_led[ RED_LED_NUM ] =
{
        P4_00,
        P4_01,
        P4_02,
        P4_03,
        P4_12,
        P4_13,
        P4_16,
        P4_20,
};

// **************************************************************************
// FreeRTOS Section

#define LOW_TASK_PRIORITY       ( configMAX_PRIORITIES - 4 )
#define NORMAL_TASK_PRIORITY    ( configMAX_PRIORITIES - 3 )
#define HIGH_TASK_PRIORITY      ( configMAX_PRIORITIES - 1 )

#define TASK_NAME_RED_LED       "red led"
#define TASK_NAME_LEFT_BUTTON   "left button"
#define TASK_NAME_RIGHT_BUTTON  "right button"

// global variables
SemaphoreHandle_t g_sem_left_button;
SemaphoreHandle_t g_sem_right_button;

int32_t g_red_led_index = 0;


// Tasks

// reset red led index
void task_left_button( void *t_arg )
{
    while ( 1 )
    {
        xSemaphoreTake( g_sem_left_button, portMAX_DELAY );

        g_red_led_index = 0;

        vTaskDelay( 1 );
    }
}


// turn on LED at g_red_led index position
void task_show_red_led( void *t_arg )
{
    int32_t l_last_index = -1;

    while ( 1 )
    {
        // check range of g_red_led_index
        if ( g_red_led_index >= RED_LED_NUM )
        {
            g_red_led_index = RED_LED_NUM - 1;
        }

        if ( g_red_led_index < 0 )
        {
            g_red_led_index = 0;
        }

        // position changed?
        if ( l_last_index != g_red_led_index )
        {
            if ( l_last_index < 0 )
            {
                l_last_index = g_red_led_index;
            }

            // turn LED off
            g_red_led[ l_last_index ].write( 0 );
            // turn LED on
            g_red_led[ g_red_led_index ].write( 1 );

            l_last_index = g_red_led_index;
        }

        vTaskDelay( 1 );
    }
}


// Start application
int main( void )
{
    // Initialize GPIO for interrupts
    InitGPIOInterrupts();

    PRINTF( "FreeRTOS demo program uses interrupts (ISR) and semaphores.\r\n" );
    PRINTF( "GPIO interrupts are enabled.\r\n" );
    PRINTF( "Interrupts are generated by buttons, see 'gpio_interrupts.cpp' file.\r\n" );
    PRINTF( "Pressing the right button will move red LED to right.\r\n" );
    PRINTF( "Pressing the left button will move red LED back to left side.\r\n" );

    // Create tasks
    if ( xTaskCreate(
            task_left_button,
            TASK_NAME_LEFT_BUTTON,
            configMINIMAL_STACK_SIZE + 100,
            NULL,
            NORMAL_TASK_PRIORITY,
            NULL) != pdPASS )
    {
        PRINTF( "Unable to create task '%s'!\r\n", TASK_NAME_LEFT_BUTTON );
    }

    if ( xTaskCreate( task_show_red_led, TASK_NAME_RED_LED, configMINIMAL_STACK_SIZE + 100, NULL, HIGH_TASK_PRIORITY, NULL) != pdPASS )
    {
        PRINTF( "Unable to create task '%s'!\r\n", TASK_NAME_RED_LED );
    }

    vTaskStartScheduler();

    while ( 1 );

    return 0 ;
}

