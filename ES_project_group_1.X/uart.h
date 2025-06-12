#ifndef XC_HEADER_UART_H
#define	XC_HEADER_UART_H

#include <xc.h> // include processor files - each processor file is guarded.  

/* Baud Rate Configuration */
#define FCY             72000000
#define BAUDRATE        9600
#define BRGVAL          ((FCY/BAUDRATE)/16)-1
#define RX_BUFFER_SIZE 128


/* Buffer Configuration */
#define UART_RX_BUFFER_SIZE     128

/* Function Declarations */

/**
 * @brief Initializes UART module with configured baud rate
 * @note Enables both transmit and receive interrupts
 */
void UART_Initialize(void);

/**
 * @brief Sends a single character via UART
 * @param data Character to be transmitted
 */
void UART_SendChar(char data);

/**
 * @brief Sends a null-terminated string via UART
 * @param str Pointer to string to be transmitted
 */
void UART_SendString(const char *str);

/**
 * @brief Receives a single character (blocking)
 * @return Received character
 */
char UART_ReceiveChar(void);

/**
 * @brief Checks if data is available in receive buffer
 * @return 1 if data available, 0 otherwise
 */
uint8_t UART_DataAvailable(void);

/**
 * @brief Checks for UART receive errors
 * @return 1 if error occurred, 0 otherwise
 */
uint8_t UART_ReceiveErrorCheck(void);

/**
 * @brief Receives a string until newline (blocking)
 * @param buffer Pointer to buffer for storing received string
 * @param maxLength Maximum number of characters to receive
 */
void UART_ReceiveString(char* buffer, uint16_t maxLength);

void process_rate_command(const char* rateStr);
void process_uart_command(const char *input);

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void);
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void);

#endif	/* XC_HEADER_UART_H */