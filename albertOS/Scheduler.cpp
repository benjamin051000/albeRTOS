/*
 * Scheduler.cpp
 */
#include <albertOS.h>
#include <string.h> // for strcpy()

/*
 * G8RTOS_Start exists in asm
 */
extern "C" void G8RTOS_Start(void);

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
extern TCB * currentThread;

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static TCB threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *	- A n array of arrays that will act as invdividual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread
 */
static PTCB Pthread[MAX_PTHREADS];


/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;

/*
 * Current Number of Periodic Threads currently in the scheduler
 */
static uint32_t NumberOfPThreads;

/*
 * Used to assign a unique ID to every thread
 */
static uint32_t IDCounter;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles) {
    SysTick_Config(numCycles);
}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 *  - Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 *  - Check for sleeping and blocked threads
 */
extern "C" void G8RTOS_Scheduler() {
    uint16_t currentMaxPriority = 256;
    TCB* tempNextThread = currentThread->next;

    for(int i = 0; i < NumberOfThreads; i++) //iterates through all threads
    {
        if(!tempNextThread->asleep && (tempNextThread->blocked != 0  || (*tempNextThread->blocked) != 0)) //not asleep, not blocked
        {
            if(tempNextThread->priority < currentMaxPriority)
            {
                currentThread = tempNextThread;
                currentMaxPriority = tempNextThread->priority;
            }
        }
        tempNextThread = tempNextThread->next;
    }
}

/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * set the PendSV flag to start the scheduler,
 * and be responsible for handling sleeping and periodic threads
 */
// TODO: This must be 'extern "C"' for some reason... figure out why.
extern "C" void SysTick_Handler() {
	systemTime += 1;

	for(int j = 0; j < NumberOfPThreads; j++)
	{
	    if(Pthread[j].executeTime <= systemTime)
	    {
	        Pthread[j].executeTime = systemTime + Pthread[j].period;
	        (*Pthread[j].handler)();
	    }
	}

	for(int i = 0; i < MAX_THREADS; i++)
	{
	    if((threadControlBlocks[i].asleep == 1) && (threadControlBlocks[i].sleepCount <= systemTime))
	    {
	        threadControlBlocks[i].asleep = 0; //wake up!
	        threadControlBlocks[i].sleepCount = 0;
	    }
	}

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //context switch!
}

/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t systemTime;

/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void albertOS::init() {
	systemTime = 0;
	NumberOfThreads = 0;
	NumberOfPThreads = 0;
	IDCounter = 0;

	//relocate vector table
	uint32_t newVTORTable = 0x20000000;
	memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
	SCB->VTOR = newVTORTable;

	//set clk speed to highest possible, disable watchdog, init hardware
	BSP_InitBoard();
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
sched_ErrCode albertOS::launch() {
    currentThread = &threadControlBlocks[0]; //set arbitary thread
    for(int i = 0; i < NumberOfThreads; i++)
    {
        if(threadControlBlocks[i].priority < currentThread->priority)
        {
            currentThread = &threadControlBlocks[i]; //select thread with highest priority to run first
        }
    }
	InitSysTick(ClockSys_GetSysFreq() / 1000); //set to 1ms and enable systick interrupt
	//set interrupt priority levels pendsv exceptions
	NVIC_SetPriority(PendSV_IRQn, 6);
	NVIC_SetPriority(SysTick_IRQn , 6);
	//enable pendsv interrupts
	NVIC_EnableIRQ(PendSV_IRQn);
	SysTick_enableInterrupt(); //start the ticking!
	G8RTOS_Start(); //asm funct

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
    int32_t status = StartCriticalSection();

    if(NumberOfThreads >= MAX_THREADS)
    {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED;
    }

    uint32_t tcbToInitialize = 9999;

    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(threadControlBlocks[i].isAlive != 1)
        {
            tcbToInitialize = i;
            break;
        }
    }

    if(tcbToInitialize == 9999) //no threads are dead
    {
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
    strcpy(threadControlBlocks[tcbToInitialize].threadName, name);

	//ADD FAKE CONTEXT, INIT STACK
	//r0 - r12 are irrelevant for initial context
	threadStacks[tcbToInitialize][STACKSIZE-3] = (int32_t)threadToAdd; //put function address at LR location
	threadStacks[tcbToInitialize][STACKSIZE-2] = (int32_t)threadToAdd; //set PC, not needed for inital context though
	threadStacks[tcbToInitialize][STACKSIZE-1] = THUMBBIT; //set status reg with thumb bit set

	if(NumberOfThreads == 0) //if head node
	{
	    threadControlBlocks[tcbToInitialize].prev = &threadControlBlocks[0]; //first node, switch back to self
	    threadControlBlocks[tcbToInitialize].next = &threadControlBlocks[0];
	}
	else if(systemTime == 0)
	{
        threadControlBlocks[tcbToInitialize].prev = &threadControlBlocks[tcbToInitialize-1];
        threadControlBlocks[tcbToInitialize].prev->next = &threadControlBlocks[tcbToInitialize];
        threadControlBlocks[tcbToInitialize].next = &threadControlBlocks[0];
        threadControlBlocks[tcbToInitialize].next->prev = &threadControlBlocks[tcbToInitialize];
	}
	else
	{
	    threadControlBlocks[tcbToInitialize].prev = currentThread;
        threadControlBlocks[tcbToInitialize].next = currentThread->next;
	    threadControlBlocks[tcbToInitialize].prev->next = &threadControlBlocks[tcbToInitialize];
	    threadControlBlocks[tcbToInitialize].next->prev = &threadControlBlocks[tcbToInitialize];
	}

	NumberOfThreads += 1;

	EndCriticalSection(status);

	return NO_ERROR; //we created a thread
}

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode albertOS::addPeriodicEvent(TaskFuncPtr PthreadToAdd, uint32_t period) {
    int32_t status = StartCriticalSection();

    if(NumberOfPThreads >= MAX_PTHREADS)
    {
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
    if(NumberOfPThreads == 0)
    {
        newPTCB.prev = &Pthread[NumberOfPThreads]; //first node, switch back to self
        newPTCB.next = &Pthread[NumberOfPThreads];
    }
    else
    {
        //doubly linked list fashion insert
        newPTCB.prev = &Pthread[NumberOfPThreads-1];
        newPTCB.prev->next = &Pthread[NumberOfPThreads];
        newPTCB.next = &Pthread[0];
        newPTCB.next->prev = &Pthread[NumberOfPThreads];
    }

    Pthread[NumberOfPThreads] = newPTCB;
    NumberOfPThreads += 1;

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
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //yield to allow other threads to run
}

//returns the currently running thread's ID
threadID albertOS::getThreadID() {
    return currentThread->threadID;
}

sched_ErrCode albertOS::killThread(threadID threadID) {
    int32_t status = StartCriticalSection();

    if(NumberOfThreads == 1)
    {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    uint32_t threadToKill = 99999;

    for(int i = 0; i < NumberOfThreads; i++)
    {
        if(threadControlBlocks[i].threadID == threadID)
        {
            threadToKill = i;
            break;
        }
    }

    if(threadToKill == 99999)
    {
        EndCriticalSection(status);
        return THREAD_DOES_NOT_EXIST;
    }

    threadControlBlocks[threadToKill].isAlive = 0; //rip
    threadControlBlocks[threadToKill].next->prev = threadControlBlocks[threadToKill].prev;
    threadControlBlocks[threadToKill].prev->next = threadControlBlocks[threadToKill].next; //update pointers

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //context switch!

    NumberOfThreads -= 1;

    EndCriticalSection(status);
    return NO_ERROR;
}

//kills currently running thread
sched_ErrCode albertOS::killSelf() {
    int32_t status = StartCriticalSection();

    if(NumberOfThreads == 1)
    {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    currentThread->isAlive = 0; //rip
    currentThread->next->prev = currentThread->prev;
    currentThread->prev->next = currentThread->next; //update pointers

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //context switch!

    NumberOfThreads -= 1;

    EndCriticalSection(status);
    return NO_ERROR;
}

//adds an aperiodic event, like an interrupt
sched_ErrCode albertOS::addAPeriodicEvent(TaskFuncPtr AthreadToAdd, uint8_t priority, IRQn_Type IRQn) {
    int32_t status = StartCriticalSection();

    if(IRQn < PSS_IRQn || IRQn > PORT6_IRQn)
    {
        EndCriticalSection(status);
        return IRQn_INVALID;
    }

    if(priority > 6)
    {
        EndCriticalSection(status);
        return HWI_PRIORITY_INVALID;
    }

    __NVIC_SetVector(IRQn, (uint32_t)AthreadToAdd);
    __NVIC_SetPriority(IRQn, priority);
    __NVIC_EnableIRQ(IRQn);

    EndCriticalSection(status);
    return NO_ERROR;
}

sched_ErrCode albertOS::killAll() {

    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(threadControlBlocks[i].isAlive && &threadControlBlocks[i] != currentThread)
        {
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
    NumberOfThreads = 1;
    return NO_ERROR;
}

/*********************************************** Public Functions *********************************************************************/
