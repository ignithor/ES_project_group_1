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
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


// LED alias definitions for clarity in code.
#define LED1 LATAbits.LATA0

extern volatile char rxBuffer[RX_BUFFER_SIZE];
extern volatile uint16_t rxIndex;
extern volatile uint8_t rxStringReady;
extern uint8_t currentRate; // Initialize with 5 Hz as required
extern uint8_t currentRate; // Initialize with 5 Hz as required
extern int x_values_acc[ARRAY_SIZE];
extern int y_values_acc[ARRAY_SIZE];
extern int z_values_acc[ARRAY_SIZE];

unsigned int uart_period_ms = 200;

char localCopy[RX_BUFFER_SIZE];

int main(void) {
    // Initialize pins
    TRISAbits.TRISA0 = 0; // pin A0 set as output (LED1)

    // Disable all analog functionality on pins to use them as digital I/O
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    LED1 = 1; // LED initially on
    int tmr_counter_led = 0;

    // Configure timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)

    // Configure SPI
    spi_setup();

    // Configure UART
    UART_Initialize();

    // Configure accelerometer
    accelerometer_config();

    // Configure timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (10ms)


    // Tracks elapsed time in main loop
    int tmr_counter_accelerometer = 0;
    int tmr_counter_uart = 0;

    char acc_message[32]; // Buffer for ACC message

    // Configure timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)


    while (1) {

        // if (rxStringReady) {
        //     strcpy(localCopy, (char*) rxBuffer); // Copy safely
        //     rxStringReady = 0; // Reset flag AFTER copy
        //     rxIndex = 0; // Also reset index here (not inside ISR)

        //     UART_SendString("You entered: ");
        //     UART_SendString(localCopy);
        //     UART_SendString("\r\n");
        //     process_uart_command(localCopy);
        // }


        // If 1000ms waited switch the LED
        if (tmr_counter_led == 1000) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }

        // // Acquire accelerometer data at 10Hz (every 100ms)
        if (++tmr_counter_accelerometer >= 100) {
            acquire_accelerometer_data();
            tmr_counter_accelerometer = 0;
        }
        
        // Calculate averages of last 5 measurements once per main loop iteration
        int x_acc = filter_acc(x_values_acc, ARRAY_SIZE);
        int y_acc = filter_acc(y_values_acc, ARRAY_SIZE);
        int z_acc = filter_acc(z_values_acc, ARRAY_SIZE);

        // Process and transmit ACC data at configurable rate (xx Hz)
        if (currentRate > 0) {
            uart_period_ms = (1.0 / currentRate)*1000.0;
            if (tmr_counter_uart % uart_period_ms == 0) {
                sprintf(acc_message, "$MACC,%d,%d,%d*\r\n", x_acc, y_acc, z_acc);
                UART_SendString(acc_message);
                tmr_counter_uart = 0;
            }
        }

        // Maintain precise 500Hz loop timing
        tmr_wait_period(TIMER1); // Wait for timer period completion
        tmr_counter_led += 2; // Increment counters by 2ms
        tmr_counter_accelerometer += 10;
        tmr_counter_uart += 10;
    }
    return 0;
}