/*
 * final_Main.c
 *
 *  Created on: Nov 10, 2023
 *      Author: clsmith3
 */



#include "timer.h"
#include "lcd.h"
#include "servo.h"
#include "uart_extra_help.h"
#include "open_interface.h"
#include "movement.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "custom.h"
#include "adc.h"
#include "ping.h"
#include "movement.h"

extern volatile int toggleFlag;   // GLOBAL VARS
extern volatile int globalVar;
int collides =0;

int main(void){

    // INITS
    oi_t *sensor = oi_alloc();
    oi_init(sensor);
    timer_init();
    uart_init();
    uart_interrupt_init();
    lcd_init();
    adc_init();               //REMINDER: calibrate IR values in 'c' and 'l'
    ping_init2();
    ping_interrupt_init();
    servo_init2();            //REMINDER: calibrate L/R values in servo.c
    lcd_puts("MANUAL");

    // VARIABLES
    int i=0;
    char out[100] ="";
    char inChar = ' ';
    int checkMate =0;
    int parkCheck =0;

    // SETTINGS
    servo_move2(0);
    int spd = 180;
    int scanThresh = 80;
    int pythonEn =0;       //1: PYTHON, 0: PUTTY

    // STORE DATA
    float scanData_fullIR[91];
    float pingDist[10];
    int objAng[10];
    int objWid[10];
    int smallestDir = -1;
    int smallestDist = 0;



    // CORE LOOP
    while(1){

        if(pythonEn==1){                                     // PYTHON
            ////////////////          //////////////////////          ////////////////          //////////////
            char my_data;       // Variable to get bytes from Client
            char command[100];  // Buffer to store command from Client
            int index = 0;      // Index position within the command buffer

            if (toggleFlag == 0){
                my_data = uart_receive(); // Get first byte of the command from the Client

                // Get the rest of the command until a newline byte (i.e., '\n') received
                while (my_data != '\n'){
                    command[index] = my_data; // Place byte into the command buffer
                    index++;
                    my_data = uart_receive(); // Get the next byte of the command
                }

                command[index] = '\n'; // place newline into command in case one wants to echo the full command back to the Client
                command[index + 1] = 0; // End command C-string with a NULL byte so that functions like printf know when to stop printing
                inChar = command[0];
                lcd_printf("Got: %s", command); // Print received command to the LCD screen
                puttyPrint(command);
                lcd_putc(command[0]);
                uart_sendChar(command[0]);
            }
            else{
                command[index] = inChar;
                index++;
                command[index] = '\n'; // place newline into command in case one wants to echo the full command back to the Client
                command[index + 1] = 0; // End command C-string with a NULL byte so that functions like printf know when to stop printing
                uart_sendChar(inChar);
            }
            // Send a response to the Client (Starter Client expects the response to end with \n)
            // In this case I am just sending back the first byte of the command received and a '\n'

            // Only send a '\n' if the first byte of the command is not a '\n',
            // to avoid sending back-to-back '\n' to the client
            if (command[0] != '\n'){
                uart_sendChar('\n');
            }
            else{
                uart_sendChar(inChar);
            }
            ////////////////          //////////////////////          ////////////////          //////////////

        }else{                                               // PUTTY

            if(toggleFlag==0){
                inChar = uart_receive();
            }
            if(inChar !=' '){
                uart_sendChar(inChar);
            }

        }


        // INPUT CONTROLS
        /*
         * w: forward
         * s: reverse
         * d: right
         * a: left
         *
         * e: stop wheels
         * r: speed, up
         * f: speed, down
         *
         * u: short forward
         * j: short reverse
         * h: short left
         * k: short right
         *
         * t: auto/manual toggle
         * o: re-orient servo, reset checkMate
         * c: IR calibrate
         * x: quick scan
         *
         * l: 180 scan
         * g: final navigate
         * y: auto drive
         * p: fully park
         *
         * z: free sensor
         *
         */


        if (inChar == 'w'){        // FORWARD                       // STANDARD DRIVE CONTROLS
            oi_setWheels(spd, spd);
        }
        else if (inChar == 'e'){   // BRAKE
            oi_setWheels(0, 0);
        }
        else if (inChar == 's'){   // REVERSE
            oi_setWheels(-spd, -spd);
        }
        else if (inChar == 'd'){   // RIGHT
            oi_setWheels(-spd*0.8, spd*0.8);
        }
        else if (inChar == 'a'){   // LEFT
            oi_setWheels(spd*0.8, -spd*0.8);
        }

        else if (inChar == 'u'){     //FORWARD                 PRECISE DRIVE CONTROLS
            precise_forward(sensor, 200, spd);
        }
        else if (inChar == 'j'){ //REVERSE
            move_reverse(sensor, 10);

        }
        else if (inChar == 'h'){  //LEFT
            turn_any(sensor, 10);

        }
        else if (inChar == 'k'){  //RIGHT
            turn_any(sensor, -10);
        }


        else if (inChar == 'r'){   // SPEED UP
            spd = spd + 50;
            sprintf(out, "SPEED: %4d \n\r", spd);
            puttyPrint(out);
        }
        else if (inChar == 'f'){   // SLOW DOWN
            spd = spd - 50;
            sprintf(out, "SPEED: %4d \n\r", spd);
            puttyPrint(out);
        }

                                                                    // UTILITY CONTROLS
        else if (inChar == 't'){   // AUTO TOGGLE
            //Starts scan/move cycle
            inChar = 'l';
        }
        else if(inChar == 'o'){    // RE-ORIENT SCANNER
            servo_move2(0);
            checkMate =0;
            parkCheck =0;
        }
        else if(inChar == 'c'){    // ADC IR FORWARD SCAN
            servo_move2(90);
            int stopper = toggleFlag;
            int value = 0;
            float scanDist = 0;
            while (toggleFlag == stopper){
                value = 0;
                scanDist = 0;
                value = scanIR(50);

                float IR_Dist = 0;
                float y_Val = ((float) value / 100);
                IR_Dist = 536 * pow(y_Val, -1.2);     //bot 6.... also darn close for bot 9

                sprintf(out, "   %4d    %f\n\r", value, IR_Dist);
                puttyPrint(out);
                timer_waitMillis(500);
            }
            inChar = ' ';
        }

        else if(inChar == 'x'){ // quick scan
            quickScan(sensor);
        }


                                                                    // SCAN CONTROLS
        else if(inChar == 'l'){    // 180 SCAN
            servo_move2(0);
            oi_setWheels(0, 0);
            timer_waitMillis(500);
            i = 0;
            int j = 0;
            int flag = 0;
            int stopper = toggleFlag;

            uart_sendChar('\n');
            uart_sendChar('\n');
            uart_sendChar('\r');
            //out ="";

            //for(i=0; i<=180 && stopper==toggleFlag; i+=2){ //COLLECT SCAN DATA
            for(i=0; i<=180; i+=2){ //COLLECT SCAN DATA
                servo_move2(i);
                int value =0;
                if(checkMate==0){
                    value = scanIR(20);
                }else{
                    value = scanIR(30);
                }
                float IR_Dist = 0;
                float y_Val = ((float) value / 100);
                IR_Dist = 536 * pow(y_Val, -1.2);       //CALIBRATED FOR: bot 6
                scanData_fullIR[i / 2] = IR_Dist;

                float ping = ping_distance();

                if ((scanData_fullIR[i / 2] < scanThresh) && ( ping < scanThresh )){

                    if (flag == 0){
                        flag = 1;
                        sprintf(out, "        - - %3.4f - -  %3.4f - - %d \n\r", ping, scanData_fullIR[i/2], j);
                        puttyPrint(out);
                        pingDist[j] = ping;
//                        pingDist[j] = scanData_fullIR[i/2];
                        objAng[j] = i;
                        objWid[j] = 2;
                    }
                    else{
                        objWid[j] += 2;
                    }
                }
                else if (scanData_fullIR[i / 2] >= scanThresh){
                    if (flag == 1){
                        ++j;
                    }
                    flag = 0;
                }

                sprintf(out, "%3d   ", i);
                puttyPrint(out);
                sprintf(out, "Dist: %3.3f  \n\r", scanData_fullIR[i / 2]);      //For reading
                //sprintf(out,"     %3.3f  \n\r", sensor_Scan.sound_dist/100);  //For graphing
                puttyPrint(out);

                if(stopper!=toggleFlag){
                    break;
                }
            }

            uart_sendChar('\n');
            uart_sendChar('\r');
            bool onObj = false;
            j = 0;
            int k = 0;
            int smallest = 180;
            smallestDir = -1;

            for (i = 0; i < 90; ++i){  // DETERMINE SMALLEST
                if (scanData_fullIR[i] < scanThresh){
                    onObj = true;
                    j += 2;
                }
                else{
                    onObj = false;
                    if (j > 0){
                        ///
                        if(pingDist[k]<=scanThresh && pingDist[k]>0){

                            if(objAng[k]==0 || (objAng[k]+objWid[k]>=180)){
                                objWid[k]= objWid[k]*2;
                            }


//                            double linDist = objWid[k] * (pingDist[k]/30);  //lineart dist adjustment, ta

//                            double angDist = objWid[k] * 0.0174532925;

//                            sprintf(out, "PING: %lf    WIDTH: %lf \n\r", (double)pingDist[k]*2.0, tan(((double)objWid[k]*3.14/180.0   )/2.0));   // angle of object end
//                            puttyPrint(out);

                            double linDist =(double)( 2.0 * (double) pingDist[k] * tan(((double)objWid[k]*3.14/180.0   )/2.0));

//                            double prep = 1-cos(2*objWid[k]/2);
//                            double linDist = (2 *pingDist[k] *prep) / tan(objWid[k]/2);

//                            double linDist = 2*( tan((double)objWid[k]/2) *pingDist[k]/2);

//                            double linDist = objWid[k] / (pow(pingDist[k],2)/pow(30,2));


//                            double linDist = objWid[k];
//                            if(pingDist[k]< 80){   //50
//                                linDist = sqrt(pow(objWid[k],2) * pow(pingDist[k]/80, 1.5) *2) ;   //50     linDist = objWid[k] * (pingDist[k]/80);
//                            }

                            //linDist = linDist*0.8;


                        ///
                            sprintf(out, "%3d-%3d  Width: %3.4f   (non-adjusted)%3d    Distance: %3.4f\n\r", objAng[k], i * 2, linDist, objWid[k], pingDist[k]);   // angle of object end
                            puttyPrint(out);
                            if (linDist < smallest && linDist < 11 && linDist >4){
                                smallest = linDist;
                                smallestDir = objAng[k] + (objWid[k] / 2);
                                smallestDist = pingDist[k];
                                //sprintf(out, "Smallest: %d", i*2, j, objWid[k]);   // angle of object end
                                //puttyPrint(out);
                            }
                        }
                        ++k;
                    }
                    j = 0;
                }
            }

            servo_move2(0);
            if(toggleFlag ==1){  // AUTO
                if(smallest > 12 || smallest <4){   //12
                    inChar = 'g';
                    checkMate =0;
                }else{
                    inChar = 'y';
                }
            }
            if(pythonEn==1){   // PYTHON
                sprintf(out, "end\n");
                puttyPrint(out);
            }
        }  //end of 'l'



        else if(inChar == 'g'){                             // FINAL NAVIGATE TODO
            //servo_move2(70);
            //int ping = ping_distance();
            oi_setWheels(0,0);
            int scan = quickScan(sensor);
            timer_waitMillis(1000);
            int j =1;
            int k=0;
            int turn = 2;

            while(scan >0){

                if(scan==1){
                    turn_any(sensor, turn);
                }else if(scan==2){
                    turn_any(sensor, -turn);
                }
                //turn_any(sensor, turn*j);
                //ping = ping_distance();
                scan = quickScan(sensor);
//                if(k%2==0){
//                    j= j*-1;
//                }else{
//                    //turn -= 10;
//                }
//                ++k;

            }

            //for(i=0; i<5; ++i){
            i=0;
            while(i<4 && scan==0){
                //ping = ping_distance();
                //if( scan==0 ){

                move_forward_bump(sensor, 200, spd);
                //}
                ++i;

                //scan = quickScan(sensor);
                //timer_waitMillis(1000);
            }
            servo_move2(0);

            if(toggleFlag == 1){
                inChar = 'l';
            }

        }




        else if(inChar == 'y'){                                                                                // AUTO- DRIVE TO SMALLEST
            int completed =0;

            if(smallestDir >=0){
                int turn = (smallestDir -90) * 0.8;  //adjust offset      * (1 -0.0056*abs(90-smallestDir)  ??  0-90 = 1.0-0.5
//                if(smallestDir <=90 ){    //right-
//                    turn = smallestDir -90;
//                }
//                else if(smallestDir >90){ //left+
//                    turn = smallestDir -90;
//                }

                turn_any(sensor, turn);

                sprintf(out,"Turn: %d / %d", smallestDir, turn);
                puttyPrint(out);

                if(abs(turn) >= 40 && checkMate>0){  // NEW NEEDS DEBUG
                    --checkMate;
                }

                int forward = (smallestDist-10)*10;
                sprintf(out,"    Forward: %d / %d \n\r", smallestDist, forward);
                puttyPrint(out);

                while(forward>0){
                    int modu = forward%200;  //TODO

                    if(modu==0){
                        modu = 200;
                    }
                    if(forward>=250){
                        move_forward_bump(sensor, modu, spd);
                    }else if(checkMate>1){
                        move_forward(sensor, forward-50, spd);  //if checkMate==1?
                    }
                    sprintf(out,"    Forward: %d   Modu: %d\n\r", forward, modu);
                    puttyPrint(out);
                    forward -= modu;
                }
                completed =1;
            }

            if(completed ==1){
                if(checkMate>1){  //Double scan to make sure
                    inChar = 'p';
                    sprintf(out,"    CHECKMATE \n\r");
                    puttyPrint(out);
                }else{
                    inChar = 'l'; //AUTO
                    if(smallestDist <40){
                        ++checkMate;
                        sprintf(out,"    CHECK %d\n\r", checkMate);
                        puttyPrint(out);
                    }
                }
            }
            else{
                checkMate =0;
                inChar = 'l'; //AUTO
            }

            if(pythonEn == 1){  // PYTHON
                sprintf(out,"end\n");
                puttyPrint(out);
            }

            //smallestDir =-1;?
        } //end of 'y'

        else if(inChar == 'p'){
            servo_move2(0);
            oi_setWheels(0, 0);
            timer_waitMillis(500);
            i = 0;
            int j = 0;
            int flag = 0;
            int stopper = toggleFlag;
            int spin = 1;

            int frontRight = 0;
            int frontLeft = 0;
            int backLeft = 0;
            int backRight = 0;


            uart_sendChar('\n');
            uart_sendChar('\n');
            uart_sendChar('\r');
            //out ="";
            unsigned char SOSNumNotes = 9;
            unsigned char SOSNotes[9]  = {60,60,60,60,60,60,60,60,60};
            unsigned char SOSDuration[9] = {10, 10, 10, 40, 40, 40, 10, 10, 10};
            oi_loadSong(0, SOSNumNotes, SOSNotes, SOSDuration);
            oi_play_song(0);
            timer_waitMillis(100);


            int lessThresh = 60;

            while(1){
           //for(i=0; i<=180 && stopper==toggleFlag; i+=2){ //COLLECT SCAN DATA
            for(i=0; i<=180; i+=2){ //COLLECT SCAN DATA
                servo_move2(i);
                int value =0;
                if(checkMate==0){
                    value = scanIR(20);
                }else{
                    value = scanIR(30);
                }
                float IR_Dist = 0;
                float y_Val = ((float) value / 100);
                IR_Dist = 536 * pow(y_Val, -1.2);       //CALIBRATED FOR: bot 6
                scanData_fullIR[i / 2] = IR_Dist;

                float ping = ping_distance();

                if ((scanData_fullIR[i / 2] < lessThresh) && ( ping < lessThresh )){

                    if (flag == 0){
                        flag = 1;
                        sprintf(out, "        - - %3.4f - -  %3.4f - - %d \n\r", ping, scanData_fullIR[i/2], j);
                        puttyPrint(out);
                        pingDist[j] = ping;
//                        pingDist[j] = scanData_fullIR[i/2];
                        objAng[j] = i;
                        objWid[j] = 2;
                    }
                     else{
                         objWid[j] += 2;
                     }

                }
                else if (scanData_fullIR[i / 2] >= lessThresh){
                    if (flag == 1){
                        double linearDist = 2.0 * ((double)pingDist[j] *      tan((((double)objWid[j] * 3.14) / 180) / 2.0));
                        sprintf(out, "%lf %d %d\n\r", linearDist, (int)pingDist[j], objWid[j]);
                            puttyPrint(out);

                        if(linearDist < 12 && linearDist > 3){ //checking linear width

                            if(spin && i<=90) //figuring out where objects are in relation to the bot
                                frontRight++;
                            else if (spin && i>90)
                                frontLeft++;
                            else if(!spin && i<=90)
                                backLeft++;
                            else if (!spin && i>90)
                                backRight++;
                            sprintf(out, "%d %d %d %d\n\r", frontRight, frontLeft, backRight,backLeft);
                            puttyPrint(out);
                        }
                        ++j;
                    }
                    flag = 0;
                }
            }

            if(spin){
            turn_any(sensor, 180);
            move_reverse(sensor, 5);
            spin = 0;
            }
            else break;
            }
            if(frontRight > frontLeft && backRight > backLeft){
                sprintf(out, "One");
                puttyPrint(out);
                turn_any(sensor, 90);
                precise_forward(sensor, 15, 100);
            }
            else if(frontRight < frontLeft && backRight < backLeft){
                sprintf(out, "Two");
                puttyPrint(out);
                turn_any(sensor, -90);
                precise_forward(sensor, 15, 100);
            }
            else if(frontRight > backRight && frontLeft > backLeft){
                sprintf(out, "Three");
                puttyPrint(out);
                turn_any(sensor, 180);
                precise_forward(sensor, 15, 100);
            }
            else if(frontRight < backRight && frontLeft < backLeft){
                sprintf(out, "Four");
                puttyPrint(out);
                precise_forward(sensor, 15, 100);
            }
            if(frontRight && frontLeft && backRight && backLeft){
                sprintf(out, "Bingo");
                puttyPrint(out);
                inChar = ' ';
                parkCheck =0;
                unsigned char rickrollNumNotes = 11;
                    unsigned char rickrollNotes[11]    = {53, 55, 48, 55, 57, 60, 58, 57, 53, 55, 48};
                    unsigned char rickrollDuration[11] = {48, 64, 16, 48, 48, 8,  8,  8,  48, 64, 64};
                    oi_loadSong(0, rickrollNumNotes, rickrollNotes, rickrollDuration);
               oi_play_song(0);
            }else{
                if(parkCheck>=2){
                    inChar = 'g';
                    parkCheck=0;
                    turn_any(sensor, 180);
                }else{
                    ++parkCheck;
                    inChar = 'p';
                }
            }
        }



        else if(inChar == 'z'){                         // FREE SENSOR
            oi_free(sensor);
            //exit;
        }



        if(toggleFlag==0 && pythonEn==1){ //FOR PYTHON
            sprintf(out, "end\n");
            puttyPrint(out);
        }


    }  //END OF COOR LOOP


}















