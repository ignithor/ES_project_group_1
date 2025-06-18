#ifndef UART_H
#define	UART_H

#include <xc.h> 

/* Baud Rate Configuration */
#define FCY             72000000
#define BAUDRATE        9600
#define BRGVAL          ((FCY/BAUDRATE)/16)-1

/* Command Buffer Configuration */
#define RX_BUFFER_COUNT 10   // Buffer 10 commands
#define RX_STRING_LENGTH 32  // Max command length

/* Buffer Sizes - Justified */
#define TX_BUFFER_SIZE 128

/* Public Function Declarations */
void UART_Initialize(void);
void UART_SendString(const char *str);
void process_uart_command(const char *input);
void process_pcref_command(const char *command);

#endif	/* UART_H */