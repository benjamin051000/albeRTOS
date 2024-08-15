#include <structures.h>
#include <semaphores.h>
#include <criticalsection.h>
#include <scheduler.h>

extern void contextSwitch();

void albeRTOS::initSemaphore(Semaphore& s, int32_t value) {
    START_CRIT_SECTION;
	s = value;
	END_CRIT_SECTION;
}


void albeRTOS::waitSemaphore(Semaphore& s) {
    START_CRIT_SECTION;

    s--; // declare ownership

    // If s < 0, it's currently claimed by another thread.
    if(s < 0) {
        currentThread->blocked = &s; // block this thread
        END_CRIT_SECTION;
        contextSwitch(); // do something else until this thread is unblocked (other thread releases the Semaphore)
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
void albeRTOS::signalSemaphore(Semaphore& s) {
    START_CRIT_SECTION;

    s++;

    // If value leq 0, other threads are blocking for this resource.
    if(s <= 0) {

        const TCB * const curr_thread_temp = currentThread;
        TCB* thread = currentThread->next;

        // Iterate through each thread.
        while(thread != curr_thread_temp) {
            if(thread->blocked == &s) // If the thread is blocked by this Semaphore,
                thread->blocked = 0; // Unblock it.

            thread = thread->next;
        }

//        while(blocked_thread->blocked != &s) { // unblock one
//            blocked_thread = blocked_thread->next;
//        }
//
//        // blocked_thread->blocked == &s, so blocked_thread is blocked by this semaphore!
//        blocked_thread->blocked = nullptr; //make it unblocked
    }
    END_CRIT_SECTION;
}
