// elevator.h
//  
#ifdef HW1_ELEVATOR
#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "copyright.h"

struct PersonAttributes
{
    int which;
    int toFloor;
    int atFloor;
};

void Elevator(int numFloors);
void ArrivingGoingFromTo(int atFloor, int toFloor);

#endif
#endif