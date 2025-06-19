#ifndef SPI_H
#define SPI_H

#include <xc.h>
#include <math.h>

// Define the accelerometer chip selector
#define ACC_CS LATBbits.LATB3

// Initializes the SPI peripheral.
// Sets up necessary SPI registers and pin configuration.
void spi_setup(void);

// Sends a byte via SPI and returns the received byte.
// Parameters:
//   addr - the byte to transmit
// Returns:
//   the byte received from the SPI slave device
int spi_write(int addr);

// Configures the BMX055 accelerometer.
// Sets power mode to normal, bandwidth to 100Hz/32Hz,
// measurement range to ±4g, and enables filtering.
void accelerometer_config(void);

// Reads raw accelerometer data from the BMX055 via SPI.
// Acquires and processes X, Y, and Z axis data into global variables.
// Converts 13-bit values by discarding the lowest 3 bits.
void acquire_accelerometer_data(void);

#endif // SPI_H