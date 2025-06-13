#ifndef XC_HEADER_TEMPLATE_H
#define XC_HEADER_TEMPLATE_H

#include <xc.h>

// PWM period: 10kHz (72MHz / 7200 = 10kHz)
#define PWM_PERIOD 7200

/**
 * Initialize PWM modules for motor control
 * Sets up RD1-RD4 as outputs and configures Output Compare modules
 * for PWM generation at 10kHz
 */
void init_pwm();

/**
 * Setup individual Output Compare module for PWM generation
 * @param con1 Pointer to OCxCON1 register
 * @param con2 Pointer to OCxCON2 register
 * @param period PWM period value
 */
void setup_oc_module(volatile unsigned int* con1, volatile unsigned int* con2, unsigned int period);

/**
 * Set PWM duty cycle for a specific Output Compare module
 * @param oc_r Pointer to OCxR register
 * @param duty Duty cycle value (0 to PWM_PERIOD)
 */
void set_pwm_duty(volatile unsigned int* oc_r, unsigned int duty);

/**
 * Set PWM for both motors
 * @param left_pwm Left motor PWM value (positive: forward, negative: backward)
 * @param right_pwm Right motor PWM value (positive: forward, negative: backward)
 */
void set_motor_pwm(int left_pwm, int right_pwm);

/**
 * Stop both motors by setting PWM to 0
 */
void stop_motors();

/**
 * Remap Programmable pins for Output Compare, sets RD1 - RD4 bits as an output
 */
void remap_pins_drive_forward();

/**
 * Remaps pins to set OC1 for Output
 */
void remap_pins_drive_forward();

/**
 * Remaps pins to set OC1 to RD4 and OC2 to RD2
 */
void remap_pins_turning();

/**
 * Generates PWM to go forward.
 * Sets up Output Compare OC1 by:
 * 1. Clearing the bits
 * 2. remaps the output pins to send OC1 signal
 * 3. Selects Peripheral Clock TCY = 72 Mhz
 * 4. User defined OC1R -> Duty cycle from 40% to 100% for driving
 * 5. User defined OC1Rs -> Period Count for PWM - right now 7200, which gives
 *    1/Period = 10KHz
 */
void drive_forward();

/**
 * Generate PWM for OC1 and OC2 to drive right.
 * Sets up OC1 and OC2 by:
 *      1. Clear bits
 *      2. remaps bits for turning action
 *      3. Sets up OC1 (40% duty)  and OC2 (60% duty)
 *      4. Start generation.
 */
void drive_right();

/**
 *  Stops PWM Generation, by clearing the bits for OC1 and OC2
 */
void drive_stop();

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// TODO If C++ is being used, regular C code needs function names to have C
// linkage so the functions can be used by the c code.

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XC_HEADER_TEMPLATE_H */
