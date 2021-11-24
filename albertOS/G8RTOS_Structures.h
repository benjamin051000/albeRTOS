/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include <G8RTOS.h>

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level,
 *      and Blocked Status
 */

typedef struct tcb {
   int32_t* stackPointer;
   struct tcb* previousTCB;
   struct tcb* nextTCB;
   Semaphore* blocked; //0 if not blocked, otherwise holds semaphore of resource its waiting for
   uint32_t sleepCount; //how long left to sleep
   int asleep; //true or false
   uint8_t priority;
   int isAlive; //true or false
   threadId_t threadID;
   char threadName[MAX_NAME_LENGTH];
} tcb_t;

/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */
typedef struct ptcb {
   struct ptcb* previousPTCB;
   struct ptcb* nextPTCB;
   uint32_t currentTime;
   uint32_t executeTime;
   uint32_t period;
   void (*Handler)(void);
} ptcb_t;

/*********************************************** Data Structure Definitions ***********************************************************/


/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;

/*********************************************** Public Variables *********************************************************************/



#endif /* G8RTOS_STRUCTURES_H_ */
