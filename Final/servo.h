/*
 * servo.h
 *
 *  Created on: Oct 31, 2023
 *      Author: clsmith3
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

void servo_init2(void);
int servo_move2(float degress);


#endif /* SERVO_H_ */
