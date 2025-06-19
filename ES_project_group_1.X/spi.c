/* ===============================================================
 * File: spi.h                                                   =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/
/*================================================================*/
#include "spi.h"
/*================================================================*/

/*================================================================*/
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
/*================================================================*/


/*================================================================*/
void spi_setup(void) {
    // Configure chip select pins for the accelerometer
    TRISBbits.TRISB3 = 0; // CS1: accelerometer set as output
    ACC_CS = 1; // Initialize CS high (inactive)

    // Configure SPI data and clock pins
    TRISAbits.TRISA1 = 1; // MISO (input)
    TRISFbits.TRISF12 = 0; // SCK (output)
    TRISFbits.TRISF13 = 0; // MOSI (output)

    // Remap SPI1 to physical pins on Microbus1 connector
    RPINR20bits.SDI1R = 0b0010001; // Map MISO (SDI1) to RPI17
    RPOR12bits.RP109R = 0b000101; // Map MOSI (SDO1) to RF13
    RPOR11bits.RP108R = 0b000110; // Map SCK1 to RF12

    // Configure SPI1 parameters
    SPI1CON1bits.MSTEN = 1; // Enable master mode
    SPI1CON1bits.MODE16 = 0; // Use 8-bit mode
    SPI1CON1bits.CKP = 1; // Set clock polarity

    // Set SPI clock speed: FSCK = (FCY)/(PPR * SPR) = 72MHz/(4*3) = 6MHz
    SPI1CON1bits.PPRE = 0b10; // Primary prescaler 4:1
    SPI1CON1bits.SPRE = 0b101; // Secondary prescaler 3:1

    // Enable SPI module and clear overflow flag
    SPI1STATbits.SPIEN = 1; // Enable SPI peripheral
    SPI1STATbits.SPIROV = 0; // Clear receive overflow flag
}
/*================================================================*/


/*================================================================*/
void accelerometer_config(void) {
    // Power on the accelerometer (exit suspend mode)
    ACC_CS = 0; // Enable chip select for accelerometer
    spi_write(0x11); // PMU_LPW register (power mode config)
    spi_write(0x00); // Set normal mode 
    ACC_CS = 1; // Disable chip select

    // Configure bandwidth 
    ACC_CS = 0;
    unsigned int address_bandwidth = 0x10; // PMU_BW register (bandwidth and ODR)
    spi_write(address_bandwidth); 
    spi_write(0x08); // Set 100Hz ODR, 32Hz bandwidth
    ACC_CS = 1;

    // Configure measurement range
    ACC_CS = 0;
    unsigned int address_measurement_range = 0x0F; 
    spi_write(address_measurement_range);
    spi_write(0x03); // Set ±2g range for best precision (0.98 mg/LSB)
    ACC_CS = 1;

    // Configure filtering
    ACC_CS = 0;
    unsigned int address_filtering = 0x13; 
    spi_write(address_filtering);
    spi_write(0b0); // Enable filtering and shadowing
    ACC_CS = 1;
}
/*================================================================*/


/*================================================================*/
void acquire_accelerometer_data(int *x_acc, int *y_acc, int *z_acc) {
    ACC_CS = 0; 

    int acc_first_address = 0x02; // First data register address for accelerometer
    spi_write(acc_first_address | 0x80); // Set MSB for read operation

    // Acquire X-axis accelerometer data
    uint8_t x_LSB_byte = spi_write(0x02); // Read X-LSB register
    uint8_t x_MSB_byte = spi_write(0x03); // Read X-MSB register
    // Process X-axis data: 12-bit value, discard lower 4 bits
    int x_value = ((x_MSB_byte << 8) | (x_LSB_byte & 0xF8)) / 16;
    *x_acc = (int) round(0.98 * x_value); // Change unit into mg

    // Acquire Y-axis accelerometer data
    uint8_t y_LSB_byte = spi_write(0x04); // Read Y-LSB register
    uint8_t y_MSB_byte = spi_write(0x05); // Read Y-MSB register
    // Process Y-axis data: 12-bit value, discard lower 4 bits
    int y_value = ((y_MSB_byte << 8) | (y_LSB_byte & 0xF8)) / 16;
    *y_acc = (int) round(0.98 * y_value); // Change unit into mg

    // Acquire Z-axis accelerometer data
    uint8_t z_LSB_byte = spi_write(0x06); // Read Z-LSB register
    uint8_t z_MSB_byte = spi_write(0x07); // Read Z-MSB register
    // Process Z-axis data: 12-bit value, discard lower 4 bits
    int z_value = ((z_MSB_byte << 8) | (z_LSB_byte & 0xF8)) / 16;
    *z_acc = (int) round(0.98 * z_value); // Change unit into mg

    ACC_CS = 1; 
}
/*================================================================*/