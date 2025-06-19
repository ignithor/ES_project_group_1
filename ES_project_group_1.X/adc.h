/* ===============================================================
 * File: adc.h                                                   =
 * Author: group 1                                               =   
 * Paul Pham Dang                                                =   
 * Waleed Elfieky                                                =
 * Yui Momiyama                                                  =
 * Mamoru Ota                                                    =
 * ===============================================================*/
/*=================================================================*/
#ifndef ADC_H
#define	ADC_H
/*=================================================================*/
//includes
#include <xc.h>
#include <math.h>
/*=================================================================*/
//buffer size
#define BUFFER_SIZE 5
/*=================================================================*/
// Configures the Analog-to-Digital Converter (ADC).
void setup_adc(void);
/*=================================================================*/
// Converts ADC reading to a distance measurement.
float adc_distance(void);
/*=================================================================*/
// Calculates the average of multiple distance readings.
int average_distance(void);
/*=================================================================*/
// Converts ADC reading to battery voltage.
float adc_battery_voltage(void);
/*=================================================================*/
// Calculates the average of multiple battery voltage readings.
float average_battery_voltage(void);
/*=================================================================*/
#endif
/*=================================================================*/