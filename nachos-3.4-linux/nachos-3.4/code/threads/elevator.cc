#ifdef HW1_ELEVATOR

#include "system.h"
#include "synch.h"
#include "elevator.h"

int person_id = 1;           // always tracks the next person id
Lock* mutex_id = new Lock("id-mutex");

int elevatorFloor = 1;       // always tracks elevator's current floor
bool goingUp = true;         // always tracks if elevator is going up or down
bool elevatorOpen = false;   // always tracks if the elevator is open
int elevatorOccupancy = 0;   // always tracks current occupancy (total is 5)

// mutex and conditional variable for the above five shared variables
Lock *mutexElevator = new Lock("elevator-mutex");
Condition *condvarElevator = new Condition("elevator-condition");

// number of persons actively inside the system
// this includes the ones waiting and the ones inside the elevator
int personsActive = 0;

// mutex and conditional variable for elevator wakeup
Lock *mutexWakeup = new Lock("wakeup-mutex");
Condition *condvarWakeup = new Condition("wakeup-condition");


//-----------------------------------------------------------------------
// elevator_subroutine
//  The job of the elevator subroutine is to make the elevator go up and
//  down the building.
//-----------------------------------------------------------------------
void elevator_subroutine(int numFloors)
{
    DEBUG('h', "Elevator started on floor %d\n", elevatorFloor);
    while(true)
    {
        // go to sleep if there are no persons to serve
        mutexWakeup->Acquire();
        printf("personsActive: %d\n", personsActive);
        while(personsActive == 0) condvarWakeup->Wait(mutexWakeup);
        mutexWakeup->Release();

        mutexElevator->Acquire();
        // loop for 50 ticks - going to next floor
        for(int i=0; i<5000000;)
        {
            // the elevator is now closed - its moving
            if (i == 0)
            {
                elevatorOpen = false;
                DEBUG('h', "Elevator is now closed\n");
            }

            i++;  // spend one tick

            // elevator is on next floor - its open
            if (i == 5000000)
            {
                elevatorOpen = true;
                DEBUG('h', "Elevator is now open\n");
            }
        }

        // elevator has reached the next floor
        if (goingUp)
        {
            elevatorFloor++;
            printf("Elevator arrives on floor %d\n", elevatorFloor);
        }
        else
        {
            elevatorFloor--;
            printf("Elevator arrives on floor %d\n", elevatorFloor);
        }

        // decide whether to go up or down
        if (elevatorFloor == numFloors)
        {
            goingUp = false;
            DEBUG('h', "Elevator will go down now\n");
        }
        if (elevatorFloor == 1)
        {
            goingUp = true;
            DEBUG('h', "Elevator will go up now\n");
        }

        // announce you have reached the next floor to all persons
        DEBUG('h', "All persons told about elevator arrival\n");
        condvarElevator->Broadcast(mutexElevator);
        mutexElevator->Release();
        currentThread->Yield();
    }
}

void person_subroutine(int arg)
{
    PersonAttributes *struct_ptr = reinterpret_cast<PersonAttributes*>(arg);
    int which = struct_ptr->which;
    int toFloor = struct_ptr->toFloor;
    int atFloor = struct_ptr->atFloor;

    DEBUG('h', "Person %d has entered the system\n", which);
    printf("Person %d wants to go to floor %d from floor %d\n",
           which, toFloor, atFloor);

    // wakeup the elevator if it's sleeping
    // a person has entered the system
    mutexWakeup->Acquire();
    personsActive++;
    condvarWakeup->Signal(mutexWakeup);
    mutexWakeup->Release();

    // wait till the elevator arrives at the floor and the occupancy is not
    // more than 4 (only 5 can fit in the elevator)
    mutexElevator->Acquire();

    while(true) {
      bool open_condition = elevatorOpen;
      bool same_direction_condition = (
        (goingUp && (toFloor > atFloor)) || (!goingUp && (toFloor < atFloor))
      );
      bool same_floor_condition = (elevatorFloor == atFloor);
      bool occupancy_condition = (elevatorOccupancy < 5);
      bool dont_wait = (
        open_condition && same_direction_condition &&
        same_floor_condition && occupancy_condition
      );
      DEBUG('h', "Open Condition: %d\n", open_condition);
      DEBUG('h', "Same Direction Condition: %d\n", same_direction_condition);
      DEBUG('h', "Same Floor Condition: %d\n", same_floor_condition);
      DEBUG('h', "Occupancy Condition: %d\n", occupancy_condition);
      if(dont_wait) break;
      else condvarElevator->Wait(mutexElevator);
    }

    // person gets into elevator
    elevatorOccupancy++;
    printf("Person %d got into the elevator\n", which);

    mutexElevator->Release();

    // wait till the elevator reaches the destination floor
    mutexElevator->Acquire();
    while(
        !(elevatorFloor == toFloor) || !elevatorOpen
    ) condvarElevator->Wait(mutexElevator);

    // person gets out of the elevator
    elevatorOccupancy--;
    printf("Person %d got out of the elevator\n", which);
    mutexElevator->Release();

    // person leaves the system
    mutexWakeup->Acquire();
    DEBUG('h', "Person %d has left the system\n", which);
    personsActive--;
    mutexWakeup->Release();
}

void Elevator(int numFloors)
{
    Thread *elevatorThread = new Thread("Elevator");

    elevatorThread->Fork(elevator_subroutine, numFloors);
}

void ArrivingGoingFromTo(int atFloor, int toFloor)
{
    mutex_id->Acquire();
    int which = person_id++;
    mutex_id->Release();

    char threadName[100];
    sprintf(threadName, "Person Thread %d", which);
    Thread *personThread = new Thread(threadName);

    PersonAttributes *struct_ptr = new PersonAttributes;
    struct_ptr->which = which;
    struct_ptr->atFloor = atFloor;
    struct_ptr->toFloor = toFloor;
    int arg = reinterpret_cast<int>(struct_ptr);

    personThread->Fork(person_subroutine, arg);
}
#endif