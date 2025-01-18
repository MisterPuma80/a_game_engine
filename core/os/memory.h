/**************************************************************************/
/*  memory.h                                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

#include "core/error/error_macros.h"
#include "core/templates/safe_refcount.h"

#include <stddef.h>
#include <new>
#include <type_traits>
#include <cxxabi.h>
#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <typeinfo>
#include <unordered_map>
#include <atomic>
#include <vector>
#include <algorithm>

class Memory {
#ifdef DEBUG_ENABLED
	static SafeNumeric<uint64_t> mem_usage;
	static SafeNumeric<uint64_t> max_usage;
#endif

	static SafeNumeric<uint64_t> alloc_count;

public:
	// Alignment:  ↓ max_align_t        ↓ uint64_t          ↓ max_align_t
	//             ┌─────────────────┬──┬────────────────┬──┬───────────...
	//             │ uint64_t        │░░│ uint64_t       │░░│ T[]
	//             │ alloc size      │░░│ element count  │░░│ data
	//             └─────────────────┴──┴────────────────┴──┴───────────...
	// Offset:     ↑ SIZE_OFFSET        ↑ ELEMENT_OFFSET    ↑ DATA_OFFSET

	static constexpr size_t SIZE_OFFSET = 0;
	static constexpr size_t ELEMENT_OFFSET = ((SIZE_OFFSET + sizeof(uint64_t)) % alignof(uint64_t) == 0) ? (SIZE_OFFSET + sizeof(uint64_t)) : ((SIZE_OFFSET + sizeof(uint64_t)) + alignof(uint64_t) - ((SIZE_OFFSET + sizeof(uint64_t)) % alignof(uint64_t)));
	static constexpr size_t DATA_OFFSET = ((ELEMENT_OFFSET + sizeof(uint64_t)) % alignof(max_align_t) == 0) ? (ELEMENT_OFFSET + sizeof(uint64_t)) : ((ELEMENT_OFFSET + sizeof(uint64_t)) + alignof(max_align_t) - ((ELEMENT_OFFSET + sizeof(uint64_t)) % alignof(max_align_t)));

	static void *alloc_static(size_t p_bytes, bool p_pad_align = false);
	static void *realloc_static(void *p_memory, size_t p_bytes, bool p_pad_align = false);
	static void free_static(void *p_ptr, bool p_pad_align = false);

	static uint64_t get_mem_available();
	static uint64_t get_mem_usage();
	static uint64_t get_mem_max_usage();
};

class DefaultAllocator {
public:
	_FORCE_INLINE_ static void *alloc(size_t p_memory) { return Memory::alloc_static(p_memory, false); }
	_FORCE_INLINE_ static void free(void *p_ptr) { Memory::free_static(p_ptr, false); }
};

void *operator new(size_t p_size, const char *p_description); ///< operator new that takes a description and uses MemoryStaticPool
void *operator new(size_t p_size, void *(*p_allocfunc)(size_t p_size)); ///< operator new that takes a description and uses MemoryStaticPool

void *operator new(size_t p_size, void *p_pointer, size_t check, const char *p_description); ///< operator new that takes a description and uses a pointer to the preallocated memory

#ifdef _MSC_VER
// When compiling with VC++ 2017, the above declarations of placement new generate many irrelevant warnings (C4291).
// The purpose of the following definitions is to muffle these warnings, not to provide a usable implementation of placement delete.
void operator delete(void *p_mem, const char *p_description);
void operator delete(void *p_mem, void *(*p_allocfunc)(size_t p_size));
void operator delete(void *p_mem, void *p_pointer, size_t check, const char *p_description);
#endif

#define memalloc(m_size) Memory::alloc_static(m_size)
#define memrealloc(m_mem, m_size) Memory::realloc_static(m_mem, m_size)
#define memfree(m_mem) Memory::free_static(m_mem)

_ALWAYS_INLINE_ void postinitialize_handler(void *) {}

template <typename T>
_ALWAYS_INLINE_ T *_post_initialize(T *p_obj) {
	postinitialize_handler(p_obj);
	return p_obj;
}

#define memnew_allocator(m_class, m_allocator) _post_initialize(new (m_allocator::alloc) m_class)
#define memnew_placement(m_placement, m_class) _post_initialize(new (m_placement) m_class)

extern bool g_is_logging;

struct Arena {
	size_t m_size;
	size_t m_used;
	uint8_t* m_buffer;

	explicit Arena(size_t size) :
		m_size(size),
		m_used(0) {
		m_buffer = new uint8_t[size];
	}

	~Arena() {
		delete[] m_buffer;
	}

	template <typename T, typename... Args>
	T* allocate(Args&&... args) {
		// Get the memory location
		size_t alignment = alignof(T);
		uintptr_t current = reinterpret_cast<uintptr_t>(m_buffer + m_used);
		uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
		size_t padding = aligned - current;

		// Make sure there is enough space
		size_t total_size = sizeof(T);
		if (m_used + total_size + padding > m_size) {
			std::cerr << "Out of memory" << std::endl;
			std::cerr.flush();
			return nullptr;
		}
		m_used += total_size + padding;

		// Call the constructor and use the memory location
		T* result = new (reinterpret_cast<void*>(aligned)) T(std::forward<Args>(args)...);
		return result;
	}

	void reset() {
		m_used = 0;
	}
};


extern Arena g_memeory_arena_images;
extern Arena g_memeory_arena_code;
extern Arena g_memeory_arena_collections;
extern Arena g_memeory_arena_physics;

bool starts_with(const std::string& str, const std::string& prefix);
bool is_type_gdscript(const int type_id);
bool is_type_collection(const int type_id);
bool is_type_physics(const int type_id);
bool is_type_image(const int type_id);

template <typename T>
std::string _get_type_name() {
	const char* name = typeid(T).name();
    int status = -4;
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };
    return (status == 0) ? res.get() : name;
}

template <typename T>
int _get_type_hash_code() {
	return typeid(T).hash_code();
}

extern std::atomic<int> counter;
extern std::unordered_map<std::size_t, int> hash_to_id;
extern std::unordered_map<std::string, int> name_to_id;
extern std::unordered_map<int, std::string> id_to_name;

template <typename T>
int hash_code_to_type_id() {
	const int hash = _get_type_hash_code<T>();
	auto it = hash_to_id.find(hash);
	if (it != hash_to_id.end()) {
		return it->second;
	} else {
		int id = ++counter;
		std::string type_name = _get_type_name<T>();
		hash_to_id[hash] = id;
		name_to_id[type_name] = id;
		id_to_name[id] = type_name;
		return id;
	}
}

template <typename T>
int t_to_type_id() {
	const std::string type_name = _get_type_name<T>();
	auto it = name_to_id.find(type_name);
	if (it != name_to_id.end()) {
		return it->second;
	} else {
		int id = ++counter;
		int hash = _get_type_hash_code<T>();
		hash_to_id[hash] = id;
		name_to_id[type_name] = id;
		id_to_name[id] = type_name;
		return id;
	}
}

template <typename T>
void print_type_info(const char* message) {
	if (g_is_logging) {
		std::string type_name = _get_type_name<T>();
		std::cout << "???? " << message << ": " << type_name << std::endl;
		std::cout.flush();
	}
}

#define memnewOldWithArgs2(T, m_class) \
({ \
	print_type_info<T>("memnewOldWithArgs2"); \
	_post_initialize(new ("") m_class); \
})

#define memnewOldWithArgs3(name, m_class) \
({ \
	if (g_is_logging) { \
		std::cout << "???? memnewOldWithArgs3: " << name << std::endl; \
		std::cout.flush(); \
	}\
	_post_initialize(new ("") m_class); \
})

#define memnewOldNoConstructor(T) \
({ \
	T* result = new ("") T; \
	print_type_info<T>("memnewOldNoConstructor"); \
	_post_initialize(result); \
	result; \
})

template <typename T, typename... Args>
/*_ALWAYS_INLINE_*/ T* memnewWithArgs(Args&&... args) {
	//const std::string type_name = _get_type_name<T>();
	const int type_id = t_to_type_id<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_id)) {
		result = g_memeory_arena_code.allocate<T>(std::forward<Args>(args)...);
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_collection(type_id)) {
		result = g_memeory_arena_collections.allocate<T>(std::forward<Args>(args)...);
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_physics(type_id)) {
		result = g_memeory_arena_physics.allocate<T>(std::forward<Args>(args)...);
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_image(type_id)) {
		result = g_memeory_arena_images.allocate<T>(std::forward<Args>(args)...);
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else {
		result = new ("") T(std::forward<Args>(args)...);
		//print_type_info<T>("memnewWithArgs");
	}

	postinitialize_handler(result);
	return result;
}

template <typename T>
/*_ALWAYS_INLINE_*/ T* memnewNoConstructor() {
	//const std::string type_name = _get_type_name<T>();
	const int type_id = t_to_type_id<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_id)) {
		result = g_memeory_arena_code.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewNoConstructor");
	} else if (is_type_collection(type_id)) {
		result = g_memeory_arena_collections.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_physics(type_id)) {
		result = g_memeory_arena_physics.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_image(type_id)) {
		result = g_memeory_arena_images.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewNoConstructor");
	} else {
		result = new ("") T;
		//print_type_info<T>("memnewNoConstructor");
	}

	postinitialize_handler(result);
	return result;
}

template <typename T>
/*_ALWAYS_INLINE_*/ T* memnewNoArgs() {
	//const std::string type_name = _get_type_name<T>();
	const int type_id = t_to_type_id<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_id)) {
		result = g_memeory_arena_code.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewNoArgs");
	} else if (is_type_collection(type_id)) {
		result = g_memeory_arena_collections.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_physics(type_id)) {
		result = g_memeory_arena_physics.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewWithArgs");
	} else if (is_type_image(type_id)) {
		result = g_memeory_arena_images.allocate<T>();
		//print_type_info<T>("!!!!!!!!memnewNoArgs");
	} else {
		result = new ("") T;
		//print_type_info<T>("memnewNoArgs");
	}

	postinitialize_handler(result);
	return result;
}




_ALWAYS_INLINE_ bool predelete_handler(void *) {
	return true;
}

template <typename T>
void memdelete(T *p_class) {
	//const std::string type_name = _get_type_name<T>();
	const int type_id = t_to_type_id<T>();

	if (!predelete_handler(p_class)) {
		return; // doesn't want to be deleted
	}

	if constexpr (!std::is_trivially_destructible_v<T>) {
		p_class->~T();
	}

	if (is_type_gdscript(type_id)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_collection(type_id)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_physics(type_id)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_image(type_id)) {
		// FIXME: Have the Arena free the memory here
	} else {
		Memory::free_static(p_class, false);
	}
}

template <typename T, typename A>
void memdelete_allocator(T *p_class) {
	if (!predelete_handler(p_class)) {
		return; // doesn't want to be deleted
	}
	if constexpr (!std::is_trivially_destructible_v<T>) {
		p_class->~T();
	}

	A::free(p_class);
}

#define memdelete_notnull(m_v) \
	{                          \
		if (m_v) {             \
			memdelete(m_v);    \
		}                      \
	}

#define memnew_arr(m_class, m_count) memnew_arr_template<m_class>(m_count)

_FORCE_INLINE_ uint64_t *_get_element_count_ptr(uint8_t *p_ptr) {
	return (uint64_t *)(p_ptr - Memory::DATA_OFFSET + Memory::ELEMENT_OFFSET);
}

template <typename T>
T *memnew_arr_template(size_t p_elements) {
	if (p_elements == 0) {
		return nullptr;
	}
	/** overloading operator new[] cannot be done , because it may not return the real allocated address (it may pad the 'element count' before the actual array). Because of that, it must be done by hand. This is the
	same strategy used by std::vector, and the Vector class, so it should be safe.*/

	size_t len = sizeof(T) * p_elements;
	uint8_t *mem = (uint8_t *)Memory::alloc_static(len, true);
	T *failptr = nullptr; //get rid of a warning
	ERR_FAIL_NULL_V(mem, failptr);

	uint64_t *_elem_count_ptr = _get_element_count_ptr(mem);
	*(_elem_count_ptr) = p_elements;

	if constexpr (!std::is_trivially_constructible_v<T>) {
		T *elems = (T *)mem;

		/* call operator new */
		for (size_t i = 0; i < p_elements; i++) {
			new (&elems[i]) T;
		}
	}

	return (T *)mem;
}

/**
 * Wonders of having own array functions, you can actually check the length of
 * an allocated-with memnew_arr() array
 */

template <typename T>
size_t memarr_len(const T *p_class) {
	uint8_t *ptr = (uint8_t *)p_class;
	uint64_t *_elem_count_ptr = _get_element_count_ptr(ptr);
	return *(_elem_count_ptr);
}

template <typename T>
void memdelete_arr(T *p_class) {
	uint8_t *ptr = (uint8_t *)p_class;

	if constexpr (!std::is_trivially_destructible_v<T>) {
		uint64_t *_elem_count_ptr = _get_element_count_ptr(ptr);
		uint64_t elem_count = *(_elem_count_ptr);

		for (uint64_t i = 0; i < elem_count; i++) {
			p_class[i].~T();
		}
	}

	Memory::free_static(ptr, true);
}

struct _GlobalNil {
	int color = 1;
	_GlobalNil *right = nullptr;
	_GlobalNil *left = nullptr;
	_GlobalNil *parent = nullptr;

	_GlobalNil();
};

struct _GlobalNilClass {
	static _GlobalNil _nil;
};

template <typename T>
class DefaultTypedAllocator {
public:
	template <typename... Args>
	_FORCE_INLINE_ T *new_allocation(const Args &&...p_args) { return memnewWithArgs<T>(p_args...); }
	_FORCE_INLINE_ void delete_allocation(T *p_allocation) { memdelete<T>(p_allocation); }
};

#endif // MEMORY_H
