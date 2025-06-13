#include <xc.h>
#include "pwm.h"

void init_pwm() {
    // Set RD1 - RD4 as output
    TRISDbits.TRISD1 = 0; // RD1 = OC1
    TRISDbits.TRISD2 = 0; // RD2 = OC2
    TRISDbits.TRISD3 = 0; // RD3 = OC3
    TRISDbits.TRISD4 = 0; // RD4 = OC4

    // Remap RP pins to Output Compare modules
    RPOR0bits.RP65R = 0b010000; // RD1 = RP65 ? OC1 (Left Backward)
    RPOR1bits.RP66R = 0b010001; // RD2 = RP66 ? OC2 (Left Forward)
    RPOR1bits.RP67R = 0b010010; // RD3 = RP67 ? OC3 (Right Backward)
    RPOR2bits.RP68R = 0b010011; // RD4 = RP68 ? OC4 (Right Forward)

    // Disable all PWM initially
    OC1CON1 = OC1CON2 = 0;
    OC2CON1 = OC2CON2 = 0;
    OC3CON1 = OC3CON2 = 0;
    OC4CON1 = OC4CON2 = 0;

    // Setup all PWM modules
    setup_oc_module(&OC1CON1, &OC1CON2, PWM_PERIOD); // Left Backward
    setup_oc_module(&OC2CON1, &OC2CON2, PWM_PERIOD); // Left Forward
    setup_oc_module(&OC3CON1, &OC3CON2, PWM_PERIOD); // Right Backward
    setup_oc_module(&OC4CON1, &OC4CON2, PWM_PERIOD); // Right Forward
}

void setup_oc_module(volatile unsigned int* con1, volatile unsigned int* con2, unsigned int period) {
    *con1 = 0;
    *con2 = 0;
    *(con2 + 1) = period;    // OCxRS
    *con2 |= 0x1F;           // SYNCSEL = OCx itself
    *con1 |= (0x07 << 0);    // OCTSEL = Peripheral clock (Fcy)
    *con1 |= (0x06 << 0);    // Edge-aligned PWM mode
}

// Duty cycle can be set from 0 to PWM_PERIOD
void set_pwm_duty(volatile unsigned int* oc_r, unsigned int duty) {
    if (duty > PWM_PERIOD) duty = PWM_PERIOD;
    *oc_r = duty;
}

// Set PWM for left and right motors (positive: forward, negative: backward)
void set_motor_pwm(int left_pwm, int right_pwm) {
    // Left motor
    if (left_pwm >= 0) {
        set_pwm_duty(&OC1R, left_pwm);  // Left forward (RD1)
        set_pwm_duty(&OC2R, 0);         // Left backward (RD2) = 0
    } else {
        set_pwm_duty(&OC1R, 0);
        set_pwm_duty(&OC2R, -left_pwm); // Left backward
    }

    // Right motor
    if (right_pwm >= 0) {
        set_pwm_duty(&OC3R, right_pwm); // Right forward (RD3)
        set_pwm_duty(&OC4R, 0);         // Right backward (RD4) = 0
    } else {
        set_pwm_duty(&OC3R, 0);
        set_pwm_duty(&OC4R, -right_pwm); // Right backward
    }
}

void stop_motors() {
    set_motor_pwm(0, 0);
}
