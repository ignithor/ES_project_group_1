#ifndef SPI_H
#define SPI_H

#include <xc.h>
#include <math.h>

// Define the accelerometer chip selector
#define ACC_CS LATBbits.LATB3

// Extern declarations for accelerometer data buffers. These arrays store the most recent
// measurements for the x, y, and z axes respectively.
extern int x_values_acc;
extern int y_values_acc;
extern int z_values_acc;

/**
 * @brief Initialize the SPI peripheral.
 * 
 * Configures SPI1 including pin remapping and clock settings for communication with sensors.
 */
void spi_setup(void);

/**
 * @brief Transmit and receive a byte over SPI.
 * 
 * Sends the specified address/command and returns the received data.
 * 
 * @param addr The address/command byte to be sent.
 * @return int The byte received via SPI.
 */
int spi_write(int addr);
void accelerometer_config(void);
void acquire_accelerometer_data(void);

/**
 * @brief Calculate the average of an array of integer values.
 * 
 * Computes the average of the provided measurements.
 * 
 * @param values The array of integer values.
 * @param size The number of elements in the array.
 * @return double The computed average.
 */
// double calculate_average(int values[], int size);
int filter_accelerometer(int values, char axis);

#endif // SPI_H