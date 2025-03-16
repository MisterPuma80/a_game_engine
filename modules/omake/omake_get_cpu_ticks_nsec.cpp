
#include "omake_get_cpu_ticks_nsec.h"
#include <chrono>
#include <cstdint>

uint64_t omake_get_cpu_ticks_nsec() {
	auto now = std::chrono::high_resolution_clock::now();
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
	return static_cast<uint64_t>(ns);
}
