/*
 * File:   main.c
 * Author: group 1
 *
 * Created on June 9, 2025, 5:32 PM
 */


#include "xc.h"
#include "interrupt.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"


// State flags
int current_state;    // Current state of the robot
int is_pwm_on;       // Flag for PWM generation status

// LED pin definition.
#define LED1 LATAbits.LATA0


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

    // Initialize interrupts and states
    init_interrupts();
    
    // Initialize PWM and ensure motors are stopped
    init_pwm();
    stop_motors();
    
    // Configure system timers
    tmr_setup_period(TIMER1, 2);  // TIMER1: 500Hz main loop timing (2ms)
    tmr_setup_period(TIMER2, 20); // TIMER2: Used for button debouncing

    while (1) {
        // Handle LED blinking (1000ms period)
        if (tmr_counter_led == 1000) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }
        
        // Update timing counters
        tmr_counter_led += 2; // Increment by 2ms
        
        // Wait for next timer period
        tmr_wait_period(TIMER1);
    }

    return 0;
}
