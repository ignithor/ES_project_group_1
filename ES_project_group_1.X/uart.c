#include "xc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "interrupt.h"
// TX Circular Buffer
static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

// Command Buffer
volatile char rxBuffer[RX_BUFFER_COUNT][RX_STRING_LENGTH] = {{0}};
volatile uint8_t rx_write_index = 0;
volatile uint8_t rx_read_index = 0;
volatile char temp_rx_buffer[RX_STRING_LENGTH] = {0};
volatile uint16_t rx_idx = 0;

// External variables
extern volatile int g_speed;
extern volatile int g_yawrate;


typedef enum {
    CMD_PCREF,
    CMD_PCSTP,
    CMD_PCSTT,
    CMD_UNKNOWN
} CommandType;


void UART_Initialize(void) {
    // PPS configuration
    RPOR0bits.RP64R = 1; // Set RP64 as U1TX
    RPINR18bits.U1RXR = 75; // Set RP75 as U1RX

    // UART mode configuration
    U1MODEbits.STSEL = 0;
    U1MODEbits.PDSEL = 0;
    U1MODEbits.ABAUD = 0;
    U1MODEbits.BRGH = 0;
    U1BRG = BRGVAL;

    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 0;

    IEC0bits.U1RXIE = 1;
    IEC0bits.U1TXIE = 0;

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
}

void UART_SendString(const char *str) {
    while (*str) {
        uint16_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
        if (next_head == tx_tail) break;
        tx_buffer[tx_head] = *str++;
        tx_head = next_head;
    }

    if (IEC0bits.U1TXIE == 0 && tx_head != tx_tail) {
        IEC0bits.U1TXIE = 1;
        IFS0bits.U1TXIF = 1;
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
            g_speed = speed;
            g_yawrate = yawrate;
        }
    }
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;

    if (tx_head != tx_tail) {
        U1TXREG = tx_buffer[tx_tail];
        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
    }

    if (tx_head == tx_tail) {
        IEC0bits.U1TXIE = 0;
    }
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    if (U1STAbits.OERR) U1STAbits.OERR = 0;

    char received = U1RXREG;

    // Echo handling
    uint16_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
    if (next_head != tx_tail) {
        tx_buffer[tx_head] = received;
        tx_head = next_head;
        if (!IEC0bits.U1TXIE) {
            IEC0bits.U1TXIE = 1;
            IFS0bits.U1TXIF = 1;
        }
    }

    // Command buffer handling
    if (received == '\r' || received == '\n') {
        if (rx_idx > 0) {
            temp_rx_buffer[rx_idx] = '\0';
            
            uint8_t next_write = (rx_write_index + 1) % RX_BUFFER_COUNT;
            if (next_write != rx_read_index) {
                strncpy((char*)rxBuffer[rx_write_index], (const char*)temp_rx_buffer, RX_STRING_LENGTH);
                rx_write_index = next_write;
            }
            rx_idx = 0;
        }
    } 
    else if (rx_idx < RX_STRING_LENGTH - 1) {
        temp_rx_buffer[rx_idx++] = received;
    }
}