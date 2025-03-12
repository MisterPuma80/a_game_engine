
#include "omake.h"
#include "core/object/script_language.h"

#include <chrono>
#include <cstdint>

#include <execinfo.h>
#include <elfutils/libdwfl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cxxabi.h>

static char* demangle_name(const char* mangled_name) {
	int status = -1;
	char* demangled = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
	if (status == 0 && demangled != nullptr) {
		return demangled;
	}
	free(demangled);
	return strdup(mangled_name);
}


void _print_stacktrace() {
	void *array[10];
	int size = backtrace(array, 10);

	Dwfl_Callbacks callbacks = {
		.find_elf = dwfl_linux_proc_find_elf,
		.find_debuginfo = dwfl_standard_find_debuginfo
	};
	Dwfl *dwfl = dwfl_begin(&callbacks);
	if (!dwfl) {
		printf("Failed to initialize dwfl\n");
		return;
	}

	int report_result = dwfl_linux_proc_report(dwfl, getpid());
	int err = dwfl_errno();
	if (err != 0 || report_result != 0) {
		printf("!!!! dwfl_linux_proc_report failed: %s\n", dwfl_errmsg(err));
		dwfl_end(dwfl);
		return;
	}

	dwfl_report_end(dwfl, nullptr, nullptr);

	for (int i = 0; i < size; i++) {
		Dwarf_Addr addr = (Dwarf_Addr)(uintptr_t)array[i];
		Dwfl_Module *mod = dwfl_addrmodule(dwfl, addr);
		err = dwfl_errno();
		if (err != 0) {
			//printf("!!!! dwfl_addrmodule failed: %s\n", dwfl_errmsg(err));
		}

		// Check if module is valid
		const char *module_name = mod ? dwfl_module_info(mod, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) : nullptr;
		const char *mangled_name = mod ? dwfl_module_addrname(mod, addr) : nullptr;
		char *func = mangled_name ? demangle_name(mangled_name) : strdup("??");
		const char *file = nullptr;
		int line = 0;

		Dwfl_Line *srcline = mod ? dwfl_module_getsrc(mod, addr) : nullptr;
		err = dwfl_errno();
		if (err != 0) {
			//printf("!!!! dwfl_module_getsrc failed: %s\n", dwfl_errmsg(err));
		}

		if (srcline) {
			Dwarf_Addr line_addr;
			file = dwfl_lineinfo(srcline, &line_addr, &line, nullptr, nullptr, nullptr);
		}

		file = file ? file : "??";
		line = line != -1 ? line : 0;
		module_name = module_name ? module_name : "??";

		// Print with diagnostics
		if (mod) {
			printf("[%d] %s at %s:%d (addr: 0x%lx)\n",
					i, func, file, line, (unsigned long)addr);
		}

		free(func);
	}

	dwfl_end(dwfl);
}

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

void Omake::print_stacktrace() {
	_print_stacktrace();
}

void Omake::_bind_methods() {
	ClassDB::bind_static_method("Omake", D_METHOD("print_stacktrace"), &Omake::print_stacktrace);
	ClassDB::bind_static_method("Omake", D_METHOD("get_cpu_ticks_nsec"), &Omake::get_cpu_ticks_nsec);
}
