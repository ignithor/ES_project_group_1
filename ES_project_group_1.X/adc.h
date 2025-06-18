#ifndef ADC_H
#define	ADC_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <math.h>

#define BUFFER_SIZE 5

void setup_adc();
float adc_distance(void);
int average_distance(void);
float adc_battery_voltage(void);

#endif	/* XC_HEADER_TEMPLATE_H */

