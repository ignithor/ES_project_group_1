/* ===============================================================
 * File: adc.c                                                   =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/
#include "adc.h"
/*=================================================================*/
// variables
float distance_buffer[BUFFER_SIZE];
float battery_buffer[BUFFER_SIZE];
int buffer_index = 0;
int buffer_filled = 0;
int battery_buffer_index = 0;
int battery_buffer_filled = 0;
/*=================================================================*/
void setup_adc(void) {
    // Configure analog pins
    ANSELBbits.ANSB11 = 1;  // Battery voltage
    TRISBbits.TRISB11 = 1;
    ANSELBbits.ANSB15 = 1;  // IR sensor
    TRISBbits.TRISB15 = 1;
    // Set ENABLE pin to high to activate the sensor
    TRISBbits.TRISB4 = 0; // pin B4 set as output (Enable sensor)
    LATBbits.LATB4 = 1; // pin set as high
    
    // Setup ADC: Automatic sampling & conversion
    AD1CON3bits.ADCS = 8; // ADC Conversion Clock Select bits
    AD1CON1bits.ASAM = 1; // Sampling begins when SAMP bit is set -> 1: Automatic
    AD1CON3bits.SAMC = 16; // Sample time 16 Tad
    AD1CON1bits.SSRC = 7; // Conversion starts automatically -> 7: Automatic
    
    AD1CON2bits.VCFG = 0; // Reference Voltage
    AD1CON2bits.CHPS = 0; // Channel selection -> 0: CH0
    AD1CON1bits.SIMSAM = 0; // Sequential sampling
    AD1CON2bits.SMPI = 1; // Interrupt after 2 conversions
    
    // Automatic Scanning
    AD1CON2bits.CSCNA = 1; // Enable scanning
    AD1CSSLbits.CSS11 = 1; // Select AN11 (Battery voltage)
    AD1CSSLbits.CSS15 = 1; // Select AN15 (IR sensor)
            
    AD1CON1bits.ADON = 1; // Turn ON ADC 
}
/*=================================================================*/

/*=================================================================*/
float adc_distance(void) {
    // Read ADC: Automatic sampling & conversion
    while (!AD1CON1bits.DONE); // Wait for the conversion to complete
    int ADC_value = ADC1BUF1;       // Read IR sensor value
    
    // Convert the result to distance
    float voltage = (float) ADC_value * 3.3 / 1023.0;
    
    // Polynomial approximation for distance based on voltage
    float distance = 2.34 - 4.74 * voltage + 4.06 * pow(voltage, 2) - 1.60 * pow(voltage, 3) + 0.24 * pow(voltage, 4);
    
    // Store in circular buffer
    distance_buffer[buffer_index] = distance;
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;
    if (buffer_filled < BUFFER_SIZE) {
        buffer_filled++;
    }
    return distance;
}
/*=================================================================*/

/*=================================================================*/
int average_distance(void) {
    float sum = 0.0;
    int i;

    if (buffer_filled == 0) {
        return 0.0; // Avoid division by zero
    }

    for (i = 0; i < buffer_filled; i++) {
        sum += distance_buffer[i];
    }
    int result = (sum * 100) / buffer_filled; // Convert to cm

    return result;
}
/*=================================================================*/

/*=================================================================*/
float adc_battery_voltage(void) {
    // Read ADC: Automatic sampling & conversion
    while (!AD1CON1bits.DONE);      // Wait for the conversion to complete
    int ADC_value = ADC1BUF0;       // Read battery voltage
    
    // Convert ADC value to battery voltage (considering voltage divider)
    float bat_vsense = (float)ADC_value * 3.3 / 1023.0;
    float vbat = bat_vsense * 3;    // Due to voltage divider (1/3 ratio)
    
    // Store in circular buffer
    battery_buffer[battery_buffer_index] = vbat;
    battery_buffer_index = (battery_buffer_index + 1) % BUFFER_SIZE;
    if (battery_buffer_filled < BUFFER_SIZE) {
        battery_buffer_filled++;
    }
    
    return vbat;
}
/*=================================================================*/

/*=================================================================*/
float average_battery_voltage(void) {
    float sum = 0.0;
    int i;

    if (battery_buffer_filled == 0) {
        return 0.0;
    }

    for (i = 0; i < battery_buffer_filled; i++) {
        sum += battery_buffer[i];
    }
    
    return sum / battery_buffer_filled;
}
/*=================================================================*/