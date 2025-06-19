#ifndef SPI_H
#define SPI_H

#include <xc.h>
#include <math.h>

// Define the accelerometer chip selector
#define ACC_CS LATBbits.LATB3

// Extern declarations for accelerometer data buffers. These arrays store the most recent
// measurements for the x, y, and z axes respectively.
extern int x_acc;
extern int y_acc;
extern int z_acc;

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

// Applies axis-specific offset correction and scaling to accelerometer values.
// Parameters:
//   values - raw accelerometer reading to filter
//   axis   - axis label ('x', 'y', or 'z')
// Returns:
//   Bias-corrected and scaled acceleration value in mg
// int filter_accelerometer(int values, char axis);

#endif // SPI_H