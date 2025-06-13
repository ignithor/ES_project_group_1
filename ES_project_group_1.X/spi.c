#include "spi.h"

//// Global variable definitions for magnetometer sensor data storage
//int x_values_mag[ARRAY_SIZE];  // X-axis magnetometer readings buffer
//int y_values_mag[ARRAY_SIZE];  // Y-axis magnetometer readings buffer
//int z_values_mag[ARRAY_SIZE];  // Z-axis magnetometer readings buffer
//int array_index_mag = 0;       // Current position in circular buffer

// Global variable definitions for accelerometer sensor data storage
int x_values_acc[ARRAY_SIZE];  // X-axis accelerometer readings buffer
int y_values_acc[ARRAY_SIZE];  // Y-axis accelerometer readings buffer
int z_values_acc[ARRAY_SIZE];  // Z-axis accelerometer readings buffer
int array_index_acc = 0;       // Current position in circular buffer

/**
 * @brief Transmits and receives a byte via SPI
 * 
 * Sends the given byte over SPI and returns the received response byte.
 * Implements a blocking mechanism to ensure transmission completion.
 * 
 * @param addr The byte to transmit via SPI
 * @return int The byte received from the SPI peripheral
 */
int spi_write(int addr) {
    // Wait until transmit buffer is not full
    while (SPI1STATbits.SPITBF == 1);
    // Send the byte
    SPI1BUF = addr;
    // Wait until a byte has been received
    while (SPI1STATbits.SPIRBF == 0);
    // Read and return the received byte
    int response = SPI1BUF;
    return response;
}

/**
 * @brief Initializes the SPI peripheral
 * 
 * Configures SPI1 pins, remaps IO, and sets up communication parameters
 * for interfacing with the sensor modules.
 */
void spi_setup(void) {
//    // Configure chip select pins for the magnetometer
//    TRISDbits.TRISD6 = 0;   // CS3: Magnetometer set as output
//    LATDbits.LATD6 = 1;     // Initialize CS high (inactive)

    // Configure chip select pins for the accelerometer
    TRISDbits.TRISD6 = 0;   // CS1: accelerometer set as output
    LATBbits.LATB0 = 1;     // Initialize CS high (inactive)

    
    // Configure SPI data and clock pins
    TRISAbits.TRISA1 = 1;   // RA1-RPI17 MISO (input)
    TRISFbits.TRISF12 = 0;  // RF12-RP108 SCK (output)
    TRISFbits.TRISF13 = 0;  // RF13-RP109 MOSI (output)
    
    // Remap SPI1 to physical pins on Microbus1 connector
    RPINR20bits.SDI1R = 0b0010001;   // Map MISO (SDI1) to RPI17
    RPOR12bits.RP109R = 0b000101;    // Map MOSI (SDO1) to RF13
    RPOR11bits.RP108R = 0b000110;    // Map SCK1 to RF12
    
    // Configure SPI1 parameters
    SPI1CON1bits.MSTEN = 1;      // Enable master mode
    SPI1CON1bits.MODE16 = 0;     // Use 8-bit mode (not 16-bit)
    SPI1CON1bits.CKP = 1;        // Set clock polarity (idle high)
    
    // Set SPI clock speed: FSCK = (FCY)/(PPR * SPR) = 72MHz/(4*3) = 6MHz
    SPI1CON1bits.PPRE = 0b10;    // Primary prescaler 4:1
    SPI1CON1bits.SPRE = 0b101;   // Secondary prescaler 3:1
    
    // Enable SPI module and clear overflow flag
    SPI1STATbits.SPIEN = 1;      // Enable SPI peripheral
    SPI1STATbits.SPIROV = 0;     // Clear receive overflow flag
}

///**
// * @brief Configures the magnetometer sensor
// * 
// * Initializes the BMX055 magnetometer by:
// * 1. Entering sleep mode
// * 2. Setting data rate to 25Hz (0b110)
// * 3. Configuring for active mode
// */
//void magnetometer_config(void) {
//    // Step 1: Put magnetometer in sleep mode first (required before changing settings)
//    LATDbits.LATD6 = 0;          // Enable chip select (active low)
//    spi_write(0x4B);             // Power control register address
//    spi_write(0x01);             // Sleep mode value
//    LATDbits.LATD6 = 1;          // Disable chip select
//    tmr_setup_period(TIMER2, 2);     //setup timer 2 to be 2 ms
//    tmr_wait_period(TIMER2);      // Wait 2ms for settings to apply
//    
//    // Step 2 & 3: Configure data rate and activate the sensor
//    LATDbits.LATD6 = 0;          // Enable chip select (active low)
//    spi_write(0x4C);             // Op mode register address
//    spi_write(0b00110000);       // Set data rate to 0b110 (25Hz)
//    LATDbits.LATD6 = 1;          // Disable chip select
//    tmr_setup_period(TIMER2, 2);     //setup timer 2 to be 2 ms
//    tmr_wait_period(TIMER2);      // Wait 2ms for settings to apply
//}
//
///**
// * @brief Acquires measurement data from the magnetometer
// * 
// * Reads the X, Y, and Z axis magnetic field measurements from the
// * BMX055 sensor and stores them in the respective circular buffers.
// * Each axis requires specific bit manipulation to obtain correct values.
// */
//void acquire_magnetometer_data(void) {
//    // Begin SPI transaction and select magnetometer register for reading
//    LATDbits.LATD6 = 0;                      // Enable chip select
//    int first_addr = 0x42;                   // First data register address
//    spi_write(first_addr | 0x80);            // Set MSB for read operation
//    
//    // Acquire X-axis magnetic data
//    uint8_t x_LSB_byte = spi_write(0x00);    // Read X-LSB register
//    uint8_t x_MSB_byte = spi_write(0x00);    // Read X-MSB register
//    // Process X-axis data: 13-bit value with 3 LSBs reserved
//    int x_value = ((x_MSB_byte << 8) | (x_LSB_byte & 0xF8)) / 8;
//    x_values_mag[array_index_mag] = x_value;
//    
//    // Acquire Y-axis magnetic data
//    uint8_t y_LSB_byte = spi_write(0x00);    // Read Y-LSB register
//    uint8_t y_MSB_byte = spi_write(0x00);    // Read Y-MSB register
//    // Process Y-axis data: 13-bit value with 3 LSBs reserved
//    int y_value = ((y_MSB_byte << 8) | (y_LSB_byte & 0xF8)) / 8;
//    y_values_mag[array_index_mag] = y_value;
//    
//    // Acquire Z-axis magnetic data
//    uint8_t z_LSB_byte = spi_write(0x00);    // Read Z-LSB register
//    uint8_t z_MSB_byte = spi_write(0x00);    // Read Z-MSB register
//    // Process Z-axis data: 15-bit value with 1 LSB reserved
//    int z_value = ((z_MSB_byte << 8) | (z_LSB_byte & 0xFE)) / 2;
//    z_values_mag[array_index_mag] = z_value;
//    
//    // End SPI transaction
//    LATDbits.LATD6 = 1;                      // Disable chip select
//    
//    // Update circular buffer index for next reading
//    array_index_mag = (array_index_mag + 1) % ARRAY_SIZE;
//}

void accelerometer_config(void) {
    // Step 1: Power on the accelerometer (exit suspend mode)
    LATBbits.LATB0 = 0;          // Enable chip select for accelerometer
    spi_write(0x11);             // PMU_LPW register (power mode config)
    spi_write(0x00);             // Normal mode 
    LATBbits.LATB0 = 1;          // Disable chip select
    tmr_setup_period(TIMER2, 2);
    tmr_wait_period(TIMER2);     // Wait 2ms

    // Step 2: Set data rate and bandwidth (e.g., 10Hz, filtered)
    LATBbits.LATB0 = 0;          // Enable chip select
    spi_write(0x10);             // PMU_BW register (bandwidth and ODR)
    spi_write(0x08);             // 100Hz ODR, 32Hz bandwidth (0x08)
    LATBbits.LATB0 = 1;          // Disable chip select
    tmr_setup_period(TIMER2, 2);
    tmr_wait_period(TIMER2);     // Wait 2ms
}

void acquire_accelerometer_data(void) {
    // Begin SPI transaction and select accelerometer register for reading
    LATBbits.LATB0 = 0;                      // Enable chip select (assumed for accelerometer)
    int first_addr = 0x02;                   // First data register address for accelerometer
    spi_write(first_addr | 0x80);            // Set MSB for read operation (read + auto-increment)

    // Acquire X-axis accelerometer data
    uint8_t x_LSB_byte = spi_write(0x00);    // Read X-LSB register
    uint8_t x_MSB_byte = spi_write(0x00);    // Read X-MSB register
    // Process X-axis data: 13-bit value, discard lower 3 bits
    int x_value = ((x_MSB_byte << 8) | (x_LSB_byte & 0xF8)) >> 3;
    x_values_acc[array_index_acc] = x_value;

    // Acquire Y-axis accelerometer data
    uint8_t y_LSB_byte = spi_write(0x00);    // Read Y-LSB register
    uint8_t y_MSB_byte = spi_write(0x00);    // Read Y-MSB register
    // Process Y-axis data: 13-bit value, discard lower 3 bits
    int y_value = ((y_MSB_byte << 8) | (y_LSB_byte & 0xF8)) >> 3;
    y_values_acc[array_index_acc] = y_value;

    // Acquire Z-axis accelerometer data
    uint8_t z_LSB_byte = spi_write(0x00);    // Read Z-LSB register
    uint8_t z_MSB_byte = spi_write(0x00);    // Read Z-MSB register
    // Process Z-axis data: 13-bit value, discard lower 3 bits
    int z_value = ((z_MSB_byte << 8) | (z_LSB_byte & 0xF8)) >> 3;
    z_values_acc[array_index_acc] = z_value;

    // End SPI transaction
    LATBbits.LATB0 = 1;                      // Disable chip select

    // Update circular buffer index for next reading
    array_index_acc = (array_index_acc + 1) % ARRAY_SIZE;
}


/**
 * @brief Calculates the average of an array of integer values
 * 
 * Computes arithmetic mean of the values in the given array.
 * Used for smoothing sensor data by averaging recent readings.
 * 
 * @param values Array of integer values to average
 * @param size Number of elements in the array
 * @return double The arithmetic mean of the values
 */
double calculate_average(int values[], int size) {
    double sum = 0.0;
    // Calculate sum of all values in array
    for(int i=0; i<size; i++) {
        sum += values[i];
    }
    // Return average value
    return sum/size;
}