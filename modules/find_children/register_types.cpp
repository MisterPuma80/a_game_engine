#include "register_types.h"

#include "core/object/class_db.h"
#include "find_children.h"

void initialize_find_children_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Find>();
}

void uninitialize_find_children_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}