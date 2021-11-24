/*
 * G8RTOS_Semaphores.h
 */

#ifndef G8RTOS_SEMAPHORES_H_
#define G8RTOS_SEMAPHORES_H_

/*********************************************** Datatype Definitions *****************************************************************/

/*
 * Semaphore typedef
 */
typedef int32_t Semaphore;

/*********************************************** Datatype Definitions *****************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 */
void G8RTOS_InitSemaphore(Semaphore *s, int32_t value);

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 */
void G8RTOS_WaitSemaphore(Semaphore *s);

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 */
void G8RTOS_SignalSemaphore(Semaphore *s);

/*********************************************** Public Functions *********************************************************************/


#endif /* G8RTOS_SEMAPHORES_H_ */
