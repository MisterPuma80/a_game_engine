
#include "omake.h"
#include "core/object/script_language.h"

Omake::Omake() {
	//print_line("Omake created");
}

Omake::~Omake() {
	//print_line("Omake destroyed");
}

uint64_t Omake::get_cpu_ticks_nsec() {
	auto now = std::chrono::high_resolution_clock::now();
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
	return static_cast<uint64_t>(ns);
}

void Omake::_bind_methods() {
	ClassDB::bind_static_method("Omake", D_METHOD("get_cpu_ticks_nsec"), &Omake::get_cpu_ticks_nsec);
}
