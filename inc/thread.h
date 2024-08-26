#pragma once
#include <cstdint>
#include "semaphores.h"

enum sched_ErrCode {
    NO_ERROR                  = 0,
    THREAD_LIMIT_REACHED      = -1,
    NO_THREADS_SCHEDULED      = -2,
    THREADS_INCORRECTLY_ALIVE = -3,
    THREAD_DOES_NOT_EXIST     = -4,
    CANNOT_KILL_LAST_THREAD   = -5,
    IRQn_INVALID              = -6,
    HWI_PRIORITY_INVALID      = -7
};


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
	void kill();
};

} // end of namespace albeRTOS
