/* ===============================================================
 * File:   main.c                                                =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * TIMER1: Controls the main loop execution rate at 500 Hz       =
 * TIMER2: Used to process bouncing in interrupt routine for     =
 * pressed button                                                =
 * ===============================================================*/

/*================================================================*/
#include "timer.h"
/*================================================================*/

/*================================================================*/
//setup timer configuration and ticks
/*================================================================*/
void tmr_setup_period(int timer, int ms) {
    switch (timer) {
        case TIMER1:
            // TIMER1: Manage main loop period at 500Hz
            T1CONbits.TON = 0; // Disable Timer1 during configuration
            TMR1 = 0; // Reset Timer1 counter register
            T1CONbits.TCKPS = 3; // Set prescaler to 1:256
            PR1 = ((FCY / 256) * ms) / 1000 - 1; // Calculate period register value
            IFS0bits.T1IF = 0; // Clear Timer1 interrupt flag
            T1CONbits.TON = 1; // Enable Timer1
            break;

        case TIMER2:
            // TIMER2: Used to process bouncing in interrupt routine for pressed button
            T2CONbits.TON = 0; // Disable Timer2 during configuration
            TMR2 = 0; // Reset Timer2 counter register
            T2CONbits.TCKPS = 3; // Set prescaler to 1:256
            PR2 = ((FCY / 256) * ms) / 1000 - 1; // Calculate period register value
            IFS0bits.T2IF = 0; // Clear Timer2 interrupt flag
            T2CONbits.TON = 1; // Enable Timer2
            break;
        default:
            // Invalid timer specified - no action taken
            break;
    }
}
/*================================================================*/

/*================================================================*/
//busy wait timer
/*================================================================*/
void tmr_wait_period(int timer) {
    switch (timer) {
        case TIMER1:
            // TIMER1: Wait for the main loop period to complete
            while (!IFS0bits.T1IF); // Block until Timer1 period flag is set
            IFS0bits.T1IF = 0; // Clear the flag for next period
            break;

        case TIMER2:
            // TIMER2: Wait for the specified delay to complete
            while (!IFS0bits.T2IF); // Block until Timer2 period flag is set
            IFS0bits.T2IF = 0; // Clear the flag for next use
            break;

        default:
            // Invalid timer specified - no action taken
            break;
    }
}
/*================================================================*/