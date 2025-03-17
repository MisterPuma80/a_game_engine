#include "register_types.h"

#include "core/object/class_db.h"
#include "omake.h"
#include "packed_node_ptr_array.h"
#include "modules/omake/omake_string_appender.h"

void initialize_omake_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Omake>();
	ClassDB::register_class<PackedNodePtrArray>();
	ClassDB::register_class<OmakeStringAppender>();
}

void uninitialize_omake_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
