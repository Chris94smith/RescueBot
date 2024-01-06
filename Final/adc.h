/*
 * adc.h
 *
 *  Created on: Oct 17, 2023
 *      Author: clsmith3
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

void adc_init(void);

int adc_read(void);


#endif /* ADC_H_ */
