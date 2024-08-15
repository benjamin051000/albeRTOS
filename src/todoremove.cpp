/**
 * WARNING This shoudn't exist. It's just to allow the thing to build.
 * These functions should be defined, one set per architecture.
 */

#ifdef TODOREMOVE
#warning "Using dummy functions for critical sections... the code WILL break!"

#include <cstdint>
#include <cstdbool>

extern "C" {
[[noreturn]] void start_RTOS(void) { while(true); }

/*
 * Starts a critical section
 * 	- Saves the state of the current PRIMASK (I-bit)
 * 	- Disables interrupts
 * Returns: The current PRIMASK State
 */
int32_t StartCriticalSection() { return -1;}

/*
 * Ends a critical Section
 * 	- Restores the state of the PRIMASK given an input
 * Param "IBit_State": PRIMASK State to update
 */
void EndCriticalSection(int32_t IBit_State) { (void)IBit_State; }

}
#endif
