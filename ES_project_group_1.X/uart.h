#ifndef XC_HEADER_UART_H
#define	XC_HEADER_UART_H

#include <xc.h> 

/* Baud Rate Configuration */
#define FCY             72000000
#define BAUDRATE        9600
#define BRGVAL          ((FCY/BAUDRATE)/16)-1

/* Buffer Sizes - Justified */
// RX buffer must hold the longest command, e.g., "$RATE,10*". 32 is sufficient and saves RAM. 
#define RX_BUFFER_SIZE 32
// TX buffer should be large enough to hold multiple messages (e.g., echo, response, MAG data)
// to prevent the main loop from ever having to wait for space. 128 is a safe size. 
#define TX_BUFFER_SIZE 128

/* Public Function Declarations */

/**
 * @brief Initializes UART module, enables RX interrupt.
 */
void UART_Initialize(void);

/**
 * @brief Queues a null-terminated string for non-blocking transmission via interrupts.
 * @param str Pointer to the string.
 */
void UART_SendString(const char *str);

/**
 * @brief Parses and acts on a command string received from UART. 
 * @param input The command string to process.
 */
void process_uart_command(const char *input);

//function to check and parse pcref command
void process_pcref_command(const char *command);

#endif	/* XC_HEADER_UART_H */