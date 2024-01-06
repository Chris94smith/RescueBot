/*
*
*   uart.h
*
*   Used to set up the RS232 connector and WIFI module
*   uses UART1 at 115200
*
*
*   @author Dane Larson
*   @date 07/18/2016
*   @ Phillip Jones updated: 9/11/2018
*/

#ifndef CYBOT_UART_H_
#define CYBOT_UART_H_

#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>


// Initialize the UART to communciate between CyBot and PuTTy
void cyBot_uart_init(void); 

// Send a byte over the UART from CyBot and PuTTy (Buad Rate 115200, No Parity, No Flow Control)
void cyBot_sendByte(char data);

// Cybot WAITs to recive a byte from PuTTy (Buad Rate 115200, No Parity, No Flow Control).
// In other words, this is a blocking fucntion.
char cyBot_getByte_blocking(void);

//Nonblocking call to receive over uart1/
// Check if a Byte was received from PuTTy, if so place in location of byte-sized variable passed to the function
//returns: 0 = No Byte available,  1 = Byte received
// !!Under Construction!!
//int cyBot_getByte_nonblocking(char *data);


#endif /* CYBOT_UART_H_ */
