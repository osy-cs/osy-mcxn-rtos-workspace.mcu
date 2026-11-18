// **************************************************************************
//
//                  FreeRTOS demo program for OSY labs
//
// Subject:         Operating Systems
// Topic:           FreeRTOS
// Author:          Petr Olivka, petr.olivka@vsb.cz, 11/2026
// Organization:    Department of Computer Science, FEECS,
//                  VSB-Technical University of Ostrava, CZ
//
// File:             GPIO initialization for interrupts
//
// **************************************************************************
//

// Semaphores for synchronization ISR and application
extern SemaphoreHandle_t g_sem_left_button;
extern SemaphoreHandle_t g_sem_right_button;

// Position of active red LED
extern int32_t g_red_led_index;

// Initialize GPIO for interrupts
void InitGPIOInterrupts();


