#include "register_types.h"

#include "core/object/class_db.h"
#include "packed_node_ptr_array.h"

void initialize_packed_node_ptr_array_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<PackedNodePtrArray>();
}

void uninitialize_packed_node_ptr_array_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}