#include "xc.h"
#include "timer.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stdlib.h"

volatile char rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rxIndex = 0;
volatile uint8_t rxStringReady = 0;

uint8_t currentRate = 5; // Initialize with 5 Hz as required


void UART_Initialize(void) {
    // PPS configuration
    RPOR0bits.RP64R = 1;       // Set RP64 as U1TX
    RPINR18bits.U1RXR = 75;     // Set RP75 as U1RX
    
    tmr_setup_period(TIMER1, 200);
    
    // UART configuration
    U1MODEbits.STSEL = 0;   // 1 Stop bit
    U1MODEbits.PDSEL = 0;   // No Parity, 8 data bits
    U1MODEbits.ABAUD = 0;   // Auto-Baud Disabled
    U1MODEbits.BRGH  = 0;   // Low Speed mode

    U1BRG = BRGVAL;         // BAUD Rate Setting for 9600
    
    // Transmit interrupt configuration
    U1STAbits.UTXISEL0 = 0; // Interrupt after one TX Character is transmitted
    U1STAbits.UTXISEL1 = 0;
    
    IEC0bits.U1TXIE = 1;    // Enable UART TX Interrupt
    IEC0bits.U1RXIE = 1;    // Enable UART RX Interrupt

    // Enable UART
    U1MODEbits.UARTEN = 1;  // Enable UART module
    U1STAbits.UTXEN = 1;    // Enable UART TX
    
    // Clear any pending flags
    IFS0bits.U1TXIF = 0;
    IFS0bits.U1RXIF = 0;
}

void UART_SendChar(char data) {
    while(U1STAbits.UTXBF); // Wait until TX buffer is empty
    U1TXREG = data;
}

void UART_SendString(const char *str) {
    while(*str) {
        UART_SendChar(*str++);
    }
}

uint8_t UART_DataAvailable(void) {
    return U1STAbits.URXDA; // Check if data is available
}

uint8_t UART_ReceiveErrorCheck(void) {
    // Returns 1 if error occurred, 0 otherwise
    if(U1STAbits.FERR) {
        return 1; // Framing error
    }
    if(U1STAbits.OERR) {
        U1STAbits.OERR = 0; // Clear overrun error
        return 1; // Overrun error
    }
    return 0;
}

char UART_ReceiveChar(void) {
    while(!UART_DataAvailable()); // Wait for data
    if(UART_ReceiveErrorCheck()) {
        return 0; // Return 0 on error
    }
    return U1RXREG;
}



void UART_ReceiveString(char* buffer, uint16_t maxLength) {
    uint16_t i = 0;
    char received;
    
    while(1) {
        if(UART_DataAvailable()) {
            received = UART_ReceiveChar();
            
            // Echo back the character
            UART_SendChar(received);
            
            // Check for Enter key (CR or LF) to terminate
            if(received == '\r' || received == '\n') {
                buffer[i] = '\0'; // Null-terminate the string
                UART_SendString("\r\n"); // Send new line after Enter
                break;
            }
            // Handle backspace
            else if(received == '\b' && i > 0) {
                i--;
                UART_SendString("\b \b"); // Erase the character on terminal
            }
            else if(i < maxLength - 1) {
                buffer[i++] = received;
            }
        }
    }
}



void process_rate_command(const char* rateStr) {
    // Check if the string can be converted to a number
    char* endptr;
    long rate = strtol(rateStr, &endptr, 10);
    
    // Check if conversion was successful and value is valid
    if (*endptr != '\0' || rateStr == endptr) {
        // Not a valid number
        UART_SendString("$ERR,1*\r\n");
        return;
    }
    
    // Check against valid rates
    if (rate == 0 || rate == 1 || rate == 2 || rate == 4 || rate == 5 || rate == 10) {
        currentRate = (uint8_t)rate;
        UART_SendString("$OK*\r\n");
        UART_SendString("New rate set to: ");
        char rateMsg[10];
        sprintf(rateMsg, "%d Hz\r\n", currentRate);
        UART_SendString(rateMsg);
    } else {
        // Invalid rate
        UART_SendString("$ERR,1*\r\n");
    }
}


void process_uart_command(const char *input) {
    // Check for RATE command format: $RATE,xx*
    if (strncmp(input, "$RATE,", 6) == 0) {
        // Find the asterisk at the end
        char* asterisk = strchr(input + 6, '*');
        
        if (asterisk != NULL && asterisk - (input + 6) > 0) {
            // Extract the rate part
            char rateStr[10];
            strncpy(rateStr, input + 6, asterisk - (input + 6));
            rateStr[asterisk - (input + 6)] = '\0';
            
            process_rate_command(rateStr);
        } else {
            // Malformed command
            UART_SendString("$ERR,1*\r\n");
        }
    }
    else if (strcmp(input, "TEST") == 0) {
        // Test command - echoes back with LED blink
        UART_SendString("TEST OK\r\n");
    }
    else if (strcmp(input, "RATE?") == 0) {
        // Query current rate
        UART_SendString("Current rate: ");
        char rateMsg[10];
        sprintf(rateMsg, "%d Hz\r\n", currentRate);
        UART_SendString(rateMsg);
    }
    else if (input[0] != '\0') {
        // Unknown command
        UART_SendString("Unknown command. Use $RATE,xx* format\r\n");
    }
}



void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void){
    // clear TX interrupt flag
    IFS0bits.U1TXIF = 0;
    while(U1STAbits.UTXBF); // Wait until TX buffer is empty
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0; // Clear RX interrupt flag

    // Check for errors
    if (U1STAbits.FERR || U1STAbits.OERR) {
        if (U1STAbits.OERR)
            U1STAbits.OERR = 0; // Clear overrun error
        //volatile char dummy = U1RXREG; // Clear RX register
        return;
    }

    char received = U1RXREG;

    // Echo the character back
    UART_SendChar(received);

    // Handle backspace
    if (received == '\b' && rxIndex > 0) {
        rxIndex--;
        return;
    }

    // Handle Enter (CR or LF)
    if (received == '\r' || received == '\n') {
        if (rxIndex > 0) {
            rxBuffer[rxIndex] = '\0';   // Null-terminate the string
            rxStringReady = 1;          // Mark string ready
        }
        return;
    }

    // Store character if space available
    if (rxIndex < RX_BUFFER_SIZE - 1) {
        rxBuffer[rxIndex++] = received;
    }
}