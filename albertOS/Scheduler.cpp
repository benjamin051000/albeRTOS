/*
 * Scheduler.cpp
 */
#include <albertOS.h>
#include <string.h> // for strcpy()

/* G8RTOS_Start exists in asm */
extern "C" void start_RTOS(void);

/* Holds the current time for the whole System */
// TODO this has to be declared in global scope (external linkage) for some reason. Not sure why
uint32_t systemTime;

/* System Core Clock From system_msp432p401r.c */
//extern uint32_t SystemCoreClock;

/* Pointer to the currently running Thread Control Block */
//extern TCB* currentThread;


/* Private namespace */
namespace {

/* Status Register with the Thumb-bit Set */
const auto THUMBBIT = 0x01000000;

/* An array of thread control blocks to hold info for each thread */
TCB threadControlBlocks[MAX_THREADS];

/* A matrix which holds individual stacks for each thread */
int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Memory to hold period event thread info */
PTCB pThreads[MAX_PTHREADS];

/* Number of Threads currently in the scheduler */
unsigned numThreads;

/* Current Number of Periodic Threads currently in the scheduler */
unsigned numPThreads;

/* Used to assign a unique ID to every thread */
unsigned IDCounter;

/* Manually sets the PendSV interrupt flag to trigger a context switch. */
inline void contextSwitch() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

} // end of private namespace


/*
 * Chooses the next thread to run.
 *  Must be 'extern "C"' because this function is called from assembly source.
 */
extern "C" void G8RTOS_Scheduler() {
    unsigned currentMaxPriority = 256;

    TCB* tempNextThread = currentThread->next;

    for(unsigned i = 0; i < numThreads; i++) { //iterates through all threads

        if(!tempNextThread->asleep && (tempNextThread->blocked != 0  || (*tempNextThread->blocked) != 0)) { //not asleep, not blocked

            if(tempNextThread->priority < currentMaxPriority) {
                currentThread = tempNextThread;
                currentMaxPriority = tempNextThread->priority;
            }
        }

        tempNextThread = tempNextThread->next;

    } // end of for loop
}

/*
 * SysTick Handler
 * Increments system time, sets the PendSV flag to start
 * the scheduler, and handles sleeping and periodic threads.
 */
// TODO: This must be 'extern "C"' for some reason... figure out why.
extern "C" void SysTick_Handler() {
	systemTime++;

	for(unsigned j = 0; j < numPThreads; j++) {
	    if(pThreads[j].executeTime <= systemTime) {
	        pThreads[j].executeTime = systemTime + pThreads[j].period;
	        (*pThreads[j].handler)();
	    }
	}

	for(unsigned i = 0; i < MAX_THREADS; i++) {
	    if((threadControlBlocks[i].asleep == 1) && (threadControlBlocks[i].sleepCount <= systemTime)) {
	        threadControlBlocks[i].asleep = 0; //wake up!
	        threadControlBlocks[i].sleepCount = 0;
	    }
	}

	contextSwitch();
}


/* Sets RTOS to an initial state (system time and number of threads) */
void albertOS::init() {
	systemTime = 0;
	numThreads = 0;
	numPThreads = 0;
	IDCounter = 0;

	// Relocate vector table
	uint32_t newVTORTable = 0x20000000;
	memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
	SCB->VTOR = newVTORTable;

	// Set clock speed to 48MHz, disable watchdog
	BSP_InitBoard();
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the SysTick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
sched_ErrCode albertOS::launch() {
    currentThread = &threadControlBlocks[0]; //set arbitary thread
    for(unsigned i = 0; i < numThreads; i++) {
        if(threadControlBlocks[i].priority < currentThread->priority) {
            currentThread = &threadControlBlocks[i]; //select thread with highest priority to run first
        }
    }

    SysTick_Config(ClockSys_GetSysFreq() / 1000); // Set SysTick timer to 1 ms
	//set interrupt priority levels PendSV exceptions
	NVIC_SetPriority(PendSV_IRQn, 6);
	NVIC_SetPriority(SysTick_IRQn , 6);
	//enable PendSV interrupts
	NVIC_EnableIRQ(PendSV_IRQn);
	SysTick_enableInterrupt(); // Start the ticking!

	start_RTOS(); //asm func

	return NO_ERROR;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are still available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode albertOS::addThread(TaskFuncPtr threadToAdd, uint8_t priorityLevel, char name[MAX_NAME_LEN]) {
    const int32_t status = StartCriticalSection();

    if(numThreads >= MAX_THREADS) {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED;
    }

    // Find a dead thread to replace.
    int tcbToInitialize = -1;
    for(unsigned i = 0; i < MAX_THREADS; i++) {
        if(threadControlBlocks[i].isAlive != 1) {
            tcbToInitialize = i;
            break;
        }
    }

    // No threads are dead.
    // TODO this doesn't seem right. What if one of the threads is simply uninitialized? Maybe they all default to dead.
    if(tcbToInitialize == -1) {
        EndCriticalSection(status);
        return THREADS_INCORRECTLY_ALIVE;
    }

    //init tcb stack pointer
    threadControlBlocks[tcbToInitialize].sp = &threadStacks[tcbToInitialize][STACKSIZE-16];
	threadControlBlocks[tcbToInitialize].asleep = 0; //we start out awake
	threadControlBlocks[tcbToInitialize].sleepCount = 0; //not sleeping
	threadControlBlocks[tcbToInitialize].blocked = 0;
	threadControlBlocks[tcbToInitialize].priority = priorityLevel; //highest priority is 255
	threadControlBlocks[tcbToInitialize].threadID = ((IDCounter++) << 16) | tcbToInitialize;
    threadControlBlocks[tcbToInitialize].isAlive = 1;
    strcpy(threadControlBlocks[tcbToInitialize].name, name);

	//ADD FAKE CONTEXT, INIT STACK
	//r0 - r12 are irrelevant for initial context
	threadStacks[tcbToInitialize][STACKSIZE-3] = (int32_t)threadToAdd; //put function address at LR location
	threadStacks[tcbToInitialize][STACKSIZE-2] = (int32_t)threadToAdd; //set PC, not needed for inital context though
	threadStacks[tcbToInitialize][STACKSIZE-1] = THUMBBIT; //set status reg with thumb bit set

	if(numThreads == 0) { //if head node
	    threadControlBlocks[tcbToInitialize].prev = &threadControlBlocks[0]; //first node, switch back to self
	    threadControlBlocks[tcbToInitialize].next = &threadControlBlocks[0];
	}
	else if(systemTime == 0) {
        threadControlBlocks[tcbToInitialize].prev = &threadControlBlocks[tcbToInitialize-1];
        threadControlBlocks[tcbToInitialize].prev->next = &threadControlBlocks[tcbToInitialize];
        threadControlBlocks[tcbToInitialize].next = &threadControlBlocks[0];
        threadControlBlocks[tcbToInitialize].next->prev = &threadControlBlocks[tcbToInitialize];
	}
	else {
	    threadControlBlocks[tcbToInitialize].prev = currentThread;
        threadControlBlocks[tcbToInitialize].next = currentThread->next;
	    threadControlBlocks[tcbToInitialize].prev->next = &threadControlBlocks[tcbToInitialize];
	    threadControlBlocks[tcbToInitialize].next->prev = &threadControlBlocks[tcbToInitialize];
	}

	numThreads++;

	EndCriticalSection(status);

	return NO_ERROR; //we created a thread
}

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param pThreads To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode albertOS::addPeriodicEvent(TaskFuncPtr PthreadToAdd, uint32_t period) {
    const int32_t status = StartCriticalSection();

    if(numPThreads >= MAX_PTHREADS) {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED; //cant make any more pthreads!
    }

    //init ptcb stack pointer
    PTCB newPTCB;
    newPTCB.currentTime = systemTime;
    newPTCB.executeTime = systemTime + period;
    newPTCB.period = period;
    newPTCB.handler = PthreadToAdd;

    //if head node
    if(numPThreads == 0) {
        newPTCB.prev = &pThreads[numPThreads]; //first node, switch back to self
        newPTCB.next = &pThreads[numPThreads];
    }
    else {
        //doubly linked list fashion insert
        newPTCB.prev = &pThreads[numPThreads-1];
        newPTCB.prev->next = &pThreads[numPThreads];
        newPTCB.next = &pThreads[0];
        newPTCB.next->prev = &pThreads[numPThreads];
    }

    pThreads[numPThreads] = newPTCB;
    numPThreads++;

    EndCriticalSection(status);
    return NO_ERROR; //we created a thread
}


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void albertOS::sleep(uint32_t durationMS) {
    currentThread->sleepCount = durationMS + systemTime;
    currentThread->asleep = true;
    contextSwitch(); //yield to allow other threads to run
}

//returns the currently running thread's ID
inline threadID albertOS::getThreadID() {
    return currentThread->threadID;
}

sched_ErrCode albertOS::killThread(threadID threadID) {
    const int32_t status = StartCriticalSection();

    if(numThreads == 1) {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    int threadToKill = -1;

    for(unsigned i = 0; i < numThreads; i++) {
        if(threadControlBlocks[i].threadID == threadID) {
            threadToKill = i;
            break;
        }
    }
    // If we didn't find the thread, this ID is invalid (or the thread has mysteriously disappeared...)
    if(threadToKill == -1) {
        EndCriticalSection(status);
        return THREAD_DOES_NOT_EXIST;
    }

    threadControlBlocks[threadToKill].isAlive = 0; // RIP
    threadControlBlocks[threadToKill].next->prev = threadControlBlocks[threadToKill].prev;
    threadControlBlocks[threadToKill].prev->next = threadControlBlocks[threadToKill].next; // Update pointers

    contextSwitch();

    numThreads--;

    EndCriticalSection(status);
    return NO_ERROR;
}

// Kills currently running thread
sched_ErrCode albertOS::killSelf() {
//    const int32_t status = StartCriticalSection();
//
//    if(numThreads == 1) {
//        EndCriticalSection(status);
//        return CANNOT_KILL_LAST_THREAD;
//    }
//
//    currentThread->isAlive = 0; // RIP
//    currentThread->next->prev = currentThread->prev;
//    currentThread->prev->next = currentThread->next; // Update pointers
//
//    contextSwitch();
//
//    numThreads -= 1;
//
//    EndCriticalSection(status);

    albertOS::killThread(albertOS::getThreadID());

    return NO_ERROR;
}

// Adds an aperiodic event, like an interrupt
sched_ErrCode albertOS::addAPeriodicEvent(TaskFuncPtr AthreadToAdd, uint8_t priority, IRQn_Type IRQn) {
    const int32_t status = StartCriticalSection();

    if(IRQn < PSS_IRQn || IRQn > PORT6_IRQn) {
        EndCriticalSection(status);
        return IRQn_INVALID;
    }

    if(priority > 6) {
        EndCriticalSection(status);
        return HWI_PRIORITY_INVALID;
    }

    // TODO used to be prepended with __, but the macro should resolve to that anyway. Ensure still works.
    NVIC_SetVector(IRQn, (uint32_t)AthreadToAdd);
    NVIC_SetPriority(IRQn, priority);
    NVIC_EnableIRQ(IRQn);

    EndCriticalSection(status);
    return NO_ERROR;
}

sched_ErrCode albertOS::killAll() {

    for(unsigned i = 0; i < MAX_THREADS; i++) {
        if(threadControlBlocks[i].isAlive && &threadControlBlocks[i] != currentThread) {
            threadControlBlocks[i].isAlive = 0;
        }
    }

//    tcb_t* temp = CurrentlyRunningThread->nextTCB;
//    while(temp != CurrentlyRunningThread)
//    {
//      threadId_t id = temp->threadID;  // Get ID
//      G8RTOS_KillThread(id); // Kill it
//      temp = temp->nextTCB; // ty, next
//    }
    currentThread->next = currentThread;
    currentThread->prev = currentThread;
    numThreads = 1;
    return NO_ERROR;
}
