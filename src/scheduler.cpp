#include <scheduler.h>
#include <structures.h>
#include <criticalsection.h>

/* G8RTOS_Start exists in asm */
extern "C" [[noreturn]] void start_RTOS(void);

/* Holds the current time for the whole System */
// TODO this has to be declared in global scope (external linkage) for some reason. Not sure why
uint32_t systemTime;

/* System Core Clock From system_msp432p401r.c */
//extern uint32_t SystemCoreClock;

/* Pointer to the currently running Thread Control Block */
TCB* currentThread;

/* Manually sets the PendSV interrupt flag to trigger a context switch. */
void contextSwitch() {
	// TODO this is architecture-specific!
    // SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/* Private namespace */
namespace {

/* Status Register with the Thumb-bit Set */
const auto THUMBBIT = 0x01000000;

/* An array of thread control blocks to hold info for each thread */
TCB threadControlBlocks[MAX_THREADS];

/* A matrix which holds individual stacks for each thread */
intptr_t threadStacks[MAX_THREADS][STACKSIZE];

/* Memory to hold period event thread info */
PTCB pThreads[MAX_PTHREADS];

/* Number of Threads currently in the scheduler */
unsigned numThreads;

/* Current Number of Periodic Threads currently in the scheduler */
unsigned numPThreads;

/* Used to assign a unique ID to every thread */
unsigned IDCounter;

} // end of private namespace


/*
 * Chooses the next thread to run.
 *  Must be 'extern "C"' because this function is called from assembly source.
 */
extern "C" void G8RTOS_Scheduler() {
    unsigned currentMaxPriority = 256;

    TCB* tempNextThread = currentThread->next;
//    TCB& tempNextThread = &currentThread->next;

    for(unsigned i = 0; i < numThreads; i++) { //iterates through all threads

        if(!tempNextThread->asleep && tempNextThread->blocked == 0) {//  || (*tempNextThread->blocked) != 0)) { // not asleep, not blocked

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
void albeRTOS::init() {
	systemTime = 0;
	numThreads = 0;
	numPThreads = 0;
	IDCounter = 0;

	// Relocate vector table
	// TODO this is architecture-specific!
	// uint32_t newVTORTable = 0x20000000;
	// memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
	// SCB->VTOR = newVTORTable;

	// Set clock speed to 48MHz, disable watchdog
	// TODO this is architecture-specific!
	// BSP_InitBoard();

	// Add an idle thread, which ensures there is always at least 1 thread available.
	addThread([](){while(true);}, 255, (char*)"idle");
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the SysTick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
[[noreturn]] sched_ErrCode albeRTOS::launch() {
    currentThread = &threadControlBlocks[0]; //set arbitary thread
    for(unsigned i = 0; i < numThreads; i++) {
        if(threadControlBlocks[i].priority < currentThread->priority) {
            currentThread = &threadControlBlocks[i]; //select thread with highest priority to run first
        }
    }

	// TODO all architecture-specific!
    // SysTick_Config(ClockSys_GetSysFreq() / 1000); // Set SysTick timer to 1 ms
	//set interrupt priority levels PendSV exceptions
	// NVIC_SetPriority(PendSV_IRQn, 6);
	// NVIC_SetPriority(SysTick_IRQn , 6);
	//enable PendSV interrupts
	// NVIC_EnableIRQ(PendSV_IRQn);
	// SysTick_enableInterrupt(); // Start the ticking!

	start_RTOS(); //asm func
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
sched_ErrCode albeRTOS::addThread(TaskFuncPtr threadToAdd, uint8_t priorityLevel, char name[MAX_NAME_LEN]) {
    START_CRIT_SECTION;

    if(numThreads >= MAX_THREADS) {
        END_CRIT_SECTION;
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
        END_CRIT_SECTION;
        return THREADS_INCORRECTLY_ALIVE;
    }

    // Create the new thread control block.
    TCB& newTCB = threadControlBlocks[tcbToInitialize];

    // Initialize TCB contents
    newTCB.sp = &threadStacks[tcbToInitialize][STACKSIZE-16];
	newTCB.asleep = false; // Start awake
	newTCB.sleepCount = 0; // Not asleep
	newTCB.blocked = 0;
	newTCB.priority = priorityLevel;
	newTCB.id = ((IDCounter++) << 16) | tcbToInitialize;
    newTCB.isAlive = true;
	// TODO fix this
    // strcpy(newTCB.name, name);
	(void)name;

	// Initialize thread stack
	// r0 - r12 are irrelevant for initial context (will be immediately overwritten)
	threadStacks[tcbToInitialize][STACKSIZE-3] = (intptr_t)threadToAdd; // Put function address at LR location
	threadStacks[tcbToInitialize][STACKSIZE-2] = (intptr_t)threadToAdd; // Set PC, but not needed for initial context
	threadStacks[tcbToInitialize][STACKSIZE-1] = THUMBBIT; // Set status reg with thumb bit set

	// If this is the only active thread, create a loop.
	if(numThreads == 0) {
	    newTCB.prev = &threadControlBlocks[0];
	    newTCB.next = &threadControlBlocks[0];
	}
	else if(systemTime == 0) {
        newTCB.prev = &threadControlBlocks[tcbToInitialize-1];
        newTCB.prev->next = &newTCB;
        newTCB.next = &threadControlBlocks[0];
        newTCB.next->prev = &newTCB;
	}
	else {
	    newTCB.prev = currentThread;
        newTCB.next = currentThread->next;
	    newTCB.prev->next = &newTCB;
	    newTCB.next->prev = &newTCB;
	}

	numThreads++;

	END_CRIT_SECTION;

	return NO_ERROR;
}

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param pThreads To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode albeRTOS::addPeriodicEvent(TaskFuncPtr pThreadToAdd, uint32_t period) {
    START_CRIT_SECTION;

    if(numPThreads >= MAX_PTHREADS) {
        END_CRIT_SECTION;
        return THREAD_LIMIT_REACHED;
    }

    // Create a new periodic thread.
    PTCB& newPTCB = pThreads[numPThreads];

    newPTCB.currentTime = systemTime;
    newPTCB.period = period;
    newPTCB.executeTime = systemTime + newPTCB.period;
    newPTCB.handler = pThreadToAdd;

    if(numPThreads == 0) {
        newPTCB.prev = &pThreads[numPThreads];
        newPTCB.next = &pThreads[numPThreads];
    }
    else {
        newPTCB.prev = &pThreads[numPThreads-1];
        newPTCB.prev->next = &pThreads[numPThreads];
        newPTCB.next = &pThreads[0];
        newPTCB.next->prev = &pThreads[numPThreads];
    }

    numPThreads++;

    END_CRIT_SECTION;
    return NO_ERROR;
}


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void albeRTOS::sleep(uint32_t durationMS) {
    currentThread->sleepCount = durationMS + systemTime;
    currentThread->asleep = true;
    contextSwitch(); //yield to allow other threads to run
}

//returns the currently running thread's ID
inline threadID albeRTOS::getThreadID() {
    return currentThread->id;
}

sched_ErrCode albeRTOS::killThread(threadID threadID) {
    START_CRIT_SECTION;

    if(numThreads == 1) {
        END_CRIT_SECTION;
        return CANNOT_KILL_LAST_THREAD;
    }

    int threadToKill = -1;

    for(unsigned i = 0; i < numThreads; i++) {
        if(threadControlBlocks[i].id == threadID) {
            threadToKill = i;
            break;
        }
    }
    // If we didn't find the thread, this ID is invalid (or the thread has mysteriously disappeared...)
    if(threadToKill == -1) {
        END_CRIT_SECTION;
        return THREAD_DOES_NOT_EXIST;
    }

    threadControlBlocks[threadToKill].isAlive = 0; // RIP
    threadControlBlocks[threadToKill].next->prev = threadControlBlocks[threadToKill].prev;
    threadControlBlocks[threadToKill].prev->next = threadControlBlocks[threadToKill].next; // Update pointers

    contextSwitch();

    numThreads--;

    END_CRIT_SECTION;
    return NO_ERROR;
}

// Kills currently running thread
sched_ErrCode albeRTOS::killSelf() {
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

    killThread(getThreadID());

    return NO_ERROR;
}

// Adds an aperiodic event, like an interrupt
sched_ErrCode albeRTOS::addAPeriodicEvent(TaskFuncPtr AthreadToAdd, uint8_t priority, IRQn_Type IRQn) {
    START_CRIT_SECTION;

	// TODO BUG horribly broken. this is currently architecture-specific.
	if(true) {
    // if(IRQn < PSS_IRQn || IRQn > PORT6_IRQn) {
		(void)IRQn;
        END_CRIT_SECTION;
        return IRQn_INVALID;
    }

    if(priority > 6) {
        END_CRIT_SECTION;
        return HWI_PRIORITY_INVALID;
    }

    // TODO used to be prepended with __, but the macro should resolve to that anyway. Ensure still works.
	// TODO architecture-specific!
    // NVIC_SetVector(IRQn, (uint32_t)AthreadToAdd);
	(void)AthreadToAdd;
    // NVIC_SetPriority(IRQn, priority);
    // NVIC_EnableIRQ(IRQn);

    END_CRIT_SECTION;
    return NO_ERROR;
}

sched_ErrCode albeRTOS::killAll() {

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
