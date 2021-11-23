/*
 * G8RTOS_Scheduler.cpp
 */

#include <albertOS.h>
#include <string.h> // for strcpy()

/* System Core Clock From system_msp432p401r.c */
//extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
extern TCB* currentThread;

/* Holds the current time for the whole System */
extern uint32_t systemTime;


/**
 * Private namespace for local-only data and functions.
 */
namespace {

/* Status Register with the Thumb-bit Set */
const auto THUMBBIT = 0x01000000;


/*
 * Assembly function to start the first thread.
 */
extern "C" void albertOS_Start();


/* Thread Control Blocks
 *  - An array of thread control blocks to hold pertinent information for each thread
 */
TCB threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *  - A n array of arrays that will act as invdividual stacks for each thread
 */
int32_t threadStacks[MAX_THREADS][STACK_SIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread
 */
PTCB PThreads[MAX_PTHREADS];


/*
 * Current Number of Threads currently in the scheduler
 */
uint32_t numberOfThreads;

/*
 * Current Number of Periodic Threads currently in the scheduler
 */
uint32_t numberOfPThreads;

/*
 * Used to assign a unique ID to every thread
 */
uint32_t IDCounter;

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static inline void InitSysTick(uint32_t numCycles) {
    SysTick_Config(numCycles);
}


/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 *  - Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 *  - Check for sleeping and blocked threads
 */
extern "C" void Scheduler() {
//    uint16_t currentMaxPriority = 256;
    uint8_t currentMaxPriority = 255;
    TCB* tempNextThread = currentThread->next;

    for(unsigned i = 0; i < numberOfThreads; i++) {
//        if(!tempNextThread->asleep && (tempNextThread->blocked != 0  || (*tempNextThread->blocked) != 0)) //not asleep, not blocked
        // TODO investigate above line. What the heck is going  on there?
        if(!tempNextThread->asleep &&
                (tempNextThread->blocked != 0  || (*(&tempNextThread))->blocked != 0)
        ) //not asleep, not blocked
        {
            if(tempNextThread->priority <= currentMaxPriority) {
                currentThread = tempNextThread;
                currentMaxPriority = tempNextThread->priority;
            }
        }
        tempNextThread = tempNextThread->next;
    }
}

} // end of private namespace


/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * set the PendSV flag to start the scheduler,
 * and be responsible for handling sleeping and periodic threads
 */
void SysTick_Handler()
{
    systemTime++;

    for(unsigned j = 0; j < numberOfPThreads; j++)
    {
        if(PThreads[j].executeTime <= systemTime)
        {
            PThreads[j].executeTime = systemTime + PThreads[j].period;
            (*PThreads[j].handler)();
        }
    }

    for(unsigned i = 0; i < MAX_THREADS; i++)
    {
        if((threadControlBlocks[i].asleep == 1) && (threadControlBlocks[i].sleepCount <= systemTime))
        {
            threadControlBlocks[i].asleep = 0; //wake up!
            threadControlBlocks[i].sleepCount = 0;
        }
    }
}

/* Holds the current time for the whole System */
uint32_t systemTime;


/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void albertOS::init()
{
	systemTime = 0;
	numberOfThreads = 0;
	numberOfPThreads = 0;
	IDCounter = 0;

	// relocate vector table
	uint32_t newVTORTable = 0x20000000;
	memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
	SCB->VTOR = newVTORTable;

	// set clock speed to 48MHz, disable watchdog, init hardware
	BSP_InitBoard();
}


/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
extern "C" sched_errCode albertOS::launch() {

    // Choose the first thread to run.
    if(numberOfThreads == 0) {
        return NO_THREADS_SCHEDULED;
    }

    // Choose a thread to run.
    for(uint32_t i = 0; i < numberOfThreads; i++) {
        if(threadControlBlocks[i].priority < currentThread->priority) {
            currentThread = &threadControlBlocks[i];
        }
    }

    // Initialize 1ms interrupt
	InitSysTick(ClockSys_GetSysFreq() / 1000);

	// Set interrupt priority levels
	NVIC_SetPriority(PendSV_IRQn, 6);
	NVIC_SetPriority(SysTick_IRQn , 6);

	// Enable PendSV interrupt
	NVIC_EnableIRQ(PendSV_IRQn);
	SysTick_enableInterrupt(); // Start 1ms interrupt
	albertOS_Start(); // Launch first thread
	return NO_ERROR;
}


/*
 * Add a thread to the Scheduler.
 * 	- Checks if there are still available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and prev tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_errCode albertOS::addThread(ThreadFuncPtr threadToAdd, uint8_t priorityLevel, const char *name) {
    const int status = StartCriticalSection();

    if(numberOfThreads >= MAX_THREADS) {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED;
    }



    // Find a thread that is dead to be replaced.
    // Save its index in the thread control block array.
    int tcbToInitialize = -1;

    for(unsigned i = 0; i < MAX_THREADS; i++) {
        if(!threadControlBlocks[i].isAlive) {
            tcbToInitialize = i;
            break;
        }
    }

    // No threads are dead. Can't add a new thread.
    // TODO BUG: This should be fine, as long as numThreads < MAX_THREADS, right?
    if(tcbToInitialize == -1) {
        EndCriticalSection(status);
        return THREADS_INCORRECTLY_ALIVE;
    }

    // Create the new thread object.
    TCB &newThread = threadControlBlocks[tcbToInitialize];

    newThread = {
        &threadStacks[tcbToInitialize][STACK_SIZE-16], // sp
        nullptr, nullptr, // prev, next
        0, // blocked
        false, // asleep
        0, // sleepCount
        priorityLevel, //priority
        true, // isAlive
        ((IDCounter++) << 16) | tcbToInitialize, // threadID
        name // threadName
    };

    //init tcb stack pointer
//    newThread.sp = &threadStacks[tcbToInitialize][STACK_SIZE-16];
//	newThread.asleep = 0; //we start out awake
//	newThread.sleepCount = 0; //not sleeping
//	newThread.blocked = 0;
//	newThread.priority = priorityLevel; //highest priority is 255
//	newThread.threadID = ((IDCounter++) << 16) | tcbToInitialize;
//    newThread.isAlive = 1;
//    strcpy(newThread.threadName, name);

	// Initialize the thread's stack.
	//r0 - r12 can be anything for initial context (they will be overwritten)
	threadStacks[tcbToInitialize][STACK_SIZE-3] = (int32_t)threadToAdd; //put function address at LR location
	threadStacks[tcbToInitialize][STACK_SIZE-2] = (int32_t)threadToAdd; //set PC, not needed for inital context though
	threadStacks[tcbToInitialize][STACK_SIZE-1] = THUMBBIT; //set status reg with thumb bit set

	// Setup the linked list of threads for the scheduler.
	// TODO I don't like the way these threads are added into the list tbh. Feels weird.
	// I mean this shouldn't even be a linked list. It should be a pqueue. Whatever. TODO good luck with that lol
	if(numberOfThreads == 0) { // Is this the only thread?
	    // Circle this thread back to itself.
	    newThread.prev = &threadControlBlocks[0];
	    newThread.next = &threadControlBlocks[0];
	}
	else if(systemTime == 0) {
	    // This is the last thread in the list.
	    // Connect it to the one before it and thread 0.
        newThread.prev = &threadControlBlocks[tcbToInitialize-1];
        newThread.prev->next = &newThread;
        newThread.next = &threadControlBlocks[0];
        newThread.next->prev = &newThread;
	}
	else {
	    // This thread apparently goes after the current thread.
	    newThread.prev = currentThread;
        newThread.next = currentThread->next;
	    newThread.prev->next = &newThread;
	    newThread.next->prev = &newThread;
	}

	numberOfThreads++;

	EndCriticalSection(status);

	return NO_ERROR;
}


/*
 * Add a periodic threads to the scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_errCode albertOS::addPeriodicEvent(ThreadFuncPtr PThreadToAdd, uint32_t period)
{
    const int status = StartCriticalSection();

    if(numberOfPThreads >= MAX_PTHREADS) {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED;
    }

    // Get location of new PThread in memory.
    PTCB &newPTCB = PThreads[numberOfPThreads];

    // Initialize new PThread.
    newPTCB = {
        nullptr, nullptr, // prev, next
        systemTime, // currentTime
        systemTime + period, // executeTime
        period, // period
        PThreadToAdd  //  handler
    };

//    newPTCB.currentTime = systemTime;
//    newPTCB.executeTime = systemTime + period;
//    newPTCB.period = period;
//    newPTCB.handler = PthreadToAdd;

    if(numberOfPThreads == 0) { // Is this the first PThread?
        newPTCB.prev = &newPTCB; // TODO verify this works instead of // &PThreads[numberOfPThreads];
        newPTCB.next = &newPTCB;
    }
    else {
        newPTCB.prev = &PThreads[numberOfPThreads-1];
        newPTCB.prev->next = &PThreads[numberOfPThreads];
        newPTCB.next = &PThreads[0];
        newPTCB.next->prev = &PThreads[numberOfPThreads];
    }

    numberOfPThreads++;

    EndCriticalSection(status);
    return NO_ERROR; //we created a thread
}


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void albertOS::sleep(uint32_t durationMS) {
    currentThread->sleepCount = systemTime + durationMS;
    currentThread->asleep = true;

    startContextSwitch(); // Yield to allow other threads to run
}


//returns the currently running thread's ID
inline threadID albertOS::getThreadID() {
    return currentThread->threadID;
}


sched_errCode albertOS::killThread(threadID threadID) {
    const int status = StartCriticalSection();

    if(numberOfThreads == 1) {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    // Find the thread object associated with this ID.
    int threadToKill = -1;
    for(unsigned i = 0; i < numberOfThreads; i++)
    {
        if(threadControlBlocks[i].threadID == threadID)
        {
            threadToKill = i;
            break;
        }
    }

    if(threadToKill == -1) {
        EndCriticalSection(status);
        return THREAD_DOES_NOT_EXIST;
    }

    TCB& thread = threadControlBlocks[threadToKill];

    thread.isAlive = false; // rip
    // update pointers
    thread.next->prev = thread.prev;
    thread.prev->next = thread.next;

    // Context switch now in case this thread is the one to be killed.
    // TODO: Only context switch if this is the thread
    // that's being killed, otherwise it should be fine, right?
    startContextSwitch();

    numberOfThreads--;

    EndCriticalSection(status);
    return NO_ERROR;
}


/**
 * Kill the currently running thread.
 */
inline sched_errCode albertOS::killSelf() {
    return killThread(getThreadID());
}


//adds an aperiodic event, like an interrupt
sched_errCode albertOS::addAperiodicEvent(
        ThreadFuncPtr AthreadToAdd,
        uint8_t priority,
        IRQn_Type IRQn)
{
    const int status = StartCriticalSection();

    // Ensure IRQn passed in is within the valid range.
    if(IRQn < PSS_IRQn || IRQn > PORT6_IRQn) {
        EndCriticalSection(status);
        return IRQn_INVALID;
    }
    // Ensure priority is valid
    if(priority > 6) {
        EndCriticalSection(status);
        return HWI_PRIORITY_INVALID;
    }

    NVIC_SetVector(IRQn, (uint32_t)AthreadToAdd);
    NVIC_SetPriority(IRQn, priority);
    NVIC_EnableIRQ(IRQn);

    EndCriticalSection(status);
    return NO_ERROR;
}


/**
 * killAll()
 * Kills all threads besides this one.
 */
sched_errCode albertOS::killAll() {

    for(TCB& thread: threadControlBlocks) {
        if(thread.isAlive && &thread != currentThread) {
            thread.isAlive = false;
        }
    }

//    for(int i = 0; i < MAX_THREADS; i++) {
//        if(threadControlBlocks[i].isAlive && &threadControlBlocks[i] != currentThread) {
//            threadControlBlocks[i].isAlive = 0;
//        }
//    }

    currentThread->next = currentThread;
    currentThread->prev = currentThread;
    numberOfThreads = 1;

    return NO_ERROR;
}
