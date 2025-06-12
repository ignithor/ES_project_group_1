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


// LED alias definitions for clarity in code.
#define LED1 LATAbits.LATA0

int main(void) {
    // Initialize pins
    TRISAbits.TRISA0 = 0; // pin A0 set as output (LED1)

    // Disable all analog functionality on pins to use them as digital I/O
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    LED1 = 1; // LED initially off


    while (1) {
        LED1 = !LED1;
        // tmr_wait_period(TIMER1); // Wait for timer period completion

    }

    return 0;
}
