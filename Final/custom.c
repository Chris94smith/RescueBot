
#include "timer.h"
#include "adc.h"
#include "uart_extra_help.h"
#include "string.h"
#include "math.h"
#include "open_interface.h"
#include "servo.h"
#include "ping.h"



//PRINT STRING TO PUTTY
void puttyPrint(char out[]){
    int i=0;
    for( i= 0; i < strlen(out); ++i){
        char b = out[i];
        //cyBot_sendByte(b);
        uart_sendChar(b);
    }
}

//CONVERT IR RAW TO DISTANCE
float convertIR(int rawIR){

    //int IR_Raw = sensor_Scan.IR_raw_val;

    // y = 600(x)^-1.5  +10
	// x = 71.13786608/ (y-10)^0.6666

    //IR CALC
    float IR_Dist =0;
    float y_Val = ((float)rawIR/100) -10;
    IR_Dist = 71.13786608/pow(y_Val, 0.6666);

    return IR_Dist;


}

int scanIR(int samples){

    int value = 0;
    int i=0;

    for (i = 0; i < samples; ++i)
    {
        value += adc_read();
        timer_waitMillis(10);
    }
    value = value / i;

    return value;

}

int boundCheck(oi_t *sensor){
    //TEST
    int valueL = 0;
    int valueR = 0;
    int valueFR = 0;
    int valueFL = 0;
    int i =0;
    char out[100] ="";
    short sampSize =1;
    int lowVal = 350;
    int highVal = 2500;

    for (i = 0; i < sampSize; ++i){
        //oi_update(sensor);
        valueFR += sensor->cliffFrontRightSignal;
        valueFL += sensor->cliffFrontLeftSignal;
        valueR += sensor->cliffRightSignal;
        valueL += sensor->cliffLeftSignal;
    }
    valueL = valueL / sampSize;
    valueR = valueR / sampSize;
    valueFR = valueFR / sampSize;
    valueFL = valueFL / sampSize;

    if(valueL > highVal || valueL <lowVal){
        sprintf(out, "SENSOR L: %d \n\r", valueL);
        puttyPrint(out);
        return 1;
    }
    else if(valueFL > highVal || valueFL <lowVal){
       sprintf(out, "SENSOR FL: %d \n\r", valueFL);
       puttyPrint(out);
       return 2;
    }
    else if(valueFR > highVal || valueFR <lowVal){
       sprintf(out, "SENSOR FR: %d \n\r", valueFR);
       puttyPrint(out);
       return 4;
    }
    else if(valueR > highVal || valueR <lowVal){
       sprintf(out, "SENSOR R: %d \n\r", valueR);
       puttyPrint(out);
       return 5;
    }
    else{
        return 3;
    }

    //    if (valueL > 2700 || valueFL > 2700 || valueFR > 2700 || valueR > 2700){
    //        sprintf(out, " -- BOUNDARY -- \n\r");
    //        puttyPrint(out);
    //    }
    //
    //    sprintf(out, "> %5d  %5d  %5d  %5d  \n\r", valueL, valueFL, valueFR, valueR);
    //    puttyPrint(out);

}

int quickScan(oi_t *sensor){
    int start = 0;
    int object = 0;
    float temp =0;
    float i=0;
    int angle=0;
    char out[100]= "";
    oi_setWheels(0,0);
    servo_move2(0);
    timer_waitMillis(700);

    sprintf(out, "\n\r");
    puttyPrint(out);

    //while (angle < 180 && object == 0 && boundCheck(sensor)==3){
    for(i=0; i<18; ++i){
        angle = start+ (i*10);
        servo_move2(angle);
        timer_waitMillis(50);

        //do{
            temp = ping_distance();
        //}while(temp<=0);

//        tempIR = scanIR(2);
//        if(tempIR<temp){
//            temp =tempIR;
//        }

        if (temp < 27 - abs(9-i)){  //- abs(7-i)
            //object = 1;
            sprintf(out, "OBJECT FRONT %d %f\n\r", angle, temp);
            puttyPrint(out);
            if(angle<=90){
                timer_waitMillis(150);
               return 1;
            }else{
                return 2;
            }
        } else{
            sprintf(out, "%d %0.2f,  ", angle, temp);
            puttyPrint(out);
        }
        //++i;
        timer_waitMillis(100);
    }


    if(object == 0){   //TODO DEBUG
        sprintf(out, "Front clear \n\r");
        puttyPrint(out);
    }

    return object;
}


