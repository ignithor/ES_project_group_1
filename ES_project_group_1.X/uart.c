/*
 * Description: This file implements a fully interrupt-driven, non-blocking UART driver
 * using circular buffers for both transmission (TX) and reception (RX), as suggested
 * by the assignment to handle tight scheduling. 
 */
#include "interrupt.h"
#include "xc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

// --- Global and Static Variables ---

// RX Buffer and state variables
volatile char rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rx_idx = 0;
volatile uint8_t rxStringReady = 0;

// TX Circular Buffer
static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;


// knows they exist in main.c
extern volatile int g_speed;
extern volatile int g_yawrate;
extern int current_state; 

typedef enum {
    CMD_PCREF,
    CMD_PCSTP,
    CMD_PCSTT,
    CMD_UNKNOWN
} CommandType;

// --- Public Functions ---

void UART_Initialize(void) {
    // PPS configuration
    RPOR0bits.RP64R = 1; // Set RP64 as U1TX
    RPINR18bits.U1RXR = 75; // Set RP75 as U1RX

    // UART mode configuration
    U1MODEbits.STSEL = 0; // 1 Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8 data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud Disabled
    U1MODEbits.BRGH = 0; // Low Speed mode
    U1BRG = BRGVAL; // BAUD Rate Setting for 9600

    // TX Interrupt is generated when a character is transferred to the shift register
    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 0;

    IEC0bits.U1RXIE = 1; // Enable UART RX Interrupt
    IEC0bits.U1TXIE = 0; // TX Interrupt is disabled until there is data to send

    // Enable UART
    U1MODEbits.UARTEN = 1; // Enable UART module
    U1STAbits.UTXEN = 1; // Enable UART TX
}

// Queues a string for transmission. This function is NON-BLOCKING.

void UART_SendString(const char *str) {
    // Add all characters to the software buffer
    while (*str) {
        uint16_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
        if (next_head == tx_tail) {
            // Buffer is full. Stop adding to prevent an overwrite.
            // This is better than freezing the whole system.
            break;
        }
        tx_buffer[tx_head] = *str++;
        tx_head = next_head;
    }

    // **CRITICAL FIX**: If the TX interrupt is disabled, the transmitter was idle.
    // We must enable it AND manually set the flag to force the ISR to run once
    // and begin the transmission process.
    if (IEC0bits.U1TXIE == 0 && tx_head != tx_tail) {
        IEC0bits.U1TXIE = 1; // Enable the interrupt
        IFS0bits.U1TXIF = 1; // Manually trigger the interrupt
    }
}

static CommandType get_command_type(const char *input) {
    if (strncmp(input, "$PCREF,", 7) == 0) return CMD_PCREF;
    if (strncmp(input, "$PCSTP,", 7) == 0) return CMD_PCSTP;
    if (strncmp(input, "$PCSTT,", 7) == 0) return CMD_PCSTT;
    return CMD_UNKNOWN;
}

void process_uart_command(const char *input) {
    switch (get_command_type(input)) {
        case CMD_PCREF:
            process_pcref_command(input);
            break;

        case CMD_PCSTP:
            if (current_state != STATE_EMERGENCY) {
                current_state = STATE_WAIT_FOR_START;
                stop_motors();
                UART_SendString("$MACK,1*\r\n");
            } else {
                UART_SendString("$MACK,0*\r\n");
            }
            break;

        case CMD_PCSTT:
            if (current_state != STATE_EMERGENCY) {
                current_state = STATE_MOVING;
                UART_SendString("$MACK,1*\r\n");
            } else {
                UART_SendString("$MACK,0*\r\n");
            }
            break;

        case CMD_UNKNOWN:
        default:
            UART_SendString("$ERR,Unknown command*\r\n");
            break;
    }
}

void process_pcref_command(const char *command) {
    int speed, yawrate;

    if (sscanf(command, "$PCREF,%d,%d*", &speed, &yawrate) == 2) {
        if ((speed >= -100 && speed <= 100) && (yawrate >= -100 && yawrate <= 100)) {
            // Parsing successful and values are valid.
            // Update the global variables instead of calling the motor function.
            g_speed = speed;
            g_yawrate = yawrate;
        }
    }
}


// --- Interrupt Service Routines (ISRs) ---

// TX Interrupt: Handles sending data from the TX circular buffer.
// This ISR is short, non-blocking, and sends one character per activation. 

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0; // Clear the interrupt flag immediately

    if (tx_head != tx_tail) {
        // Buffer is not empty, send the next character.
        // The hardware will set U1TXIF again when this transfer completes,
        // which will call this ISR again, creating a chain reaction.
        U1TXREG = tx_buffer[tx_tail];
        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
    }

    if (tx_head == tx_tail) {
        // Buffer is now empty, so the job is done. Disable the interrupt.
        // It will be re-enabled and re-triggered by UART_SendString when new data arrives.
        IEC0bits.U1TXIE = 0;
    }
}

// RX Interrupt: Handles received characters, echoes them, and builds a command string. 

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0; // Clear RX interrupt flag
    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0;
    }

    char received = U1RXREG; // Read character from hardware

    // --- Non-blocking Echo ---
    // Place the received character into the TX buffer to be sent by the TX interrupt.
    uint16_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
    if (next_head != tx_tail) {
        tx_buffer[tx_head] = received;
        tx_head = next_head;

        // Same kick-start logic as UART_SendString
        if (IEC0bits.U1TXIE == 0) {
            IEC0bits.U1TXIE = 1;
            IFS0bits.U1TXIF = 1;
        }
    }

    // --- Process Character for Command String ---
    if (received == '\r' || received == '\n') {
        if (rx_idx > 0) {
            rxBuffer[rx_idx] = '\0'; // Null-terminate
            rxStringReady = 1; // Set flag for the main loop
            rx_idx = 0; // Reset for next command
        }
    } else if (rx_idx < RX_BUFFER_SIZE - 1) {
        rxBuffer[rx_idx++] = received;
    }
}