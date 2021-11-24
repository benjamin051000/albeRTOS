/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include <albertOS.h>

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level,
 *      and Blocked Status
 */

struct tcb_t {
   int32_t* stackPointer;
   tcb_t* previousTCB;
   tcb_t* nextTCB;
   Semaphore* blocked; //0 if not blocked, otherwise holds semaphore of resource its waiting for
   uint32_t sleepCount; //how long left to sleep
   int asleep; //true or false
   uint8_t priority;
   int isAlive; //true or false
   threadID threadID;
   char threadName[MAX_NAME_LEN];
};

/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */
struct ptcb_t {
   ptcb_t* previousPTCB;
   ptcb_t* nextPTCB;
   uint32_t currentTime;
   uint32_t executeTime;
   uint32_t period;
   void (*Handler)(void);
};

/*********************************************** Data Structure Definitions ***********************************************************/


/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;

/*********************************************** Public Variables *********************************************************************/



#endif /* G8RTOS_STRUCTURES_H_ */
