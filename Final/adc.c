/*
 * adc.c
 *
 *  Created on: Oct 17, 2023
 *      Author: clsmith3
 */

#include "adc.h"
#include "timer.h"

void adc_init(void){

    //PORT B
    GPIO_PORTB_AFSEL_R |= 0b00010000;
    GPIO_PORTB_DEN_R |= 0b00000011;    //10001
    GPIO_PORTB_DIR_R &= 0b11101111; // Force 0's in the desired locations  11111110
    GPIO_PORTB_AMSEL_R |= 0x10;      //AMSEL
    GPIO_PORTB_ADCCTL_R |= 0x10;     //ADCCTL


    // ADC 0
    SYSCTL_RCGCADC_R |= 0b001; //ADC run mode clock gating control
    timer_waitMillis(1);
    ADC0_ACTSS_R |= 0b00001;    // active sample sequencer
    //ADC0_EMUX_R |= 0b00000;     // event multiplexer select
    //ADC0_PSSI_R |= 0b000001;     // processor sample sequence initiate
    //
    ADC0_SSMUX0_R |= 10;   // sample sequence multiplexer
    ADC0_SSCTL0_R |= 0b0110;   // sample sequence control    //TIED = temperature, interrupt, end, differential    0110
    //ADC0_SSFIFO0_R   // sample sequence result FIFO
    //
    //ADC0_IM_R |= 0x000100001;       //interrupt mask
    //ADC0_RIS_R |= 0x000001;      // raw interrupt status

    //
    ADC0_SAC_R |= 0x004;      //sample averaging control

}

int adc_read(void){

    //IR data in FIFO <single value
    //PSSI calls scanner
    ADC0_PSSI_R |= 0b000001;
    while(~ADC0_RIS_R & 0x01){

    }
    ADC0_ISC_R |= 0x0001;      // interrupt status and clear
    return ADC0_SSFIFO0_R;


}

