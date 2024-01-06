/*
 * servo.c
 *
 *  Created on: Oct 31, 2023
 *      Author: clsmith3
 */
#include "ping.h"
#include "timer.h"
#include "open_interface.h"

void servo_init2(void){

    //GPTM regs
    //PWM mode
    // GPTMTnMATCHR match register,  tiva fig.11-5 section 11.4.5  PMR

    SYSCTL_RCGCGPIO_R |= 0x2;
    GPIO_PORTB_DEN_R |= 0x20;
    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_DIR_R |= 0x20;
    GPIO_PORTB_PCTL_R &= ~0xF00000;  //?
    GPIO_PORTB_PCTL_R |= 0x700000;   //?


    SYSCTL_RCGCTIMER_R |= 0x2;

    TIMER1_CTL_R |= 0xc00;    // TIMER3_CTL_R |=   0xc00;  >> TIMER3_CTL_R |=   0x100;
    TIMER1_TBMR_R |= 0xa;                        // TIMER3_TBMR_R |= 0b00111;

    TIMER1_TBILR_R = 0xe200; // TIMER3_TBILR_R |= 0xFFFF;      320,000 == 4e200
    TIMER1_TBPR_R |= 0x4;     // TIMER3_TBPR_R |= 0xFF;
    TIMER1_CFG_R |= 0x4;      // TIMER3_CFG_R |=  0b100;

    TIMER1_CTL_R |= 0x100;
}


int servo_move2(float degrees){



//    if(degrees<0){
//        degrees =0;
//    }
//    else if(degrees>180){
//        degrees =180;
//    }

    //float turn2 = 158.333*degrees -500; //bot 11
    //float turn2 = -15.556*degrees +2600;  //bot 6
//    int h = (int)(157.07*degrees +350);  //bot 9

    //y=mx+b
    //y= -15.556x +2600   bot#6

    //int h = 16000 + degrees*500; //for calibration
    int h = (int) 152.96*degrees -1805;  // cal for bot 6

    TIMER1_TBMATCHR_R = (320000 - h) & 0xFFFF;
    TIMER1_TBPMR_R = (320000 -h) >> 16;

    //TIMER1_TBMATCHR_R = 0xA380;     // 304,000 == 4A380
    //TIMER1_TBPMR_R |= 0x4;


    return h;
}











