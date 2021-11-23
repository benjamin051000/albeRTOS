/*
 * G8RTOS_Scheduler.h
 */
#pragma once

#include <albertOS.h>

using threadID = uint32_t;
using ThreadFuncPtr = void(*)();

const unsigned MAX_THREADS = 25;
const unsigned MAX_PTHREADS = 6;
const unsigned STACK_SIZE = 512;
const int OSINT_PRIORITY = 7;

const unsigned MAX_NAME_LEN = 16;


enum sched_errCode {
    NO_ERROR                  = 0,
    THREAD_LIMIT_REACHED      = -1,
    NO_THREADS_SCHEDULED      = -2,
    THREADS_INCORRECTLY_ALIVE = -3,
    THREAD_DOES_NOT_EXIST     = -4,
    CANNOT_KILL_LAST_THREAD   = -5,
    IRQn_INVALID              = -6,
    HWI_PRIORITY_INVALID      = -7
};

namespace albertOS {
/*
 * Initializes variables and hardware for G8RTOS usage
 */
void init();

/**
 * startContextSwitch
 * Start a context switch by triggering the PendSV interrupt.
 */
inline void startContextSwitch() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes Systick Timer
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler.
 * This will only return if the scheduler fails
 */
extern "C" sched_errCode launch();

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_errCode addThread(ThreadFuncPtr threadToAdd, uint8_t priorityLevel, const char *name);

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_errCode addPeriodicEvent(ThreadFuncPtr PthreadToAdd, uint32_t period);


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void sleep(uint32_t durationMS);

//returns the currently running thread's ID
threadID getThreadID();

sched_errCode killThread(threadID threadID);

//kills currently running thread
sched_errCode killSelf();

//kills all threads except currently running
sched_errCode killAll();

//adds an aperiodic event, like an interrupt
sched_errCode addAperiodicEvent(ThreadFuncPtr AthreadToAdd, uint8_t priority, IRQn_Type IRQn);

} // end of namespace albertOS
