#include "adc.h"

void setup_adc(void){
    
    // Configure only the analog pins to be used
    ANSELBbits.ANSB15 = 1;
    TRISBbits.TRISB15 = 1;
    // Set ENABLE pin to high to activate the sensor
    TRISBbits.TRISB4 = 0;   // pin B4 set as output (Enable sensor)
    LATBbits.LATB4 = 1;     // pin set as high
    
    // Setup ADC: Manual sampling & Automatic conversion
    AD1CON3bits.ADCS = 8;   // ADC Conversion Clock Select bits
    AD1CON1bits.ASAM = 0;   // Sampling begins when SAMP bit is set -> 0: Manual
    AD1CON3bits.SAMC = 16;  // Sample time 16 Tad
    AD1CON1bits.SSRC = 7;   // Conversion starts automatically -> 7: Automatic
    
    AD1CON2bits.VCFG = 0;   // Reference Voltage
    AD1CON2bits.CHPS = 0;   // Channel selection -> 0: CH0
    AD1CHS0bits.CH0NA = 0;  // Channel 0 Negative input select -> Vref-
    AD1CHS0bits.CH0SA = 15;  // Channel 0 Positive input select -> AN5
    
    AD1CON1bits.SIMSAM = 0;  // Sequential sampling
    AD1CON2bits.SMPI = 0;    // Interrupt after (n+1) number of sample/conversion operations
    AD1CON1bits.ADON = 1;    // Turn ON ADC
}