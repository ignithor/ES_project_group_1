/* ===============================================================
 * File:   uart.c                                                =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/

/*========================================================*/
//include
#include "uart.h"
/*========================================================*/
// External variables
extern volatile int g_speed;
extern volatile int g_yawrate;
/*========================================================*/
// TX Circular Buffer "handling transition"
static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;
/*========================================================*/
// Command Buffer to store multiple commands in case of receiving 
volatile char rxBuffer[RX_BUFFER_COUNT][RX_STRING_LENGTH] = {
    {0}};
volatile uint8_t rx_write_index = 0;
volatile uint8_t rx_read_index = 0;
volatile char temp_rx_buffer[RX_STRING_LENGTH] = {0};
volatile uint16_t rx_idx = 0;
/*========================================================*/
//extern user defined variables to handle current state
typedef enum {
    STATE_WAIT_FOR_START = 0,
    STATE_MOVING,
    STATE_EMERGENCY
} RobotState;
extern volatile RobotState current_state;
/*========================================================*/
//user defined variables for uart
typedef enum {
    CMD_PCREF,
    CMD_PCSTP,
    CMD_PCSTT,
    CMD_UNKNOWN
} CommandType;
/*========================================================*/
//declare uart configuration
/*========================================================*/
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
/*========================================================*/

/*========================================================*/
/* handling send string to the buffer applying non 
 * blocking mechanism later*/
/*========================================================*/
void UART_SendString(const char *str) {
    while (*str) {
        // Circular buffer management
        uint16_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
        if (next_head == tx_tail) break;
        tx_buffer[tx_head] = *str++;
        tx_head = next_head;
    }

    if (IEC0bits.U1TXIE == 0 && tx_head != tx_tail) {
        IEC0bits.U1TXIE = 1; // Enable the interrupt 
        }
}
/*========================================================*/

/*========================================================*/
// static function to handle the received command type
/*========================================================*/
static CommandType get_command_type(const char *input) {
    if (strncmp(input, "$PCREF,", 7) == 0) return CMD_PCREF;
    if (strncmp(input, "$PCSTP,", 7) == 0) return CMD_PCSTP;
    if (strncmp(input, "$PCSTT,", 7) == 0) return CMD_PCSTT;
    return CMD_UNKNOWN;
}
/*========================================================*/

/*========================================================*/
/* process command for uart and execute corresponding 
 * command based on current state process_pcref_command
 * will only parse the values to be updated later if we enter
 * moving state */
/*========================================================*/
void process_uart_command(const char *input) {
    switch (get_command_type(input)) {
        case CMD_PCREF:
            process_pcref_command(input);
            break;
        case CMD_PCSTP:
            if (current_state != STATE_EMERGENCY) {
                current_state = STATE_WAIT_FOR_START;
                set_motor_pwm(0, 0); // stop motors
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
            // we don't have logs at the moment here we must forward it to logs
            // at moment informing user by uart only.
            UART_SendString("$ERR,Unknown command*\r\n");
    }
}
/*========================================================*/


/*=============================================================*/
//function to parse speed and yawrate
/*============================================================*/
void process_pcref_command(const char *command) {
    int speed, yawrate;
    if (sscanf(command, "$PCREF,%d,%d*", &speed, &yawrate) == 2) {
        if ((speed >= -100 && speed <= 100) && (yawrate >= -100 && yawrate <= 100)) {
            g_speed = speed;                // this variable is global in main implementation
            g_yawrate = yawrate;            // this variable is global in main implementation
        }
    }
}
/*========================================================*/



/*========================================================*/
// --- Interrupt Service Routines (ISRs) ---
/*========================================================*/
/* TX interrupt will implement a circular buffer mechanism 
 * in sending data from the buffer that send string has filled
 * This ISR is short, non-blocking, and sends one character 
 * per activation.*/
/*========================================================*/
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;  // clear interrupt flag

    if (tx_head != tx_tail) {
        U1TXREG = tx_buffer[tx_tail];   // this will raise the flag again
        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;  
    }

    if (tx_head == tx_tail) {
        IEC0bits.U1TXIE = 0;
    }
}
/*========================================================*/
 
/*========================================================*/
/*
* RX interrupt handling receiveing char using circular buffers
* one buffer to receive the string command and the other buffer
* is to save the string to prevent any lose if we receive another
* command immediately before main loop process the last command
*/
/*========================================================*/
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0; // Clear the interrupt flag 
    if (U1STAbits.OERR) U1STAbits.OERR = 0;

    char received = U1RXREG;

    // Command buffer handling
    if (received == '\r' || received == '\n') {
        if (rx_idx > 0) {
            temp_rx_buffer[rx_idx] = '\0';

            uint8_t next_write = (rx_write_index + 1) % RX_BUFFER_COUNT;
            if (next_write != rx_read_index) {
                strncpy((char*) rxBuffer[rx_write_index], (const char*) temp_rx_buffer, RX_STRING_LENGTH);
                rx_write_index = next_write;
            }
            rx_idx = 0;
        }
    }
    else if (rx_idx < RX_STRING_LENGTH - 1) {
        temp_rx_buffer[rx_idx++] = received;
    }
}
/*========================================================*/