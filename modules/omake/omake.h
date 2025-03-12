#ifndef OMAKE_H
#define OMAKE_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include <chrono>
#include <cstdint>

class Omake : public RefCounted {
	GDCLASS(Omake, RefCounted)

protected:
	static void _bind_methods();

public:
	Omake();
	~Omake();

	static uint64_t get_cpu_ticks_nsec();
};

#endif // OMAKE_H
