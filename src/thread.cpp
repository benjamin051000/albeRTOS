#include <cstdint>
#include <cstring>
#include <thread.h>

namespace albeRTOS {

Thread::Thread(const TaskPtr t, const uint8_t priority, const char name[MAX_THREAD_NAME_LEN])
    : task(t), sp((intptr_t) nullptr), id(0), priority(priority), asleep_for(0), alive(true), blocked_by(nullptr) {
  strncpy(this->name, name, MAX_THREAD_NAME_LEN);
}

constexpr bool Thread::is_asleep() const {
	return asleep_for > 0;
}

} // namespace albeRTOS
