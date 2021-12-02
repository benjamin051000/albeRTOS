/*
 * Semaphores.cpp
 */
#include <albertOS.h>


void albertOS::initSemaphore(Semaphore& s, int32_t value) {
    START_CRIT_SECTION;

	s = value;

	END_CRIT_SECTION;
}


void albertOS::waitSemaphore(Semaphore& s) {
    START_CRIT_SECTION;

    s--; // declare ownership

    if(s < 0) {
        currentThread->blocked = &s; //block thread
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //yield to allow other threads to run
    }

    END_CRIT_SECTION;
}

/*
 * Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void albertOS::signalSemaphore(Semaphore& s) {
    START_CRIT_SECTION;

    s++;

    if(s <= 0) {
        TCB* ptr = currentThread->next;
        while(ptr->blocked != &s) { // unblock first thread associated with this semaphore
            ptr = ptr->next;
        }

        *(ptr->blocked) = 0; //make it unblocked
    }
    END_CRIT_SECTION;
}
