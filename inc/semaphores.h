#pragma once

#include <cstdint>


namespace albeRTOS {

class Semaphore {
	int32_t value;
public:
	Semaphore(const int32_t value);
	void wait();
	void signal();
};

} // end of namespace albeRTOS
