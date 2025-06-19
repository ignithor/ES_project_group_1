/* ===============================================================
 * File: pwm.c                                                   =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/

/*================================================================*/
#include "pwm.h"
/*================================================================*/

/*================================================================*/
// Initialize PWM modules for motor control
void init_pwm(void) {
    // Configure RD1-RD4 as outputs for motor control
    TRISDbits.TRISD1 = 0; // RD1 = OC1 (Left Backward)
    TRISDbits.TRISD2 = 0; // RD2 = OC2 (Left Forward)
    TRISDbits.TRISD3 = 0; // RD3 = OC3 (Right Backward)
    TRISDbits.TRISD4 = 0; // RD4 = OC4 (Right Forward)

    // Remap RP pins to Output Compare modules
    RPOR0bits.RP65R = 0b010000; // RD1 = RP65 -> OC1 (Left Backward)
    RPOR1bits.RP66R = 0b010001; // RD2 = RP66 -> OC2 (Left Forward)
    RPOR1bits.RP67R = 0b010010; // RD3 = RP67 -> OC3 (Right Backward)
    RPOR2bits.RP68R = 0b010011; // RD4 = RP68 -> OC4 (Right Forward)

    // Disable all PWM modules initially
    OC1CON1 = OC1CON2 = 0; // Left Backward
    OC2CON1 = OC2CON2 = 0; // Left Forward
    OC3CON1 = OC3CON2 = 0; // Right Backward
    OC4CON1 = OC4CON2 = 0; // Right Forward

    // Initialize all PWM modules with specified period
    setup_oc_module(&OC1CON1, &OC1CON2, PWM_PERIOD); // Left Backward
    setup_oc_module(&OC2CON1, &OC2CON2, PWM_PERIOD); // Left Forward
    setup_oc_module(&OC3CON1, &OC3CON2, PWM_PERIOD); // Right Backward
    setup_oc_module(&OC4CON1, &OC4CON2, PWM_PERIOD); // Right Forward
}
/*================================================================*/

/*================================================================*/
// Configure an Output Compare module for PWM generation
/*================================================================*/
void setup_oc_module(volatile unsigned int* con1, volatile unsigned int* con2, unsigned int period) {
    // Clear control registers
    *con1 = 0;
    *con2 = 0;

    // Set period value
    *(con2 + 1) = period; // OCxRS = period

    // Configure synchronization
    *con2 |= 0x1F; // SYNCSEL = OCx itself

    // Configure clock source and PWM mode
    *con1 |= (0x07 << 0); // OCTSEL = Peripheral clock (Fcy)
    *con1 |= (0x06 << 0); // Edge-aligned PWM mode
}
/*================================================================*/

/*================================================================*/
// Set PWM duty cycle for a specific Output Compare module
/*================================================================*/
void set_pwm_duty(volatile unsigned int* oc_r, unsigned int duty) {
    // Limit duty cycle to PWM_PERIOD
    if (duty > PWM_PERIOD) {
        duty = PWM_PERIOD;
    }
    *oc_r = duty;
}
/*================================================================*/

/*================================================================*/
// Set PWM for both motors
/*================================================================*/
void set_motor_pwm(int left_pwm, int right_pwm) {
    // Control left motor
    if (left_pwm >= 0) {
        // Forward direction
        set_pwm_duty(&OC1R, left_pwm); // Left backward = 0
        set_pwm_duty(&OC2R, 0); // Left forward = duty
    } else {
        // Backward direction
        set_pwm_duty(&OC1R, 0); // Left backward = |duty|
        set_pwm_duty(&OC2R, -left_pwm); // Left forward = 0
    }

    // Control right motor
    if (right_pwm >= 0) {
        // Forward direction
        set_pwm_duty(&OC3R, right_pwm); // Right forward = duty
        set_pwm_duty(&OC4R, 0); // Right backward = 0
    } else {
        // Backward direction
        set_pwm_duty(&OC3R, 0); // Right forward = 0
        set_pwm_duty(&OC4R, -right_pwm); // Right backward = |duty|
    }
}
/*================================================================*/

/*================================================================*/
/* Function to control motor speed with a value between -100 to 100
we have ensured this value in parsing stage
*/
/*================================================================*/
void control_motors(int speed, int yawrate) {
    // 1. Map speed and yawrate from [-100, 100] to [-PWM_PERIOD, PWM_PERIOD]
    long speed_pwm = (long) speed * PWM_PERIOD / 100;
    long yaw_pwm = (long) yawrate * PWM_PERIOD / 100;

    // 2. Mix the speed and yaw signals to get individual motor PWMs
    // For an anti-clockwise (positive yaw) turn, we want the right motor to go faster
    // and the left motor to go slower.
    long left_pwm_raw = speed_pwm - yaw_pwm;
    long right_pwm_raw = speed_pwm + yaw_pwm;

    // 3. check PWM values to the valid range [-PWM_PERIOD, PWM_PERIOD] if not set it to the limit
    // to prevent overflow and ensure proper motor driver operation.
    int left_pwm_final = left_pwm_raw;
    if (left_pwm_final > PWM_PERIOD) {
        left_pwm_final = PWM_PERIOD;
    } else if (left_pwm_final < -PWM_PERIOD) {
        left_pwm_final = -PWM_PERIOD;
    }

    int right_pwm_final = right_pwm_raw;
    if (right_pwm_final > PWM_PERIOD) {
        right_pwm_final = PWM_PERIOD;
    } else if (right_pwm_final < -PWM_PERIOD) {
        right_pwm_final = -PWM_PERIOD;
    }

    // 4. Call the low-level motor control function with the calculated PWM values
    set_motor_pwm(left_pwm_final, right_pwm_final);
}
