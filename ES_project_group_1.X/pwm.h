#ifndef PWM_H
#define PWM_H

#include <xc.h>

// PWM period: 10kHz (72MHz / 7200 = 10kHz)
#define PWM_PERIOD 7200

// Initialize PWM modules for motor control
void init_pwm();

// Setup individual Output Compare module for PWM generation
void setup_oc_module(volatile unsigned int* con1, volatile unsigned int* con2, unsigned int period);

// Set PWM duty cycle for a specific Output Compare module
void set_pwm_duty(volatile unsigned int* oc_r, unsigned int duty);

// Set PWM for both motors
void set_motor_pwm(int left_pwm, int right_pwm);

// Stop both motors by setting PWM to 0
void stop_motors();

#endif /* PWM_H */
