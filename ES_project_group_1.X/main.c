/*
 * File:   main.c
 * Author: group 1
 *
 * Created on June 9, 2025, 5:32 PM
 */


#include "xc.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"

// State definitions
#define STATE_WAIT_FOR_START 0
#define STATE_MOVING         1
#define STATE_EMERGENCY      2

// State flags
int current_state;    // Current state of the robot
int is_pwm_on;       // Flag for PWM generation status

// LED alias definitions for clarity in code.
#define LED1 LATAbits.LATA0

// Interrupt routine for pressed button
void __attribute__((__interrupt__, __auto_psv__))_INT1Interrupt(void) {
   IFS1bits.INT1IF = 0; // clear the interrupt flag
   IEC1bits.INT1IE = 0; // disable interrupt

   // Only process button press if not in emergency state
   if (current_state != STATE_EMERGENCY) {
       if (current_state == STATE_WAIT_FOR_START) {
           // Transition to Moving state
           current_state = STATE_MOVING;
           // Set both motors to move forward at 50% duty cycle
           set_motor_pwm(PWM_PERIOD/2, PWM_PERIOD/2);
       } else if (current_state == STATE_MOVING) {
           // Transition back to Wait for Start state
           current_state = STATE_WAIT_FOR_START;
           stop_motors();
       }
   }

   tmr_setup_period(TIMER2, 20);
   IEC0bits.T2IE = 1;
}

// Debouncing using two interrupts
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void){
   IFS0bits.T2IF = 0;      // clear the flag on the interrupt 2
   IEC0bits.T2IE = 0;      // disable the interrupt of the timer 2
   
   IEC1bits.INT1IE = 1;
//    LATAbits.LATA0 = !LATAbits.LATA0;   
}

int main(void) {
    // Initialize pins
    TRISAbits.TRISA0 = 0; // pin A0 set as output (LED1)

    // Disable all analog functionality on pins to use them as digital I/O
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    LED1 = 1; // LED initially on
    int tmr_counter_led = 0;

    // Initialize states
    current_state = STATE_WAIT_FOR_START;
    is_pwm_on = 0;

    // Set pin as input
    TRISEbits.TRISE8 = 1; // T2 button set as input
    TRISEbits.TRISE9 = 1; // T3 button set as input

    /*Enable interrupts in INT1*/
    RPINR0bits.INT1R = 0x58;    //RPI88 in hex (88)
    INTCON2bits.GIE = 1;        // set global interrupt enable
    IFS1bits.INT1IF = 0;        // clear the interrupt flag
    IEC1bits.INT1IE = 1;        // enable interrupt

    init_pwm();
    stop_motors();  // Ensure motors are stopped at startup

    // Configure timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)
    tmr_setup_period(TIMER2, 20);


    while (1) {

        // If 1000ms waited switch the LED
        if (tmr_counter_led == 1000) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }

        tmr_counter_led += 2; // Increment counters by 2ms
        tmr_wait_period(TIMER1); // Wait for timer period completion
    }

    return 0;
}
