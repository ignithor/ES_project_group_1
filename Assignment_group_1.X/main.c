/*
 * File:   main.c
 * Author: Team 1
 *
 * Description: This file implements the main program flow for the magnetometer
 * data acquisition and transmission system. It uses a non-blocking, interrupt-driven
 * approach for UART communication and ensures all tasks meet their deadlines.
 */

#include "xc.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "timer.h"
#include "uart.h"
#include "spi.h"
#include "stdlib.h"

// LED alias for clarity
#define LED2 LATGbits.LATG9

// Extern declarations for global variables defined in other files
extern volatile char rxBuffer[RX_BUFFER_SIZE];
extern volatile uint8_t rxStringReady;
extern uint8_t currentRate; // UART rate variable from uart.c

// Extern declarations for magnetometer data from spi.c
extern int x_values[ARRAY_SIZE];
extern int y_values[ARRAY_SIZE];
extern int z_values[ARRAY_SIZE];


void algorithm() {
    // Simulate a 7ms algorithm execution time 
    tmr_wait_ms(TIMER2, 7);
}

int main(void) {
    // ---- System Initialization ----
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000; // Disable analog inputs
    TRISGbits.TRISG9 = 0; // Configure LED2 pin as output
    LED2 = 1; // LED2 initially on

    // Initialize peripherals
    spi_setup();
    magnetometer_config(); // Configures magnetometer for 25Hz 
    UART_Initialize();

    // Initialize timers
    tmr_setup_period(TIMER1, 10); // Main loop timer at 100Hz (10ms period) 

    // Send initialization message. System starts at 5Hz 
    UART_SendString("System Initialized. Magnetometer rate: 5 Hz.\r\n"); 

    // ---- Main Loop Variables ----
    int tmr_counter_led = 0;
    int tmr_counter_magnetometer = 0;
    int tmr_counter_uart = 0;
    int tmr_counter_yaw = 0;
    
    char localCopy[RX_BUFFER_SIZE];
    char mag_message[40];
    char yaw_message[25];

    // ---- Main Control Loop ----
    while (1) {
        localCopy[0] = '\0'; // Clear the local buffer

        // **Critical Section**: Disable RX interrupt to safely check flag and copy buffer 
        IEC0bits.U1RXIE = 0; 
        if (rxStringReady) {
            strcpy(localCopy, (char*) rxBuffer);
            rxStringReady = 0; // Reset flag inside critical section
        }
        IEC0bits.U1RXIE = 1; // Re-enable RX interrupt

        // Process command outside the critical section to keep it short
        if (localCopy[0] != '\0') {
            process_uart_command(localCopy);
        }

        // --- Execute Simulated Algorithm ---
        algorithm(); // 7ms execution 

        // --- Timed Tasks (Scheduled based on 10ms main loop tick) ---

        // Task: Blink LED2 at 1 Hz (500ms on, 500ms off) 
        if (tmr_counter_led >= 500) {
            LED2 = !LED2;
            tmr_counter_led -= 500; // Use subtraction to avoid drift
        }

        // Task: Acquire magnetometer data at 25 Hz (every 40ms) 
        if (tmr_counter_magnetometer >= 40) {
            acquire_magnetometer_data();
            tmr_counter_magnetometer -= 40;
        }
        
        // Calculate averages of the last 5 measurements 
        double x_avg = calculate_average(x_values, ARRAY_SIZE);
        double y_avg = calculate_average(y_values, ARRAY_SIZE);
        double z_avg = calculate_average(z_values, ARRAY_SIZE);

        // Task: Send MAG data at the user-configurable rate 
        if (currentRate > 0) {
            unsigned int uart_period_ms = 1000 / currentRate;
            if (tmr_counter_uart >= uart_period_ms) {
                sprintf(mag_message, "$MAG,%.2f,%.2f,%.2f*\r\n", x_avg, y_avg, z_avg);
                UART_SendString(mag_message);
                tmr_counter_uart -= uart_period_ms;
            }
        }

        // Task: Send YAW data at a fixed 5 Hz (every 200ms) 
        if (tmr_counter_yaw >= 200) {
            double angle = atan2(y_avg, x_avg) * (180.0 / M_PI); // Compute angle 
            sprintf(yaw_message, "$YAW,%.2f*\r\n", angle);
            UART_SendString(yaw_message);
            tmr_counter_yaw -= 200;
        }

        // --- Loop Synchronization & Counter Increments ---
        tmr_wait_period(TIMER1); // Wait for the 10ms period to end 
        
        tmr_counter_led += 10;
        tmr_counter_magnetometer += 10;
        tmr_counter_uart += 10;
        tmr_counter_yaw += 10;
    }

    return 0;
}