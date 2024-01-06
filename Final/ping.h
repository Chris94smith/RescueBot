/*
 * ping.h
 *
 *  Created on: Oct 24, 2023
 *      Author: clsmith3
 */

#ifndef PING_H_
#define PING_H_

void ping_init2(void);
int ping_read(void);
void send_pulse(void);
void ping_interrupt_init();
void ping_interrupt_handler(void);
float ping_distance(void);

#endif /* PING_H_ */
