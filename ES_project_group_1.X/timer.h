#ifndef TIMER_H
#define TIMER_H

#include <xc.h>

// Timer usage definitions:
// TIMER1: Main loop period management at 500 Hz.
// TIMER2: Used to process bouncing in interrupt routine for pressed button.
#define TIMER1 1
#define TIMER2 2

// Define System Clock Frequency used for timer calculations.
#define FCY 72000000


/**
 * @brief Configure a timer for a specified period
 * 
 * Sets up the selected timer with appropriate prescaler and period register
 * values to generate timing events at the specified millisecond interval.
 * 
 * @param timer Timer identifier (TIMER1, TIMER2)
 * @param ms Desired period in milliseconds
 */
void tmr_setup_period(int timer, int ms);

/**
 * @brief Wait for a timer period to complete
 * 
 * Blocks execution until the specified timer's period has elapsed (flag set)
 * then clears the flag for the next period.
 * 
 * @param timer Timer identifier (TIMER1 or TIMER2)
 */
void tmr_wait_period(int timer);

#endif