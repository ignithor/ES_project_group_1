/* ===============================================================
 * File: interrupt.c                                             =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/
/*===============================================================*/
#include "interrupt.h"
/*===============================================================*/

/*===============================================================*/
typedef enum {
    STATE_WAIT_FOR_START = 0,
    STATE_MOVING,
    STATE_EMERGENCY
} RobotState;
extern volatile RobotState current_state;
// External state variables
extern int is_pwm_on;
/*===============================================================*/

/*===============================================================*/
// Initialize all interrupts
void init_interrupts(void) {
    // Set pin as input
    TRISEbits.TRISE8 = 1; // T2 button set as input
    TRISEbits.TRISE9 = 1; // T3 button set as input

    // Configure INT1 interrupt for button input
    RPINR0bits.INT1R = 0x58; // Map INT1 to RPI88 (RE8)
    INTCON2bits.GIE = 1; // Enable global interrupts
    IFS1bits.INT1IF = 0; // Clear interrupt flag
    IEC1bits.INT1IE = 1; // Enable INT1 interrupt
}
/*===============================================================*/
/*===============================================================*/

/*===============================================================*/
// Interrupt routine for pressed button
/*===============================================================*/
void __attribute__((__interrupt__, __auto_psv__)) _INT1Interrupt(void) {
    // Clear interrupt flag and disable interrupt during processing
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 0;

    if (current_state != STATE_EMERGENCY) {
        switch (current_state) {
            case STATE_WAIT_FOR_START:
                // Transition to Moving state
                current_state = STATE_MOVING;
                break;

            case STATE_MOVING:
                // Transition back to Wait for Start state
                current_state = STATE_WAIT_FOR_START;
                set_motor_pwm(0, 0); // stop motors
                break;

            default:
                // Invalid state, transition to Wait for Start
                current_state = STATE_WAIT_FOR_START;
                set_motor_pwm(0, 0); // stop motors
                break;
        }
    }

    // Setup debounce timer
    tmr_setup_period(TIMER2, 20);
    IEC0bits.T2IE = 1;
}
/*===============================================================*/

/*===============================================================*/
// Debouncing using two interrupts
/*===============================================================*/
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void) {
    // Clear timer interrupt flag
    IFS0bits.T2IF = 0;
    // Disable timer interrupt
    IEC0bits.T2IE = 0;
    // Re-enable button interrupt
    IEC1bits.INT1IE = 1;
}
/*===============================================================*/
