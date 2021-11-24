/*
 * G8RTOS.h
 */

#ifndef G8RTOS_H_
#define G8RTOS_H_

// STL includes
#include <stdint.h>

// Target-specific includes
#include <msp.h>
#include <BSP.h>
#include <driverlib.h>

// Kernel includes
#include <G8RTOS_Semaphores.h>
#include <G8RTOS_CriticalSection.h>
#include <G8RTOS_Scheduler.h>
#include <G8RTOS_Structures.h> // TODO maybe include before scheduler?
#include <G8RTOS_IPC.h>


#endif /* G8RTOS_H_ */
