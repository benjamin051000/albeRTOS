#pragma once
#include <cstdint>
#include <functional>
#include "semaphores.h"


namespace albeRTOS {

// TODO make this private somehow?
constexpr auto MAX_THREAD_NAME_LEN = 16;

using TaskPtr = void(*)(); // TODO move elsewhere?
// using TaskPtr = std::function<void()>;

class Thread {
	const TaskPtr task; // Has some tiny overhead, TODO consider a normal function pointer instead
	const intptr_t sp; // Stack pointer
	const uint32_t id; // Thread ID
	const uint8_t priority;
	uint32_t asleep_for;
	bool alive;
	Semaphore* blocked_by;
	char name[MAX_THREAD_NAME_LEN];

public:
	Thread(const TaskPtr t, const uint8_t priority, const char name[MAX_THREAD_NAME_LEN]);

	constexpr bool is_asleep() const;
};

} // end of namespace albeRTOS
