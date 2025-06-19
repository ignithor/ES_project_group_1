/* ===============================================================
 * File: uart.h                                                  =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/

#ifndef UART_H
#define	UART_H

#include <xc.h> 
#include "pwm.h"
#include "xc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Baud Rate Configuration */
#define FCY             72000000
#define BAUDRATE        9600
#define BRGVAL          ((FCY/BAUDRATE)/16)-1

/* Buffer Sizes */

/* Command Buffer Configuration */

// All these messages are sent by the robot to the PC
// $MBATT,v_batt* at 1Hz
// $MDIST,distance* at 10Hz
// $MACC,x,y,z* at 10Hz
// $MACK,1* (Acknowledgment of command success)
// $MACK,0* (Acknowledgment of command failure)
// $MEMRG,1* (Emergency state)
// $MEMRG,0* (End Emergency state)
// While UART send at 3.2 Mhz
#define RX_BUFFER_COUNT 8   // Buffer 8 commands

// We chose 32 bytes for the max string length
// This is enough for our commands, the longest theoretical command is:
// $MACC,-2147483648,-2147483648,-2147483648*
// However, in practice, we can't reach these values with our sensors,
// The maximum is closer to a result like $MACC,-600,-400,1000*
// Which is 21 characters long, so 32 bytes is more than enough.
#define RX_STRING_LENGTH 32  // Max command length

// We chose 128 bytes for the TX buffer
// This message is the longest that you can send : $PCREF,-100,-100* 
// It's 17 characters long, so 128 bytes can hold at least 7 full messages
// It's more than enough for our needs.
#define TX_BUFFER_SIZE 128


/* Public Function Declarations */
void UART_Initialize(void);
void UART_SendString(const char *str);
void process_uart_command(const char *input);
void process_pcref_command(const char *command);

#endif	/* UART_H */