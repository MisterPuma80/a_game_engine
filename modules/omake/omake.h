#ifndef OMAKE_H
#define OMAKE_H

#include "core/object/ref_counted.h"

void _print_stacktrace();

class Omake : public RefCounted {
	GDCLASS(Omake, RefCounted)

protected:
	static void _bind_methods();

public:
	Omake();
	~Omake();

	static uint64_t get_cpu_ticks_nsec();
	static void print_stacktrace();
};

#endif // OMAKE_H
