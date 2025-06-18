/*
 * File:   main.c
 * Author: group 1
 *
 * Created on June 9, 2025, 5:32 PM
 */

#include <stdio.h>
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

extern int x_values_acc[ARRAY_SIZE];
extern int y_values_acc[ARRAY_SIZE];
extern int z_values_acc[ARRAY_SIZE];

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
    float distance_threshold = 0.2; // Distance threshold for emergency state (15cm)

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
    int tmr_counter_send_distance = 0;
    int tmr_counter_accelerometer = 0;
    int tmr_counter_uart = 0;

    // Configure SPI
    spi_setup();

    // Configure accelerometer
    accelerometer_config();
    char acc_message[32]; // Buffer for ACC message

    while (1) {
        if (rxStringReady) {
            // A command is ready. Call the processor function.
            process_uart_command((const char *) rxBuffer);
            // CRITICAL: Clear the flag so we don't process the same command again.
            rxStringReady = 0; // it will be setted later if we recieve another command from uart
        }

        // Handle LED blinking (1000ms period)
        if (tmr_counter_led == 500) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }

        distance = adc_distance(); // Read distance from ADC

        if (tmr_counter_send_distance == 100) { // Send distance every 100ms
            char distance_message[TX_BUFFER_SIZE];
            sprintf(distance_message, "$MDIST,%d*\r\n", average_distance());
            UART_SendString(distance_message);
            tmr_counter_send_distance = 0; // Reset send distance counter
        }

        if (current_state == STATE_MOVING) {
            if (distance < distance_threshold) {
                UART_SendString("$MEMRG,1* \r\n");
                LATGbits.LATG9 = 1; // DEBUG
                tmr_counter_emergency = 0; // Reset emergency counter
                current_state = STATE_EMERGENCY;
                stop_motors();
            } else {
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
                    UART_SendString("$MEMRG,0 \r\n");
                }

            }
        }

        // Acquire accelerometer data at 10Hz (every 100ms)
        if (tmr_counter_accelerometer == 100) {
            tmr_counter_accelerometer = 0; // Reset accelerometer counter
            acquire_accelerometer_data();
        }

        // Process and transmit ACC data at configurable rate (10 Hz)
        if (tmr_counter_uart == 100) {
            // Filter accelerometer value
            int x_acc = filter_accelerometer(x_values_acc, ARRAY_SIZE, 'x');
            int y_acc = filter_accelerometer(y_values_acc, ARRAY_SIZE, 'y');
            int z_acc = filter_accelerometer(z_values_acc, ARRAY_SIZE, 'z');

            sprintf(acc_message, "$MACC,%d,%d,%d*\r\n", x_acc, y_acc, z_acc);
            UART_SendString(acc_message);
            tmr_counter_uart = 0;
        }

        // Maintain precise 500Hz loop timing
        tmr_wait_period(TIMER1); // Wait for timer period completion

        // Update timing counters (increment by 2ms)
        tmr_counter_send_distance += 2;
        tmr_counter_led += 2;
        tmr_counter_accelerometer += 2;
        tmr_counter_uart += 2;
    }
    return 0;
}