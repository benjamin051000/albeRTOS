#ifdef TODOREMOVE
#include <stdint.h>
#include <stdbool.h>

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
#endif
