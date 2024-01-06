/*
 * movement.h
 *
 *  Created on: Sep 5, 2023
 *      Author: clsmith3
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

int move_forward(oi_t*sensor, int centimeters, int spd);
void brake_check(oi_t *sensor);
int move_reverse(oi_t *sensor, int centimeters);
//void turn_clockwise(oi_t*sensor, int degrees);
//void turn_counterclockwise(oi_t *sensor, int degrees);

void turn_any(oi_t *sensor, int degrees);

int cal(void);

void textDrive(oi_t *sensor, char command);

int move_forward_bump(oi_t *sensor, int centimeters, int spd);
int precise_forward(oi_t *sensor, int centimeters, int spd);
void boundary_turn(oi_t *sensor);


#endif /* MOVEMENT_H_ */
