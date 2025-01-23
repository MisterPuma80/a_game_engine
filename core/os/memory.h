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


//void print_stack_trace();


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
	constexpr uint32_t  _types[] = {
		//hash_string("GDScript"),
		//hash_string("GDScriptParser"),
	};
	constexpr size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_collection(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("ArrayPrivate"),
		//hash_string("DictionaryPrivate"),
		//hash_string("HashMapElement"),
		//hash_string("HashMap"),
		//hash_string("Variant"),
	};
	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_physics(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
/*
		hash_string("RigidBody3D"),
		hash_string("StaticBody3D"),
		hash_string("CharacterBody3D"),
		hash_string("DirectionalLight3D"),
		//hash_string("GPUParticles3D"),
		hash_string("MeshInstance3D"),
		//hash_string("Node3D"),
		//hash_string("Area3D"),
		hash_string("CollisionShape3D"),
		//hash_string("NavigationRegion3D"),
		//hash_string("Timer"),
*/
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_image(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		hash_string("Image"),
	};
	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_control(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("Window"),
		//hash_string("Label"),
		//hash_string("Button"),
		//hash_string("VBoxContainer"),
		//hash_string("HBoxContainer"),
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_font(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("FontFile"),
		//hash_string("FontVariation"),
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_string(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("StringName"),
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_texture(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("ImageTexture"),
		//hash_string("ViewportTexture"),
		//hash_string("TextureRect"),
		//hash_string("TextureButton"),
		//hash_string("CompressedTexture2D"),
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}

constexpr bool is_type_rendering(const uint32_t type_sig) {
	constexpr uint32_t _types[] = {
		//hash_string("RenderingDevice"),
		//hash_string("RenderingServerDefault"),
		//hash_string("RenderingDeviceDriverVulkan"),
		//hash_string("RenderingContextDriverVulkan"),
		//hash_string("RenderingDeviceGraph"),
	};

	const size_t length = sizeof(_types) / sizeof(_types[0]);
	return _is_in_types(_types, length, type_sig);
}


struct Arena {
	size_t m_size;
	size_t m_used;
	uint8_t* m_buffer = nullptr;
	bool m_is_valid;

	explicit Arena(bool is_valid=false) {
		m_is_valid = is_valid;
		if (g_is_logging) {
			std::cout << "!!!! Arena constructor: " << std::endl;
			std::cout.flush();
		}
	}

	~Arena() {
		if (g_is_logging) {
			std::cout << "!!!! Arena destructor: " << std::endl;
			std::cout.flush();
		}

		if (m_buffer != nullptr) {
			delete[] m_buffer;
			m_buffer = nullptr;
			if (g_is_logging) {
				std::cout << "???? freeing buffer memory: " << std::endl;
				std::cout.flush();
			}
		}
	}

	template <typename T, typename... Args>
	/*_ALWAYS_INLINE_*/ T* allocate(Args&&... args) {
		if (! m_is_valid) {
			std::cerr << "!!! Arena not valid!" << std::endl;
			std::cerr.flush();
			return nullptr;
		}

		// Allocate the buffer if needed
		if (m_buffer == nullptr) {
			m_size = 1024 * 1024 * 512;
			m_buffer = new uint8_t[m_size];
			if (g_is_logging) {
				std::cout << "???? allocating buffer memory: " << m_size << std::endl;
				std::cout.flush();
			}
		}

		// Get the memory location
		size_t alignment = alignof(T);
		uintptr_t current = reinterpret_cast<uintptr_t>(m_buffer + m_used);
		uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
		size_t padding = aligned - current;

		// Make sure there is enough space
		size_t total_size = sizeof(T);
		if (m_used + total_size + padding > m_size) {
			std::cerr << "!!! Out of memory!" << std::endl;
			std::cerr.flush();
			//print_stack_trace();
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

static Arena init_arena;



extern Arena g_memory_arena_code;
extern Arena g_memory_arena_images;
extern Arena g_memory_arena_collections;
extern Arena g_memory_arena_physics;
extern Arena g_memory_arena_controls;
extern Arena g_memory_arena_fonts;
extern Arena g_memory_arena_string;


template <typename T>
std::string_view _get_type_name() {
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
/*
	// Get the string before any ::
	pos = func_name.find("::");
	if (pos != std::string::npos) {
		func_name = func_name.substr(0, pos);
	}
*/
	return func_name;
}

template <typename T>
std::string_view _get_type_raw_name() {
	std::string_view func_name = __PRETTY_FUNCTION__;
	return func_name;
}

template<typename T>
uint32_t _get_type_sig() {
    std::string_view type_name = _get_type_name<T>();
    return hash_string(type_name);
}
/*
template <typename T>
void print_type_info(const char* message) {
	if (g_is_logging) {
		std::string_view type_name = _get_type_name<T>();
		std::cout << "???? " << message << ": type:" << type_name << std::endl;
		std::cout.flush();
	}
}
*/

enum class ArenaType {
	invalid,
	code,
	collections,
	physics,
	images,
	controls,
	fonts,
	strings,
};

constexpr ArenaType get_arena_type_for_sig(uint32_t type_sig) {
	if (is_type_gdscript(type_sig)) {
		return ArenaType::code;
	} else if (is_type_collection(type_sig)) {
		return ArenaType::collections;
	} else if (is_type_physics(type_sig)) {
		return ArenaType::physics;
	} else if (is_type_image(type_sig)) {
		return ArenaType::images;
	} else if (is_type_control(type_sig)) {
		return ArenaType::controls;
	} else if (is_type_font(type_sig)) {
		return ArenaType::fonts;
	} else if (is_type_string(type_sig)) {
		return ArenaType::strings;
	} else {
		return ArenaType::invalid;
	}
}

template <int ignore = 0>
Arena& get_arena(ArenaType arena_type) {
	switch (arena_type) {
		case ArenaType::invalid:     return init_arena;
		case ArenaType::code:        return g_memory_arena_code;
		case ArenaType::collections: return g_memory_arena_collections;
		case ArenaType::physics:     return g_memory_arena_physics;
		case ArenaType::images:      return g_memory_arena_images; break;
		case ArenaType::controls:    return g_memory_arena_controls;
		case ArenaType::fonts:       return g_memory_arena_fonts;
		case ArenaType::strings:     return g_memory_arena_string;
		default:                     return init_arena;
	}
}

template <int ignore = 0>
constexpr const char* get_arena_name(ArenaType arena_type) {
	switch (arena_type) {
		case ArenaType::invalid:     return "invalid";
		case ArenaType::code:        return "code";
		case ArenaType::collections: return "collections";
		case ArenaType::physics:     return "physics";
		case ArenaType::images:      return "images";
		case ArenaType::controls:    return "controls";
		case ArenaType::fonts:       return "fonts";
		case ArenaType::strings:     return "string";
		default:                     return "invalid";
	}
}

#define memnewOldWithArgs2(T, m_class) \
({ \
	/*print_type_info<T>("!!!!!!!! ????? memnewOldWithArgs2");*/ \
	if (g_is_logging) { \
		std::cout << "???? memnewOldWithArgs2: " << std::endl; \
		std::cout.flush(); \
	}\
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
	/*print_type_info<T>("!!!!!!!! none memnewOldNoConstructor");*/ \
	if (g_is_logging) { \
		std::cout << "???? memnewOldNoConstructor: " << std::endl; \
		std::cout.flush(); \
	}\
	_post_initialize(new ("") T); \
})

template <typename T, typename... Args>
_ALWAYS_INLINE_ T* memnewWithArgs(Args&&... args) {
	const uint32_t type_sig = _get_type_sig<T>();
	//T* result = nullptr;
	const ArenaType arena_type = get_arena_type_for_sig(type_sig);
	Arena& arena = get_arena(arena_type);
	//constexpr const char* name = get_arena_name<arena_type>();
	//const std::string message = "!!!!!!!! " + std::string(name) + " memnewWithArgs";
	//print_type_info<T>(message.c_str());

	if (g_is_logging) {
		std::cout << "?!?!?!?! memnewWithArgs name: " << _get_type_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewWithArgs raw name: " << _get_type_raw_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewWithArgs type_sig: " << type_sig << std::endl;
		std::cout << "?!?!?!?! memnewWithArgs arena_type: " << (int) arena_type << std::endl;
		std::cout << "?!?!?!?! memnewWithArgs arena: " << (long) &arena << std::endl;
		std::cout << "?!?!?!?! memnewWithArgs arena.m_is_valid: " << arena.m_is_valid << std::endl;
		std::cout.flush();
	}

///*
	if (arena_type != ArenaType::invalid) {
		return _post_initialize(arena.allocate<T>(std::forward<Args>(args)...));
	} else {
		return _post_initialize(new ("") T(std::forward<Args>(args)...));
	}
//*/
	//result = new ("") T(std::forward<Args>(args)...);

	//return _post_initialize(result);
}

template <typename T>
_ALWAYS_INLINE_ T* memnewNoConstructor() {
	const uint32_t type_sig = _get_type_sig<T>();
	//T* result = nullptr;
	const ArenaType arena_type = get_arena_type_for_sig(type_sig);
	Arena& arena = get_arena(arena_type);
	//constexpr const char* name = get_arena_name<arena_type>();
	//const std::string message = "!!!!!!!! " + std::string(name) + " memnewNoConstructor";
	//print_type_info<T>(message.c_str());

	if (g_is_logging) {
		std::cout << "?!?!?!?! memnewNoConstructor name: " << _get_type_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewNoConstructor raw name: " << _get_type_raw_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewNoConstructor type_sig: " << type_sig << std::endl;
		std::cout << "?!?!?!?! memnewNoConstructor arena_type: " << (int) arena_type << std::endl;
		std::cout << "?!?!?!?! memnewNoConstructor arena: " << (long) &arena << std::endl;
		std::cout << "?!?!?!?! memnewNoConstructor arena.m_is_valid: " << arena.m_is_valid << std::endl;
		std::cout.flush();
	}

///*
	if (arena_type != ArenaType::invalid) {
		return _post_initialize(arena.allocate<T>());
	} else {
		return _post_initialize(new ("") T);
	}
//*/
	//result = new ("") T;

	//return _post_initialize(result);
}


template <typename T>
_ALWAYS_INLINE_ T* memnewNoArgs() {
	const uint32_t type_sig = _get_type_sig<T>();
	//T* result = nullptr;
	const ArenaType arena_type = get_arena_type_for_sig(type_sig);
	Arena& arena = get_arena(arena_type);
	//constexpr const char* name = get_arena_name<arena_type>();
	//const std::string message = "!!!!!!!! " + std::string(name) + " memnewNoArgs";
	//print_type_info<T>(message.c_str());

	if (g_is_logging) {
		std::cout << "?!?!?!?! memnewNoArgs name: " << _get_type_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewNoArgs raw name: " << _get_type_raw_name<T>() << std::endl;
		std::cout << "?!?!?!?! memnewNoArgs type_sig: " << type_sig << std::endl;
		std::cout << "?!?!?!?! memnewNoArgs arena_type: " << (int) arena_type << std::endl;
		std::cout << "?!?!?!?! memnewNoArgs arena: " << (long) &arena << std::endl;
		std::cout << "?!?!?!?! memnewNoArgs arena.m_is_valid: " << arena.m_is_valid << std::endl;
		std::cout.flush();
	}

///*
	if (arena_type != ArenaType::invalid) {
		return _post_initialize(arena.allocate<T>());
	} else {
		return _post_initialize(new ("") T);
	}
//*/
	//result = new ("") T;

	//return _post_initialize(result);
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

	const ArenaType arena_type = get_arena_type_for_sig(type_sig);
	if (g_is_logging) {
		std::cout << "?!?!?!?! memdelete name: " << _get_type_name<T>() << std::endl;
		std::cout << "?!?!?!?! memdelete raw name: " << _get_type_raw_name<T>() << std::endl;
		std::cout << "?!?!?!?! memdelete type_sig: " << type_sig << std::endl;
		std::cout << "?!?!?!?! memdelete arena_type: " << (int) arena_type << std::endl;
		std::cout.flush();
	}

	if (arena_type == ArenaType::invalid) {
		Memory::free_static(p_class, false);
	}
/*
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
*/
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
