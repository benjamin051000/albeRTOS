#pragma once

#include <cstdint>

namespace albeRTOS {

class ScopedCriticalSection {
	int32_t imask_state; // The current state of the instruction mask (I-bit)
public:
	ScopedCriticalSection(); 
	~ScopedCriticalSection(); 
};

} // end of namespace albeRTOS
