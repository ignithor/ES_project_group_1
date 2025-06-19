/* ===============================================================
 * File: interrupt.h                                             =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "pwm.h"
#include "timer.h"
#include <xc.h>

// Interrupt handler declarations
void __attribute__((__interrupt__, __auto_psv__)) _INT1Interrupt(void);
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void);

// Interrupt initialization
void init_interrupts(void);

#endif /* INTERRUPT_H */
