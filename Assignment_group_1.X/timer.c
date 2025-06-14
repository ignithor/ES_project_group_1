/*
 * File:   timer.c
 * Author: Team 1
 *
 * Created on March 3, 2025, 9:32 AM
 * 
 * Description:
 * This file implements timer management functionality for the application.
 * It handles configuring and using multiple timer peripherals for different timing requirements:
 * - TIMER1: Controls the main loop execution rate at 100Hz
 * - TIMER2: Used for precise busy wait delays (e.g., 7ms algorithm simulation)
 */

#include "xc.h"
#include "timer.h"

/**
 * @brief Configure a timer for a specified period
 * 
 * Sets up the selected timer with appropriate prescaler and period register
 * values to generate timing events at the specified millisecond interval.
 * 
 * @param timer Timer identifier (TIMER1, TIMER2)
 * @param ms Desired period in milliseconds
 */
void tmr_setup_period(int timer, int ms) {
    switch (timer) {
        case TIMER1:
            // TIMER1: Manage main loop period at 100Hz
            T1CONbits.TON = 0;      // Disable Timer1 during configuration
            TMR1 = 0;               // Reset Timer1 counter register
            T1CONbits.TCKPS = 3;    // Set prescaler to 1:256
            PR1 = ((FCY / 256) * ms) / 1000 - 1;  // Calculate period register value
            IFS0bits.T1IF = 0;      // Clear Timer1 interrupt flag
            T1CONbits.TON = 1;      // Enable Timer1
            break;

        case TIMER2:
            // TIMER2: Used for precise delays (e.g., 7ms algorithm simulation)
            T2CONbits.TON = 0;      // Disable Timer2 during configuration
            TMR2 = 0;               // Reset Timer2 counter register
            T2CONbits.TCKPS = 3;    // Set prescaler to 1:256
            PR2 = ((FCY / 256) * ms) / 1000 - 1;  // Calculate period register value
            IFS0bits.T2IF = 0;      // Clear Timer2 interrupt flag
            T2CONbits.TON = 1;      // Enable Timer2
            break;
        default:
            // Invalid timer specified - no action taken
            break;
    }
}

/**
 * @brief Wait for a timer period to complete
 * 
 * Blocks execution until the specified timer's period has elapsed (flag set)
 * then clears the flag for the next period.
 * 
 * @param timer Timer identifier (TIMER1 or TIMER2)
 */
void tmr_wait_period(int timer) {
    switch (timer) {
        case TIMER1:
            // TIMER1: Wait for the main loop period to complete
            while (!IFS0bits.T1IF);  // Block until Timer1 period flag is set
            IFS0bits.T1IF = 0;       // Clear the flag for next period
            break;
            
        case TIMER2:
            // TIMER2: Wait for the specified delay to complete
            while (!IFS0bits.T2IF);  // Block until Timer2 period flag is set
            IFS0bits.T2IF = 0;       // Clear the flag for next use
            break;

        default:
            // Invalid timer specified - no action taken
            break;
    }
}

/**
 * @brief Perform a busy wait for specified milliseconds
 * 
 * Configures and uses the specified timer to create a precise
 * delay for the given number of milliseconds.
 * 
 * @param timer Timer identifier (typically TIMER2)
 * @param ms Delay duration in milliseconds
 */
void tmr_wait_ms(int timer, int ms) {
    // Configure timer for the specified period
    tmr_setup_period(timer, ms);
    
    // Block until the timer period completes
    tmr_wait_period(timer);

    // Stop the timer if it was TIMER2 (for one-shot delays)
    switch (timer) {
        case TIMER2:
            T2CONbits.TON = 0;  // Disable Timer2 after use
            break;
        default:
            // Other timers remain running for continuous operation
            break;
    }
}