/*
 * ping.c
 *
 *  Created on: Oct 24, 2023
 *      Author: clsmith3
 */

#include "ping.h"
#include "timer.h"
#include "open_interface.h"
#include <inc/tm4c123gh6pm.h>
#include "custom.h"
#include "lcd.h"

int statusFlag =0;
int clockTime =0;
int timeSpan =0;

void ping_init2(void){

    SYSCTL_RCGCGPIO_R |= 0x2;

    GPIO_PORTB_DEN_R |= 0x8;
    GPIO_PORTB_AFSEL_R |= 0x8;
    GPIO_PORTB_DIR_R |= 0x8;
    GPIO_PORTB_PCTL_R &= ~0xF000;
    GPIO_PORTB_PCTL_R |= 0x7000;
    //AFSEL??

    SYSCTL_RCGCTIMER_R |= 0x8;

    TIMER3_CTL_R |=   0xc00;
    TIMER3_TBMR_R |= 0b00111;

    TIMER3_CFG_R |=  0b100;
    TIMER3_TBILR_R |= 0xFFFF;
    TIMER3_TBPR_R |= 0xFF;
    TIMER3_IMR_R |=  0b010000000000;
    TIMER3_CTL_R |=   0x100;
    //Input edge-time mode, input capture
    // Timer 3B
    // 16-bit count-down mode - - errata not GPTM#11 end of pt2
    // 8-bit pre-scaler as extension to get 24-bit timer
    // initialized by ILR and PR regs to 0xFFFF and 0xFF
    // config and enable interrupts
    // write interrupt handler
    // MVIC - needs to be included
    // Enable the clock for the timer
    // ICR - needs to be included

}

int ping_read(void){
    //timeSpan =0;
    statusFlag = 0;
    send_pulse();
    while(statusFlag != 2){}
    // reconfig PB3 from digital out to timer IN.
    // cahnge alt-func for PB3to detect edges of pulse echo
    // AFSEL and PTCL
    // Pulse will have rising edge and falling edge, difference is round trip time
    // value stored in TIMER3_TBR_R

    return timeSpan;

}

void send_pulse(void){

    TIMER3_CTL_R &=   ~0x100;
    GPIO_PORTB_AFSEL_R &= ~0x8;
    TIMER3_IMR_R &=  ~0b010000000000;

    GPIO_PORTB_DIR_R |= 0x8;
    GPIO_PORTB_DATA_R |= 0x8;
    timer_waitMicros(10);
    GPIO_PORTB_DATA_R &= 0x7;
    GPIO_PORTB_DIR_R &= ~0x8;

    TIMER3_IMR_R |=  0b010000000000;
    GPIO_PORTB_AFSEL_R |= 0x8;
    TIMER3_CTL_R |=   0x100;
}


void ping_interrupt_init(){

    TIMER3_IMR_R |=  0b010000000000;

    NVIC_EN1_R |= 0x10;

    IntRegister(INT_TIMER3B, ping_interrupt_handler);
    IntMasterEnable();
}

void ping_interrupt_handler(void){

        if(TIMER3_MIS_R & 0x400){

            //lcd_putc('h');

            //++statusFlag;
            if(statusFlag == 0){
                clockTime = TIMER3_TBR_R;
                statusFlag =1;
            }
            else if(statusFlag == 1){
                timeSpan = clockTime - TIMER3_TBR_R;
                statusFlag =2;
            }

        TIMER3_ICR_R |= 0x400;
        }
}

float ping_distance(void){

    float temp = -1;
    do{
    temp = (17000 * (float)ping_read())/16000000;
    timer_waitMillis(10);
    }while(temp<0);

    return temp;
}








