#ifndef SPI_H
#define SPI_H

#include <xc.h>
#include "timer.h"

// Define buffer size for magnetometer data (using the last 5 measurements)
#define ARRAY_SIZE 5

// Extern declarations for magnetometer data buffers. These arrays store the most recent
// measurements for the x, y, and z axes respectively.
extern int x_values[ARRAY_SIZE];
extern int y_values[ARRAY_SIZE];
extern int z_values[ARRAY_SIZE];

// Global index used for circular buffer management for magnetometer data
extern int array_index;

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

/**
 * @brief Configure the magnetometer.
 * 
 * Sets the magnetometer to active mode and configures the data rate to 25Hz.
 */
void magnetometer_config(void);

/**
 * @brief Acquire data from the magnetometer.
 * 
 * Reads magnetic data from the sensor registers and stores them in the corresponding buffers.
 */
void acquire_magnetometer_data(void);

/**
 * @brief Calculate the average of an array of integer values.
 * 
 * Computes the average of the provided measurements.
 * 
 * @param values The array of integer values.
 * @param size The number of elements in the array.
 * @return double The computed average.
 */
double calculate_average(int values[], int size);

#endif // SPI_H