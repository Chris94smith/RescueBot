

#ifndef CUSTOM_H_
#define CUSTOM_H_
#include "open_interface.h"

//Print string to putty
void puttyPrint(char out[]);


//Raw IR to distance
float convertIR(int rawIR);

//
int scanIR(int samples);

int boundCheck(oi_t *sensor);

int quickScan(oi_t *sensor);


#endif 
