/*
 * movement.c
 *
 *  Created on: Sep 5, 2023
 *      Author: clsmith3
 */
    #include "open_interface.h"
    #include "math.h"
    #include "stdlib.h"
    #include "uart_extra_help.h"
    #include "custom.h"
    #include "ping.h"
    #include "servo.h"

    extern int collides;

/*
      *
        uint32_t cliffLeftSignal : 1;
        uint32_t cliffFrontLeftSignal : 1;
        uint32_t cliffFrontRightSignal : 1;
        uint32_t cliffRightSignal : 1;
      *
      */

    int move_reverse(oi_t *sensor, int centimeters){
           double sum = 0;
           //centimeters = centimeters*10;
           oi_setWheels(-100, -100 );

           while (sum < centimeters){ //&& (sensor->bumpRight + sensor->bumpLeft)<1){
               oi_update(sensor);
               sum -= sensor->distance;
           }
           oi_setWheels(0, 0);               // stop
           return 0;
       }

    void turn_any(oi_t *sensor, int degrees){
            double sum =0;

            if(degrees>0){  //counter
                oi_setWheels(130, -130);             // right, left
            }
            else if(degrees<0){ //clockwise
                oi_setWheels(-130, 130);
            }

            while (abs(sum) < abs(degrees)) {
                oi_update(sensor);
                sum += sensor->angle;
            }
            oi_setWheels(0, 0);               // stop
        }

       void boundary_turn(oi_t *sensor){

           int test= boundCheck(sensor);
           if(test<3){
               oi_setWheels(-150, 150);
           }
           else if (test > 3){
               oi_setWheels(150, -150);
           }

           do{
               oi_update(sensor);
               test = boundCheck(sensor);
   //            if(test<3){
   //                oi_setWheels(-150, 150);
   //            }
   //            else if(test>3){
   //                oi_setWheels(150, -150);
   //            }
               timer_waitMillis(120);
           }while(test !=3);
           oi_setWheels(0,0);
           timer_waitMillis(1000);
       }


    int precise_forward(oi_t *sensor, int centimeters, int spd){
        extern volatile int toggleFlag;
        int stop = toggleFlag;
        double sum = 0;
        //centimeters = centimeters*10;
        oi_update(sensor);
        oi_setWheels(spd, spd);            // move forward; full speed //Change?

        while (sum < centimeters && (sensor->bumpRight + sensor->bumpLeft) < 1
                && boundCheck(sensor) == 3 && (stop == toggleFlag)){
            oi_update(sensor);
            sum += sensor->distance;
        }
        oi_setWheels(0, 0);
        return 0;
    }

    int move_forward(oi_t *sensor, int centimeters, int spd){
        extern volatile int toggleFlag;
        int stop = toggleFlag;
        double sum = 0;
        float frontCheck = 100;
        //centimeters = centimeters*10;
        servo_move2(90);
        oi_update(sensor);


        //if(quickScan(sensor) ==0){
            while (sum < centimeters
                    && (sensor->bumpRight + sensor->bumpLeft)<1
                    && boundCheck(sensor)==3
                    && (stop == toggleFlag)
                    && frontCheck >5){
                oi_setWheels(spd, spd);            // move forward; full speed //Change?
                frontCheck = ping_distance();
                oi_update(sensor);
                sum += sensor->distance;
             }
//        }else{
//            turn_any(sensor, -30);
//        }
         oi_setWheels(0, 0);               // stop

         if( (sensor->bumpRight + sensor->bumpLeft >=1) || (boundCheck(sensor)!=3) ){
             turn_any(sensor, -30);
             return centimeters - sum;
         }
         else{
             return 0;
         }
    }



    void move_forward_bump(oi_t *sensor, int centimeters, int spd){
        extern volatile int toggleFlag;
        int stop = toggleFlag;
        double sum = 0;
        char out[100]= "";
        //centimeters = centimeters*10;
        //oi_update(sensor);
        //oi_setWheels(spd, spd);            // move forward; full speed //Change?
        int temp = quickScan(sensor);
        timer_waitMillis(500);
        int stopper = toggleFlag;

        if(centimeters==0){ //DEBUG
            sprintf(out, " - - 0 CENTIMETERS - - %d\n\r");
            puttyPrint(out);
        }

        if(temp ==0){
            oi_setWheels(spd, spd);
            while (sum < centimeters
                    && (sensor->bumpRight + sensor->bumpLeft)<1
                    && boundCheck(sensor)==3
                    && (stop == toggleFlag)){
                oi_update(sensor);
                sum += sensor->distance;
             }
        }else if(temp==1){
            turn_any(sensor, 10);
        }else if(temp==2){
            turn_any(sensor, -10);
        }
        timer_waitMillis(150);
        oi_setWheels(0, 0);               // stop
        if (stopper == toggleFlag){
            if (sensor->bumpRight > 0 || boundCheck(sensor) > 3){
                sprintf(out, " - - BUMP RIGHT - - %d\n\r", boundCheck(sensor));
                puttyPrint(out);
//                move_reverse(sensor, 100);
//                if (boundCheck(sensor) == 5){
//                    turn_any(sensor, 15);
//                }
//                else{
//                    turn_any(sensor, 45 + (45 * collides));
//                    ++collides;
//                    sprintf(out, "Collides: %d\n\r", collides);
//                    puttyPrint(out);
//                }
                if(sensor->bumpRight ==0){
                    boundary_turn(sensor);
                }else{
                    move_reverse(sensor, 100);
                    turn_any(sensor, 45 + (45 * collides));
                    ++collides;
                    sprintf(out, "Collides: %d\n\r", collides);
                    puttyPrint(out);
                }

                move_forward_bump(sensor, centimeters, spd);  // ?
            }

            else if (sensor->bumpLeft > 0 || boundCheck(sensor) < 3){
                sprintf(out, " - - BUMP LEFT - - %d\n\r", boundCheck(sensor));
                puttyPrint(out);
//                move_reverse(sensor, 100);
//                if (boundCheck(sensor) == 1){
//                    turn_any(sensor, -15);
//                }
//                else{
//                    turn_any(sensor, -45 - (45 * collides));
//                    ++collides;
//                    sprintf(out, "Collides: %d\n\r", collides);
//                    puttyPrint(out);

                if(sensor->bumpLeft ==0){
                    boundary_turn(sensor);
                }else{
                    move_reverse(sensor, 100);
                    turn_any(sensor, -45 - (45 * collides));
                    ++collides;
                    sprintf(out, "Collides: %d\n\r", collides);
                    puttyPrint(out);
                }

                move_forward_bump(sensor, centimeters, spd);   //?

        }

    }

        if(collides>1){
            collides = 0;
        }
    }








    /*
    //angles: +counterclock, -clockwise
    void turn_clockwise(oi_t *sensor, int degrees){
        double sum = 0;

        degrees = 0-degrees; //Set to negative

        oi_setWheels(-80, 80);             // right, left
        while (sum > degrees) {
            oi_update(sensor);
            sum += sensor->angle;
        }
        oi_setWheels(0, 0);               // stop
    }



        //angles: +counterclock, -clockwise
    void turn_counterclockwise(oi_t *sensor, int degrees){
        double sum = 0;

        oi_setWheels(80, -80);             // right, left
        while (sum < degrees){
            oi_update(sensor);
            sum += sensor->angle;
        }
        oi_setWheels(0, 0);               // stop
    }

     */






    int cal(void){
        timer_init();
        lcd_init();
        cyBOT_init_Scan(0b111);
        cyBOT_SERVO_cal();

        return 0;
    }



    void textDrive(oi_t *sensor, char command){
        if(command == 'w'){
           move_forward(sensor, 100, 100);
        }
        else if(command == 's'){
            move_reverse(sensor, 100);
        }
        else if(command == 'd'){
            turn_any(sensor, -45);
        }
        else if(command == 'a'){
            turn_any(sensor, 45);
        }
        else{
            oi_setWheels(0,0);
        }

    }





