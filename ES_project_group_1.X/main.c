/* ===============================================================
 * File:   main.c                                                =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * Created on June 9, 2025, 5:32 PM                              =
 * ===============================================================*/
/*================================================================*/

//includes                                                                  
#include <stdio.h>
#include "xc.h"
#include "interrupt.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "adc.h"
/*================================================================*/

// Macros
// LED pin definition.
#define LED1 LATAbits.LATA0
// Define TURN signal pins
#define TURN_L LATFbits.LATF1
#define TURN_R LATBbits.LATB8
/*================================================================*/

/*===============================================================================*/

// Global variables                                                                
// 'volatile' is used because these are modified by UART and read by main loop 
volatile int g_speed = 0;                                                      
volatile int g_yawrate = 0;                                                    
// State flags                                                                 
int is_pwm_on; // Flag for PWM generation status                               
// Command buffer "all commands receieved in circular buffer"                  
extern volatile char rxBuffer[RX_BUFFER_COUNT][RX_STRING_LENGTH];              
extern volatile uint8_t rx_write_index;                                        
extern volatile uint8_t rx_read_index;                                                                                                  
/*================================================================================*/

// User defined variables
typedef enum {
    STATE_WAIT_FOR_START = 0,
    STATE_MOVING,
    STATE_EMERGENCY
} RobotState;
/*==============================================================================*/

// Instance of the user defined variable
volatile RobotState current_state;
/*==============================================================================*/

int main(void) {
    /*==========================================================================*/
    // Initializing variables, state and some hardware configurations

    // Disable all analog functionalities on pins to use them as digital I/O
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    // Initialize LEDS pins
    TRISAbits.TRISA0 = 0; // pin A0 set as output (LED1)
    TRISFbits.TRISF1 = 0; // Left LED
    TRISBbits.TRISB8 = 0; // Right LED
    // Initialize state
    TURN_L = 0;
    TURN_R = 0;
    float distance = 0.0; // Variable to store distance from IR sensor
    float distance_threshold = 0.2; // Distance threshold for emergency state (20cm)
    // Initialize states
    current_state = STATE_WAIT_FOR_START;
    is_pwm_on = 0; //pwm initially off
    // Register for accelerometer
    int x_acc, y_acc, z_acc;
    /*==========================================================================*/
    //peripheral initialization
    UART_Initialize();
    setup_adc(); // setup IR sensor and battery ADC
    // Initialize interrupts
    init_interrupts();
    // Initialize PWM and ensure motors are stopped
    init_pwm();
    //stop motor command to make sure pwm signal are zero
    set_motor_pwm(0, 0); // stop motors
    // Configure SPI
    spi_setup();
    // Configure accelerometer
    accelerometer_config();
    /*==========================================================================*/
    // Configure system timers
    tmr_setup_period(TIMER1, 2); // TIMER1: 500Hz main loop timing (2ms)
    tmr_setup_period(TIMER2, 20); // TIMER2: Used for button debouncing
    LED1 = 1; // LED initially on indicating all inilization went good and the system is ready now
    /*==========================================================================*/
    // Initializing counters 
    int tmr_counter_led = 0;
    int tmr_counter_side_leds = 0;
    int tmr_counter_emergency = 0;
    int tmr_counter_send_distance = 0;
    int tmr_counter_accelerometer = 0;
    int tmr_counter_battery = 0;
    int tmr_counter_battery_read = 0;
    /*==========================================================================*/

    while (1) {
        /*==========================================================================*/
        // Process all pending commands from the circular buffer
        while (rx_read_index != rx_write_index) {
            process_uart_command((const char *) rxBuffer[rx_read_index]);
            rx_read_index = (rx_read_index + 1) % RX_BUFFER_COUNT;
        }
        /*==========================================================================*/
        // Handle LED blinking at 1Hz
        if (tmr_counter_led == 500) {
            LED1 = !LED1;
            tmr_counter_led = 0;
        }
        /*==========================================================================*/
        // Distance handling
        distance = adc_distance(); // Read distance from ADC
        if (tmr_counter_send_distance == 100) { // Send distance every 100ms
            char distance_message[RX_STRING_LENGTH];
            sprintf(distance_message, "$MDIST,%d*\r\n", average_distance());
            UART_SendString(distance_message);
            tmr_counter_send_distance = 0; // Reset send distance counter
        }
        /*==========================================================================*/
        // Battery handling
        // Acquire battery voltage at 5Hz (every 200ms)
        if (tmr_counter_battery_read == 200) {
            adc_battery_voltage();
            tmr_counter_battery_read = 0;
        }
        // Send battery voltage at 1Hz (every 1000ms)
        if (tmr_counter_battery == 1000) {
            double avg_battery_voltage = average_battery_voltage();
            char bat_message[RX_STRING_LENGTH];
            sprintf(bat_message, "$MBATT,%.2f*\r\n", avg_battery_voltage);
            UART_SendString(bat_message);
            tmr_counter_battery = 0;
        }
        /*==========================================================================*/
        // State moving handling
        if (current_state == STATE_MOVING) {
            if (distance < distance_threshold) {
                UART_SendString("$MEMRG,1* \r\n");
                tmr_counter_emergency = 0; // Reset emergency counter
                current_state = STATE_EMERGENCY;
                set_motor_pwm(0, 0); // stop motors
            } else {
                control_motors(g_speed, g_yawrate);
            }
        }
        /*==========================================================================*/
        // State emergency handling
        if (current_state == STATE_EMERGENCY) {
            tmr_counter_side_leds += 2; // Increment side LED counter by 2ms
            if (tmr_counter_side_leds == 500) {
                TURN_L = !TURN_L;
                TURN_R = !TURN_R;
                tmr_counter_side_leds = 0; // Reset side LED counter
            }
            if (distance < distance_threshold) {
                tmr_counter_emergency = 0; // Reset emergency counter
            } else {
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
        /*==========================================================================*/
        // Acquire accelerometer data at 10Hz (every 100ms)
        if (tmr_counter_accelerometer == 100) {
            tmr_counter_accelerometer = 0; // Reset accelerometer counter
            acquire_accelerometer_data(&x_acc, &y_acc, &z_acc);
            char acc_message[RX_STRING_LENGTH]; // Buffer for ACC message
            sprintf(acc_message, "$MACC,%d,%d,%d*\r\n", x_acc, y_acc, z_acc);
            UART_SendString(acc_message);
        }
        /*==========================================================================*/
        // Time handling
        // Maintain 500Hz loop timing
        tmr_wait_period(TIMER1); // Wait for timer period completion
        // Update timing counters (increment by 2ms)
        tmr_counter_send_distance += 2;
        tmr_counter_led += 2;
        tmr_counter_accelerometer += 2;
        tmr_counter_battery += 2;
        tmr_counter_battery_read += 2;
        /*==========================================================================*/
    }
    return 0;
}