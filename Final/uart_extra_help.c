/*
*
*   uart_extra_help.c
* Description: This is file is meant for those that would like a little
*              extra help with formatting their code, and followig the Datasheet.
*/

#include "uart_extra_help.h"
#include "timer.h"
#include <stdio.h>
#include "lcd.h"
#include "custom.h"

#define REPLACE_ME 0x00

int char_counter = 0;
int globalVar=0;

int toggleFlag = 0;

void uart_init(void)
{
    //SYSCTL_RCGCGPIO_R |= REPLACE_ME;      // enable clock GPIOB (page 340)
    //SYSCTL_RCGCUART_R |= REPLACE_ME;      // enable clock UART1 (page 344)
    //GPIO_PORTB_AFSEL_R = REPLACE_ME;      // sets PB0 and PB1 as peripherals (page 671)
    //GPIO_PORTB_PCTL_R  = REPLACE_ME;       // pmc0 and pmc1       (page 688)  also refer to page 650
    //GPIO_PORTB_DEN_R   = REPLACE_ME;        // enables pb0 and pb1
    //GPIO_PORTB_DIR_R   = REPLACE_ME;        // sets pb0 as output, pb1 as input
    //
    SYSCTL_RCGCGPIO_R |= 0b00000010;
    timer_waitMillis(1);            // Small delay before accessing device after turning on clock
    SYSCTL_RCGCUART_R |= 0b00000010;      // enable clock UART1 (page 344)
    GPIO_PORTB_AFSEL_R |= 0b00000011;
    GPIO_PORTB_PCTL_R &= 0x00000011;     // Force 0's in the desired locations   00010001
    GPIO_PORTB_PCTL_R |= 0x00000011;     // Force 1's in the desired locations   00010001
    GPIO_PORTB_DEN_R |= 0b00000011;    //10001
    GPIO_PORTB_DIR_R &= 0b11111100;      // Force 0's in the desired locations  11111110
    GPIO_PORTB_DIR_R |= 0x1;      // Force 1's in the desired locataions 10000
    //



    //compute baud values [UART clock= 16 MHz] 
    double fbrd;
    int    ibrd;


    fbrd = 44; // page 903
    ibrd = 8;
    //fbrd = REPLACE_ME;

    UART1_CTL_R &= 0b0;      // disable UART1 (page 918)
    UART1_IBRD_R = ibrd;        // write integer portion of BRD to IBRD
    UART1_FBRD_R = (int)fbrd;   // write fractional portion of BRD to FBRD
    UART1_LCRH_R = 0b01100000;        // write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R   = 0b0000;          // use system clock as clock source (page 939)
    UART1_CTL_R |= 0b1100000001;        // enable UART1

}

void uart_sendChar(char data) //blocking
{
    while(UART1_FR_R & 0b100000){
        //char_counter++;
        //lcd_putc(char_counter);
    }
    UART1_DR_R = data;  //loc
}

// Things to consider for part 1:
// A counter on how many characters there are (counter displayed on the lcd).
// Once characters reach 20 or user presses enter, clear the lcd and then display the characters on line 1.

char uart_receive(void) //blocking
{
    while(UART1_FR_R & 0b10000){
        if ((char_counter >= 5)) {
            //printf("Max");
        }
    }

    //lcd_putc(char_counter);
    char_counter++;
    char temp2 = UART1_DR_R;
    return temp2;
}

void uart_sendStr(const char *data)
{
    //TODO
	
}

// _PART3


void uart_interrupt_init()
{
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b10000; //enable interrupt on receive - page 924 (Put in this? 0x4000.D000)

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0b1000000; //enable uart1 interrupts - page 104   0x0000.0058

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number
    //IntRegister(INT_UART1, REPLACE_ME)
}

void uart_interrupt_handler()
{
// STEP1: Check the Masked Interrupt Status
    if(UART1_MIS_R & 0x10){

//STEP2:  Copy the data 
    globalVar = UART1_DR_R & 0xFF;


    if( (char)globalVar == 't'){
        toggleFlag = (toggleFlag? 0: 1);
        lcd_clear();
        if(toggleFlag==1){
            lcd_puts("AUTO");
            puttyPrint(" AUTO \n\r");
        }else{
            lcd_puts("MANUAL");
            puttyPrint(" MANUAL \n\r");
        }
        //oi_setWheels(0, 0);
        timer_waitMillis(1);

    }

//STEP3:  Clear the interrupt
    UART1_ICR_R |= 0b10000;
    }
}
