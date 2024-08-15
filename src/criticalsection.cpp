#include <criticalsection.h>

namespace  {

/*
 * Starts a critical section
 * 	- Saves the state of the current PRIMASK (I-bit)
 * 	- Disables interrupts
 * Returns: The current PRIMASK State
 */
extern "C" int32_t StartCriticalSection();

/*
 * Ends a critical Section
 * 	- Restores the state of the PRIMASK given an input
 * Param "IBit_State": PRIMASK State to update
 */
extern "C" void EndCriticalSection(int32_t IBit_State);

} // end of namespace


namespace albeRTOS {

ScopedCriticalSection::ScopedCriticalSection() {
	imask_state = StartCriticalSection();
}

ScopedCriticalSection::~ScopedCriticalSection() {
	EndCriticalSection(imask_state);
}

} // end of namespace albeRTOS
