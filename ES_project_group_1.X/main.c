/*
 * File:   main.c
 * Author: group 1
 *
 * Created on June 9, 2025, 5:32 PM
 */

#include <math.h>
#include "xc.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "adc.h"


// LED alias definitions for clarity in code.
#define LED1 LATAbits.LATA0
#define LED2 LATGbits.LATG9

// Define TURN signal pins
#define TURN_L LATFbits.LATF1
#define TURN_R LATBbits.LATB8

int main(void) {


    // Disable all analog functionality on pins to use them as digital I/O
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    // Initialize pins
    TRISAbits.TRISA0 = 0; // pin A0 set as output (LED1)
    TRISGbits.TRISG9 = 0; // pin G9 set as output (LED2)

    TRISFbits.TRISF1 = 0; // Left LED
    TRISBbits.TRISB8 = 0; // Right LED

    setup_adc(); // setup IR sensor ADC

    LED1 = 1; // LED initially on
    int tmr_counter_led = 0;

    // Initialize state
    TURN_L = 0;
    TURN_R = 0;

    // Configure timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)


    while (1) {

        // If 1000ms waited switch the LED
        if (tmr_counter_led == 500) {
            LED1 = !LED1;
            TURN_L = !TURN_L;
            TURN_R = !TURN_R;
            tmr_counter_led = 0;
        }
        // Read ADC: Manual sampling & Automatic conversion
        AD1CON1bits.SAMP = 1; // Start sampling
        while (!AD1CON1bits.DONE); // Wait for the conversion to complete
        int ADC_value = ADC1BUF0; // Read the conversion result

        // Convert the result distance
        float voltage = (float) ADC_value * 3.3 / 1023;
        float distance = 2.34 - 4.74 * voltage + 4.06 * pow(voltage, 2) - 1.60 * pow(voltage, 3) + 0.24 * pow(voltage, 4);

        // For testing
        if (distance < 0.3) {
            LATGbits.LATG9 = 1;
        } else {
            LATGbits.LATG9 = 0;
        }

        tmr_counter_led += 2; // Increment counters by 2ms
        tmr_wait_period(TIMER1); // Wait for timer period completion
    }

    return 0;
}