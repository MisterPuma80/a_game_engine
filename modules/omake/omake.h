#ifndef OMAKE_H
#define OMAKE_H

#include "core/object/ref_counted.h"
#include "packed_node_ptr_array.h"
#include "modules/omake/omake_string_appender.h"

class Omake : public RefCounted {
	GDCLASS(Omake, RefCounted)

protected:
	static void _bind_methods();

public:
	Omake();
	~Omake();

	OmakeStringAppender get_appender();

	static uint64_t get_cpu_ticks_nsec();
	static Ref<PackedNodePtrArray> get_children(const Node *p_node, const bool p_include_internal = true);
	static Ref<PackedNodePtrArray> find_all(const Node *p_node);
	static Ref<PackedNodePtrArray> find_by(const Node *p_node, const String &p_pattern, const String &p_type, const bool p_recursive = true, const bool p_owned = true);
	static Ref<PackedNodePtrArray> find_by_name(const Node *p_node, const String &p_node_name);
	static Ref<PackedNodePtrArray> find_by_type(const Node *p_node, const String &p_type_name);
	static Ref<PackedNodePtrArray> find_by_group(const Node *p_node, const String &p_group_name);
	static Ref<PackedNodePtrArray> find_by_groups(const Node *p_node, const TypedArray<String> &p_group_names);
};

#endif // OMAKE_H
