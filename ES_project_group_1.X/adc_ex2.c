#include "config.h"

// Global variables
int init_UART_tx = 0;
char buffer_tx[20];
int buffer_index = 0;


void __attribute__ ((__interrupt__ , __auto_psv__)) _U1TXInterrupt(void){
    // Clear TX interrupt flag 
    IFS0bits.U1TXIF = 0;
    
    // Write to TX register the character specified by the buffer index.
    char current_char = buffer_tx[buffer_index];
    // Exit interrupt IF data from buffer was already sent entirely
    if (current_char == '\0'){
        buffer_index = 0;
        return;
    }  
    U1TXREG = current_char;
    buffer_index++;
}

void __attribute__ ((__interrupt__ , __auto_psv__)) _T2Interrupt(void) {
    // Clear interrupt flag
    IFS0bits.T2IF = 0; 
    // Change global variable for UART transmit initialization
    init_UART_tx = 1;
}


int main(void) {
    
    // Disable all analog pins
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    // For LED testing
    TRISGbits.TRISG9 = 0;   // pin G9 set as output (LED 2)
    
    // Configure only the analog pins to be used
    ANSELBbits.ANSB5 = 1;
    TRISBbits.TRISB5 = 1;
    // Set ENABLE pin to high to activate the sensor
    TRISBbits.TRISB4 = 0;   // pin B4 set as output (Enable sensor)
    LATBbits.LATB4 = 1;     // pin set as high
    
    // Setup UART
    uart_setup();
    
    // Setup ADC: Manual sampling & Automatic conversion
    AD1CON3bits.ADCS = 8;   // ADC Conversion Clock Select bits
    AD1CON1bits.ASAM = 0;   // Sampling begins when SAMP bit is set -> 0: Manual
    AD1CON3bits.SAMC = 16;  // Sample time 16 Tad
    AD1CON1bits.SSRC = 7;   // Conversion starts automatically -> 7: Automatic
    
    AD1CON2bits.VCFG = 0;   // Reference Voltage
    AD1CON2bits.CHPS = 0;   // Channel selection -> 0: CH0
    AD1CHS0bits.CH0NA = 0;  // Channel 0 Negative input select -> Vref-
    AD1CHS0bits.CH0SA = 5;  // Channel 0 Positive input select -> AN5
    
    AD1CON1bits.SIMSAM = 0;  // Sequential sampling
    AD1CON2bits.SMPI = 0;    // Interrupt after (n+1) number of sample/conversion operations
    AD1CON1bits.ADON = 1;    // Turn ON ADC
    
    // Send to UART frequency (5Hz)
    tmr_setup_period(TIMER2, 200, 1);

    while(1){
        // Read ADC: Manual sampling & Automatic conversion
        AD1CON1bits.SAMP = 1;           // Start sampling
        while (!AD1CON1bits.DONE);      // Wait for the conversion to complete
        int ADC_value = ADC1BUF0;       // Read the conversion result
        
        // Convert the result distance
        float voltage = (float) ADC_value * 3.3 / 1023;
        float distance = 2.34 - 4.74 * voltage + 4.06 * pow(voltage, 2) - 1.60 * pow(voltage, 3) + 0.24 * pow(voltage, 4);

        // For testing
        if (distance < 0.3){LATGbits.LATG9 = 1;}
        else {LATGbits.LATG9 = 0;} 
        
        // Initialize UART TX
        if (init_UART_tx){
            memset(buffer_tx, 0, sizeof(buffer_tx)); // Clear buffer
            sprintf(buffer_tx, "$Dist=%.2f", distance);
            U1TXREG = buffer_tx[buffer_index];
            buffer_index++;
            init_UART_tx = 0;
        }
    }
    
    return 0;
}


void uart_setup(){
    // Remapping of UART1 to pins of MicroBus 2
    RPINR18bits.U1RXR = 0x4B;
    RPOR0bits.RP64R = 1;
    IEC0bits.U1RXIE = 1;
    
    // UART Configuration
    U1BRG = 468;                // Obtained from (FCY/(16*baud_rate) - 1)
    U1STAbits.UTXISEL0 = 0;     // Interrupt after one TX Character is transmitted
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1TXIE = 1;        // Enable UART TX Interrupt
    IFS0bits.U1TXIF = 0;        // clear TX interrupt flag
    U1MODEbits.UARTEN = 1;      // Enable UART
    U1STAbits.UTXEN = 1;        // Enable U1TX
}



void tmr_wait_ms(int timer, int ms){

    // List of prescaler values: {ratio, bits}
    int prescaler[][2] = {
        {1, 0}, {8, 1}, {64, 2}, {256, 3}
    };
    
    // CONSTANTS FOR PRESCALER AND TIMER PR2 VALUE
    int prescaler_bits;
    long int pr_value;
    long int temp_pr_value;

    // OBTAINING THE PR VALUE
    for (int i = 0; i < 4; i++) {
        temp_pr_value = (FCY_PROP * ms) / (prescaler[i][0]);
        prescaler_bits = prescaler[i][1];
        if (temp_pr_value < MAX_VALUE_16_BITS) {break;}
    }
    
    // SETUP & WAITING
    while (temp_pr_value > 0){
        
        if(temp_pr_value >= MAX_VALUE_16_BITS) {pr_value = MAX_VALUE_16_BITS;}
        else {pr_value = temp_pr_value;}

        temp_pr_value = temp_pr_value - MAX_VALUE_16_BITS;
            
        // SETUP THE APPROPRIATE TIMER REGISTER VALUES
        tmr_set_register_values(timer, pr_value, prescaler_bits, 0);
        
        // BUSY WAITING FOR FLAG
        if (timer == 1){
            while(!IFS0bits.T1IF);      // busy waiting
            IFS0bits.T1IF = 0;          // Clear the interrupt flag
        }
        else if (timer == 2){
            while(!IFS0bits.T2IF);   
            IFS0bits.T2IF = 0;      
        }
        else if (timer == 3){
            while(!IFS0bits.T3IF);  
            IFS0bits.T3IF = 0;        
        }
        else if (timer == 4){
            while(!IFS1bits.T4IF);    
            IFS1bits.T4IF = 0;          
        }
    }
}


void tmr_set_register_values(int timer, int pr_value, int prescaler_bits,
                             int has_interrupt){
    if (timer == 1){
        TMR1 = 0;                                   // Reset timer counter
        PR1 = pr_value;                             // Timer will count up to this value
        T1CONbits.TCKPS = prescaler_bits;           // Prescaler bits (0,1,2,3)
        IFS0bits.T1IF = 0;                          // Clear the interrupt flag
        if (has_interrupt) {IEC0bits.T1IE = 1;}     // Enable timer interrupt
        T1CONbits.TON = 1;                          // Start the timer
    }
    else if (timer == 2){
        TMR2 = 0;                   
        PR2 = pr_value;                   
        T2CONbits.TCKPS = prescaler_bits;   
        IFS0bits.T2IF = 0;
        if (has_interrupt) {IEC0bits.T2IE = 1;}     
        T2CONbits.TON = 1;          
    }
    else if (timer == 3){
        TMR3 = 0;                   
        PR3 = pr_value;                   
        T3CONbits.TCKPS = prescaler_bits;   
        IFS0bits.T3IF = 0;
        if (has_interrupt) {IEC0bits.T3IE = 1;} 
        T3CONbits.TON = 1;          
    }
    else if (timer == 4){
        TMR4 = 0;                   
        PR4 = pr_value;                   
        T4CONbits.TCKPS = prescaler_bits;   
        IFS1bits.T4IF = 0;      
        if (has_interrupt) {IEC1bits.T4IE = 1;}  
        T4CONbits.TON = 1;          
    }
}

void tmr_setup_period(int timer, int ms, int has_interrupt){

    // List of prescaler values: {ratio, bits}
    int prescaler[][2] = {
        {1, 0}, {8, 1}, {64, 2}, {256, 3}
    };
    
    // CALCULATE THE PRESCALER & PR VALUES
    int prescaler_bits;
    long int pr_value;
    
    for (int i = 0; i < 4; i++) {
        pr_value = (FCY_PROP * ms) / (prescaler[i][0]);
        prescaler_bits = prescaler[i][1];
        if (pr_value < MAX_VALUE_16_BITS) {break;}
    }
    
    tmr_set_register_values(timer, pr_value, prescaler_bits, has_interrupt);
    
}


int tmr_wait_period(int timer){
    
    if (timer == 1){
        if (IFS0bits.T1IF){
            IFS0bits.T1IF = 0; 
            return 1;
        }
        else{
            while(!IFS0bits.T1IF); 
            IFS0bits.T1IF = 0; 
            return 0;
        }
    }

    if (timer == 2){
        if (IFS0bits.T2IF) {
            IFS0bits.T2IF = 0; 
            return 1;
        }
        else{
            while(!IFS0bits.T2IF);   
            IFS0bits.T2IF = 0;          
            return 0;
        }
    }

    if (timer == 3){
        if (IFS0bits.T3IF){
            IFS0bits.T3IF = 0;
            return 1;
        }
        else{
            while(!IFS0bits.T3IF) {};  
            IFS0bits.T3IF = 0;         
            return 0;
        }
    }

    if (timer == 4){
        if (IFS1bits.T4IF){
            IFS1bits.T4IF = 0;
            return 1;
        }
        else{
            while(!IFS1bits.T4IF) {};     
            IFS1bits.T4IF = 0;         
            return 0;
        }
    }
    
    return -1;
}