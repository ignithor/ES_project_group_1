#ifndef TIMER_H
#define TIMER_H
/*================================================================*/
#include <xc.h>
/*================================================================*/

/*================================================================*/
// Timer usage definitions:
// TIMER1: Main loop period management at 500 Hz.
// TIMER2: Used to process bouncing in interrupt routine for pressed button.
#define TIMER1 1
#define TIMER2 2
#define FCY 72000000
/*================================================================*/

/*================================================================*/
// Configures the specified timer to generate a periodic interrupt.
// Sets the prescaler and period register to match the desired timing.
// Parameters:
//   timer - Timer identifier (TIMER1 or TIMER2)
//   ms    - Desired period in milliseconds
void tmr_setup_period(int timer, int ms);
/*================================================================*/

/*================================================================*/
// Waits until the selected timer completes its current period.
// Blocks until the timer flag is set, then clears the flag.
// Parameters:
//   timer - Timer identifier (TIMER1 or TIMER2)
void tmr_wait_period(int timer);
/*================================================================*/
#endif