/*
 * G8RTOS_Semaphores.h
 */
#pragma once

using Semaphore = int32_t;

namespace albertOS {
/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 */
void initSemaphore(Semaphore &s, int32_t value);

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 */
void waitSemaphore(Semaphore &s);

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 */
void signalSemaphore(Semaphore &s);

} // end of namespace albertOS
