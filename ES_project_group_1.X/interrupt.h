#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "pwm.h"
#include "timer.h"
#include "xc.h"

// State definitions
#define STATE_WAIT_FOR_START 0
#define STATE_MOVING 1
#define STATE_EMERGENCY 2

// External state variables
extern int current_state;
extern int is_pwm_on;

// Interrupt handler declarations
void __attribute__((__interrupt__, __auto_psv__)) _INT1Interrupt(void);
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void);

// Interrupt initialization
void init_interrupts(void);

#endif /* INTERRUPT_H */
