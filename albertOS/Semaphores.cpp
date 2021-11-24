/*
 * Semaphores.cpp
 */

#include <albertOS.h>

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void albertOS::initSemaphore(Semaphore* s, int32_t value)
{
	int32_t status = StartCriticalSection();

	(*s) = value;

	EndCriticalSection(status);
}

/*
 * No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread is sempahore is unavalible
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void albertOS::waitSemaphore(Semaphore* s)
{
    int32_t status = StartCriticalSection();

    (*s) -= 1; //declare ownership

    if((*s) < 0)
    {
        currentThread->blocked = s; //block thread
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //yield to allow other threads to run
    }

    EndCriticalSection(status);
}

/*
 * Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void albertOS::signalSemaphore(Semaphore* s)
{
    int32_t status = StartCriticalSection();

    (*s) += 1;

    if((*s) <= 0)
    {
        TCB* ptr = currentThread->next;
        while(ptr->blocked != s) //unblock first thread associated with this semaphore
        {
            ptr = ptr->next;
        }

        *(ptr->blocked) = 0; //make it unblocked
    }

    EndCriticalSection(status);
}
