#pragma once

#include "thread.h"
#include <cstdint>

using threadID = uint32_t;
using TaskFuncPtr = void(*)(); // Function pointer alias

const unsigned MAX_THREADS = 25;
const unsigned MAX_PTHREADS = 6;

const unsigned STACKSIZE = 512;
const unsigned OSINT_PRIORITY = 7;
const unsigned MAX_NAME_LEN = 16;


/* Holds the current time for the whole System */
extern uint32_t systemTime;

/**
 * API namespace
 */
namespace albeRTOS {

constexpr auto MAX_NUM_THREADS = 32;

class Scheduler {
	Thread threads[MAX_NUM_THREADS];
	Thread *current_thread;
public:
	Scheduler();
	~Scheduler() = delete;
	// TODO rule of 3? This should really be a singleton

	[[noreturn]] sched_ErrCode launch() const;
	sched_ErrCode add_thread(Thread t); // TODO rvalue?
};

/*
 * Initializes variables and hardware for G8RTOS usage
 */
void init();

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes Systick Timer
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
[[noreturn]] sched_ErrCode launch();

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode addThread(TaskFuncPtr threadToAdd, uint8_t priorityLevel, char name[MAX_NAME_LEN]);

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode addPeriodicEvent(TaskFuncPtr PthreadToAdd, uint32_t period);


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
[[noreturn]] void sleep(uint32_t durationMS);

//returns the currently running thread's ID
threadID getThreadID();

sched_ErrCode killThread(threadID threadID);

//kills currently running thread
sched_ErrCode killSelf();

//kills all threads except currently running
sched_ErrCode killAll();

//adds an aperiodic event, like an interrupt
using IRQn_Type = int32_t; // TODO this is likely a BUG
sched_ErrCode addAPeriodicEvent(TaskFuncPtr AthreadToAdd, uint8_t priority, IRQn_Type IRQn);

[[noreturn]] void contextSwitch();

} // end of namespace albertOS
