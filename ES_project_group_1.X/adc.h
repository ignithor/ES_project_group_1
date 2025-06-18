#ifndef ADC_H
#define	ADC_H

#include <xc.h> // include processor files - each processor file is guarded.
#include <math.h>

#define BUFFER_SIZE 5

extern float distance_buffer[BUFFER_SIZE];
extern float battery_buffer[BUFFER_SIZE];
extern int buffer_index;
extern int buffer_filled;
extern int battery_buffer_index;
extern int battery_buffer_filled;

void setup_adc(void);
float adc_distance(void);
int average_distance(void);
float adc_battery_voltage(void);
float average_battery_voltage(void);

#endif	/* XC_HEADER_TEMPLATE_H */

