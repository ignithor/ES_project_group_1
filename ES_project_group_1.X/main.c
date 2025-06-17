/*
 * File:   main.c
 * Author: group 1
 *
 * Created on June 9, 2025, 5:32 PM
 */

#include <math.h>
#include "xc.h"
#include "interrupt.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "adc.h"


// State definitions
#define STATE_WAIT_FOR_START 0
#define STATE_MOVING         1
#define STATE_EMERGENCY      2

// State flags
int current_state; // Current state of the robot
int is_pwm_on; // Flag for PWM generation status

// 'volatile' is used because these are modified by UART and read by main loop
volatile int g_speed = 0;
volatile int g_yawrate = 0;


extern volatile char rxBuffer[RX_BUFFER_SIZE];
extern volatile uint8_t rxStringReady;


// LED pin definition.
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

    UART_Initialize();
    setup_adc(); // setup IR sensor ADC

    // Initialize state
    TURN_L = 0;
    TURN_R = 0;

    float distance = 0.0; // Variable to store distance from IR sensor
    float distance_threshold = 0.3; // Distance threshold for emergency state (30cm)

    // Initialize states
    current_state = STATE_WAIT_FOR_START;
    is_pwm_on = 0;

    // Initialize interrupts and states
    init_interrupts();

    // Initialize PWM and ensure motors are stopped
    init_pwm();
    stop_motors();

    // Configure system timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)
    tmr_setup_period(TIMER2, 20); // TIMER2: Used for button debouncing
    LED1 = 1; // LED initially on
    int tmr_counter_led = 0;
    int tmr_counter_side_leds = 0;
    int tmr_counter_emergency = 0;

    while (1) {
        if (rxStringReady) {
        // A command is ready. Call the processor function.
        process_uart_command((const char *)rxBuffer);
        // CRITICAL: Clear the flag so we don't process the same command again.
        rxStringReady = 0; // it will be setted later if we recieve another command from uart
        }
        
        // Handle LED blinking (1000ms period)
        if (tmr_counter_led == 500) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }

        distance = adc_distance(); // Read distance from ADC

        if (current_state == STATE_MOVING) {
            if (distance < distance_threshold) {
                UART_SendString("$MEMRG,1*");
                LATGbits.LATG9 = 1; // DEBUG
                tmr_counter_emergency = 0; // Reset emergency counter
                current_state = STATE_EMERGENCY;
                stop_motors();
            }
            else
            {
                control_motors(g_speed, g_yawrate);
            }
        }

        if (current_state == STATE_EMERGENCY) {
            tmr_counter_side_leds += 2; // Increment side LED counter by 2ms
            if (tmr_counter_side_leds == 500) {
                TURN_L = !TURN_L;
                TURN_R = !TURN_R;
                tmr_counter_side_leds = 0; // Reset side LED counter
            }
            if (distance < distance_threshold) {
                LATGbits.LATG9 = 1; // DEBUG
                tmr_counter_emergency = 0; // Reset emergency counter
            } else {
                LATGbits.LATG9 = 0;
                tmr_counter_emergency += 2; // Increment emergency counter by 2ms
                if (tmr_counter_emergency == 5000) { // If 5000ms passed in emergency state
                    current_state = STATE_WAIT_FOR_START; // Reset to wait for start state
                    tmr_counter_emergency = 0; // Reset emergency counter
                    TURN_L = 0; // Turn off left turn signal
                    TURN_R = 0; // Turn off right turn signal
                    tmr_counter_side_leds = 0; // Reset side LED counter
                    UART_SendString("$MEMRG,0");
                }

            }
        }

        // Update timing counters
        tmr_counter_led += 2; // Increment by 2ms

        // Wait for next timer period
        tmr_wait_period(TIMER1);
    }
    return 0;
}