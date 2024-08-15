/*
 * Structures.h
 */
#pragma once


#include <cstdint>
#include "ipc.h"

// TODO this file should not be exposed on the public API. It's for internal kernel use only.

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level,
 *      and Blocked Status
 */
struct TCB {
   int32_t* sp;
   TCB *prev, *next;
   Semaphore* blocked; // 0 if not blocked, otherwise holds semaphore of resource its waiting for
   uint32_t sleepCount; // how long left to sleep
   bool asleep;
   uint8_t priority;
   bool isAlive;
   threadID threadID;
   char name[MAX_NAME_LEN];
};

/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */
struct PTCB {
   PTCB* prev;
   PTCB* next;
   uint32_t currentTime;
   uint32_t executeTime;
   uint32_t period;
   TaskFuncPtr handler;
};

/* Pointer to the current running thread. */
extern TCB* currentThread;
