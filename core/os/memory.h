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
#include <string_view>
#include <cstring>

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


// Hash string with FNV-1a
constexpr uint32_t hash_string(std::string_view str) {
    const uint32_t fnv_prime = 0x811C9DC5;
    uint32_t hash = 0;

    for (char c : str) {
        hash ^= c;
        hash *= fnv_prime;
    }
    return hash;
}

#include <iostream>
#include <execinfo.h>
#include <cxxabi.h>
#include <string>
#include <cstdlib>
#include <dlfcn.h>
#include <unistd.h>
#include <cstdio>


void print_stack_trace();


bool starts_with(const std::string& str, const std::string& prefix);


constexpr bool _is_in_types(const uint32_t types[], const size_t length, const uint32_t type_sig) {
	for (size_t i=0; i<length; ++i) {
		if (types[i] == type_sig) {
			return true;
		}
	}
	return false;
}

constexpr bool is_type_gdscript(const uint32_t type_sig) {
	constexpr uint32_t  _types_gdscript[] = {
		hash_string("GDScript"),
		hash_string("GDScriptParser"),
	};
	constexpr size_t length = sizeof(_types_gdscript) / sizeof(_types_gdscript[0]);
	return _is_in_types(_types_gdscript, length, type_sig);
}

constexpr bool is_type_collection(const uint32_t type_sig) {
	constexpr uint32_t _types_collection[] = {
		hash_string("ArrayPrivate"),
		hash_string("DictionaryPrivate"),
		hash_string("HashMapElement"),
		hash_string("HashMap"),
		hash_string("Variant"),
	};
	const size_t length = sizeof(_types_collection) / sizeof(_types_collection[0]);
	return _is_in_types(_types_collection, length, type_sig);
}

constexpr bool is_type_physics(const uint32_t type_sig) {
	constexpr uint32_t _types_physics[] = {
		hash_string("RigidBody3D"),
		hash_string("StaticBody3D"),
		hash_string("CharacterBody3D"),
	};

	const size_t length = sizeof(_types_physics) / sizeof(_types_physics[0]);
	return _is_in_types(_types_physics, length, type_sig);
}

constexpr bool is_type_image(const uint32_t type_sig) {
	constexpr uint32_t _types_image[] = {
		hash_string("Image"),
	};
	const size_t length = sizeof(_types_image) / sizeof(_types_image[0]);
	return _is_in_types(_types_image, length, type_sig);
}

constexpr bool is_type_control(const uint32_t type_sig) {
/*
	const std::vector<uint32_t> fucks = {
		hash_string("Window"),
		hash_string("Label"),
		hash_string("Button"),
		hash_string("VBoxContainer"),
		hash_string("HBoxContainer"),
	};

	for (size_t i=0; i<fucks.size(); i++) {
		if (fucks[i] == type_sig) {
			return true;
		}
	}
*/
	return false;
}

constexpr bool is_type_font(const uint32_t type_sig) {
/*
	const std::vector<uint32_t> fucks = {
		hash_string("FontFile"),
		hash_string("FontVariation"),
	};

	for (size_t i=0; i<fucks.size(); i++) {
		if (fucks[i] == type_sig) {
			return true;
		}
	}
*/
	return false;
}

constexpr bool is_type_string(const uint32_t type_sig) {
	constexpr uint32_t _types_string[] = {
		hash_string("StringName"),
	};

	const size_t length = sizeof(_types_string) / sizeof(_types_string[0]);
	return _is_in_types(_types_string, length, type_sig);
}

constexpr bool is_type_texture(const uint32_t type_sig) {
/*
	const std::vector<uint32_t> fucks = {
		hash_string("ImageTexture"),
		hash_string("ViewportTexture"),
		hash_string("TextureRect"),
		hash_string("TextureButton"),
		hash_string("CompressedTexture2D"),
	};

	for (size_t i=0; i<fucks.size(); i++) {
		if (fucks[i] == type_sig) {
			return true;
		}
	}
*/
	return false;
}

constexpr bool is_type_rendering(const uint32_t type_sig) {
/*
	const std::vector<uint32_t> fucks = {
		hash_string("RenderingDevice"),
		hash_string("RenderingServerDefault"),
		hash_string("RenderingDeviceDriverVulkan"),
		hash_string("RenderingContextDriverVulkan"),
		hash_string("RenderingDeviceGraph"),
	};

	for (size_t i=0; i<fucks.size(); i++) {
		if (fucks[i] == type_sig) {
			return true;
		}
	}
*/
	return false;
}

template <size_t Size>
struct Arena {
	size_t m_used;
	uint8_t* m_buffer = nullptr;
	bool is_initialized;

	explicit Arena() {
		is_initialized = true;
	}

	~Arena() {
		delete[] m_buffer;
	}

	template <typename T, typename... Args>
	T* allocate(Args&&... args) {
		if (m_buffer == nullptr) {
			m_buffer = new uint8_t[Size];
		}

		// Get the memory location
		size_t alignment = alignof(T);
		uintptr_t current = reinterpret_cast<uintptr_t>(m_buffer + m_used);
		uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
		size_t padding = aligned - current;

		// Make sure there is enough space
		size_t total_size = sizeof(T);
		if (m_used + total_size + padding > Size) {
			std::cerr << "!!! Out of memory!" << std::endl;
			std::cerr.flush();
			print_stack_trace();
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


extern Arena<1024 * 1024 * 512> g_memory_arena_images;
extern Arena<1024 * 1024 * 512> g_memory_arena_code;
extern Arena<1024 * 1024 * 512> g_memory_arena_collections;
extern Arena<1024 * 1024 * 512> g_memory_arena_physics;
extern Arena<1024 * 1024 * 512> g_memory_arena_controls;
extern Arena<1024 * 1024 * 512> g_memory_arena_fonts;
extern Arena<1024 * 1024 * 512> g_memory_arena_string;



template <typename T>
constexpr std::string_view _get_type_name() {
	std::string_view func_name = __PRETTY_FUNCTION__;
	size_t pos = std::string::npos;

	// Get the string after opening [T = ?]
	pos = func_name.find(" [T = ");
	if (pos != std::string::npos) {
		func_name = func_name.substr(pos + 6);
	}

	// Get the string before closing [T = ?]
	pos = func_name.find("]");
	if (pos != std::string::npos) {
		func_name = func_name.substr(0, pos);
	}

	// Get the string before any <
	pos = func_name.find("<");
	if (pos != std::string::npos) {
		func_name = func_name.substr(0, pos);
	}

	// Get the string before any ::
	pos = func_name.find("::");
	if (pos != std::string::npos) {
		func_name = func_name.substr(0, pos);
	}

	return func_name;
}

template<typename T>
constexpr uint32_t _get_type_sig() {
    std::string_view type_name = _get_type_name<T>();
    return hash_string(type_name);
}

template <typename T>
void print_type_info(const char* message) {
	if (g_is_logging) {
		std::string_view type_name = _get_type_name<T>();
		std::cout << "???? " << message << ": type:" << type_name << std::endl;
		std::cout.flush();
	}
}

#define memnewOldWithArgs2(T, m_class) \
({ \
	uint32_t type_sig = _get_type_sig<T>(); \
	T* result = nullptr; \
	 \
	if (is_type_gdscript(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! gdscript memnewOldWithArgs2"); \
	} else if (is_type_collection(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! collection memnewOldWithArgs2"); \
	} else if (is_type_physics(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! physics memnewOldWithArgs2"); \
	} else if (is_type_image(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! images memnewOldWithArgs2"); \
	} else if (is_type_control(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! control memnewOldWithArgs2"); \
	} else if (is_type_font(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! fonts memnewOldWithArgs2"); \
	} else if (is_type_string(type_sig)) { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! string memnewOldWithArgs2"); \
	} else { \
		result = new ("") m_class; \
		print_type_info<T>("!!!!!!!! none memnewOldWithArgs2"); \
	} \
	 \
	_post_initialize(result); \
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
	uint32_t type_sig = _get_type_sig<T>(); \
	T* result = nullptr; \
	 \
	if (is_type_gdscript(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! gdscript memnewOldNoConstructor"); \
	} else if (is_type_collection(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! collection memnewOldNoConstructor"); \
	} else if (is_type_physics(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! physics memnewOldNoConstructor"); \
	} else if (is_type_image(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! images memnewOldNoConstructor"); \
	} else if (is_type_control(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! control memnewOldNoConstructor"); \
	} else if (is_type_font(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! fonts memnewOldNoConstructor"); \
	} else if (is_type_string(type_sig)) { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! string memnewOldNoConstructor"); \
	} else { \
		result = new ("") T; \
		print_type_info<T>("!!!!!!!! none memnewOldNoConstructor"); \
	} \
	 \
	_post_initialize(result); \
	result; \
})

template <typename T, typename... Args>
/*_ALWAYS_INLINE_*/ T* memnewWithArgs(Args&&... args) {
	constexpr uint32_t type_sig = _get_type_sig<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_sig)) {
		result = g_memory_arena_code.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! gdscript memnewWithArgs");
	} else if (is_type_collection(type_sig)) {
		result = g_memory_arena_collections.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! collection memnewWithArgs");
	} else if (is_type_physics(type_sig)) {
		result = g_memory_arena_physics.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! physics memnewWithArgs");
	} else if (is_type_image(type_sig)) {
		result = g_memory_arena_images.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! images memnewWithArgs");
	} else if (is_type_control(type_sig)) {
		result = g_memory_arena_controls.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! control memnewWithArgs");
	} else if (is_type_font(type_sig)) {
		result = g_memory_arena_fonts.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! fonts memnewWithArgs");
	} else if (is_type_string(type_sig)) {
		result = g_memory_arena_string.allocate<T>(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! string memnewWithArgs");
	} else {
		result = new ("") T(std::forward<Args>(args)...);
		print_type_info<T>("!!!!!!!! none memnewWithArgs");
	}

	postinitialize_handler(result);
	return result;
}

template <typename T>
/*_ALWAYS_INLINE_*/ T* memnewNoConstructor() {
	constexpr uint32_t type_sig = _get_type_sig<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_sig)) {
		result = g_memory_arena_code.allocate<T>();
		print_type_info<T>("!!!!!!!! gdscript memnewNoConstructor");
	} else if (is_type_collection(type_sig)) {
		result = g_memory_arena_collections.allocate<T>();
		print_type_info<T>("!!!!!!!! collection memnewNoConstructor");
	} else if (is_type_physics(type_sig)) {
		result = g_memory_arena_physics.allocate<T>();
		print_type_info<T>("!!!!!!!! physics memnewNoConstructor");
	} else if (is_type_image(type_sig)) {
		result = g_memory_arena_images.allocate<T>();
		print_type_info<T>("!!!!!!!! images memnewNoConstructor");
	} else if (is_type_control(type_sig)) {
		result = g_memory_arena_controls.allocate<T>();
		print_type_info<T>("!!!!!!!! control memnewNoConstructor");
	} else if (is_type_font(type_sig)) {
		result = g_memory_arena_fonts.allocate<T>();
		print_type_info<T>("!!!!!!!! fonts memnewNoConstructor");
	} else if (is_type_string(type_sig)) {
		result = g_memory_arena_string.allocate<T>();
		print_type_info<T>("!!!!!!!! string memnewNoConstructor");
	} else {
		result = new ("") T;
		print_type_info<T>("!!!!!!!! none memnewNoConstructor");
	}

	postinitialize_handler(result);
	return result;
}

template <typename T>
/*_ALWAYS_INLINE_*/ T* memnewNoArgs() {
	constexpr uint32_t type_sig = _get_type_sig<T>();
	T* result = nullptr;

	if (is_type_gdscript(type_sig)) {
		result = g_memory_arena_code.allocate<T>();
		print_type_info<T>("!!!!!!!! gdscript memnewNoArgs");
	} else if (is_type_collection(type_sig)) {
		result = g_memory_arena_collections.allocate<T>();
		print_type_info<T>("!!!!!!!! collection memnewNoArgs");
	} else if (is_type_physics(type_sig)) {
		result = g_memory_arena_physics.allocate<T>();
		print_type_info<T>("!!!!!!!! physics memnewNoArgs");
	} else if (is_type_image(type_sig)) {
		result = g_memory_arena_images.allocate<T>();
		print_type_info<T>("!!!!!!!! images memnewNoArgs");
	} else if (is_type_control(type_sig)) {
		result = g_memory_arena_controls.allocate<T>();
		print_type_info<T>("!!!!!!!! control memnewNoArgs");
	} else if (is_type_font(type_sig)) {
		result = g_memory_arena_fonts.allocate<T>();
		print_type_info<T>("!!!!!!!! fonts memnewNoArgs");
	} else if (is_type_string(type_sig)) {
		result = g_memory_arena_string.allocate<T>();
		print_type_info<T>("!!!!!!!! string memnewNoArgs");
	} else {
		result = new ("") T;
		print_type_info<T>("!!!!!!!! none memnewNoArgs");
	}

	postinitialize_handler(result);
	return result;
}




_ALWAYS_INLINE_ bool predelete_handler(void *) {
	return true;
}

template <typename T>
void memdelete(T *p_class) {
	uint32_t type_sig = _get_type_sig<T>();

	if (!predelete_handler(p_class)) {
		return; // doesn't want to be deleted
	}

	if constexpr (!std::is_trivially_destructible_v<T>) {
		p_class->~T();
	}

	if (is_type_gdscript(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_collection(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_physics(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_image(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_control(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_font(type_sig)) {
		// FIXME: Have the Arena free the memory here
	} else if (is_type_string(type_sig)) {
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
