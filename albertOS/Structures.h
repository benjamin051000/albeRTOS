/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */
#pragma once

#include <albertOS.h>

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level,
 *      and Blocked Status
 */
struct TCB {
   int32_t* sp; // Stack pointer
   TCB *prev, *next;
   Semaphore blocked; // 0 if not blocked, otherwise holds semaphore of resource it's waiting for
   bool asleep; // true or false
   uint32_t sleepCount; // how long left to sleep
   uint8_t priority;
   bool isAlive; //true or false
   threadID threadID;
   const char *threadName[MAX_NAME_LEN];

   // TODO add operator overloading for == and !=
};

/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */
struct PTCB {
   PTCB *prev, *next;
   uint32_t currentTime;
   uint32_t executeTime;
   uint32_t period;
   void (*handler)(void);
};

/**
 * Global variable for current thread.
 */
TCB * currentThread;
