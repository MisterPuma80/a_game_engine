#ifndef OMAKE_FIND_H
#define OMAKE_FIND_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "modules/omake/packed_node_ptr_array.h"

class Node;

class OmakeFind {
public:
	static Ref<PackedNodePtrArray> children(const Node *p_node, const bool p_include_internal = true);
	static Ref<PackedNodePtrArray> all(const Node *p_node);
	static Ref<PackedNodePtrArray> by(const Node *p_node, const String &p_pattern, const String &p_type, const bool p_recursive = true, const bool p_owned = true);
	static Ref<PackedNodePtrArray> by_name(const Node *p_node, const String &p_node_name);
	static Ref<PackedNodePtrArray> by_type(const Node *p_node, const String &p_type_name);
	static Ref<PackedNodePtrArray> by_group(const Node *p_node, const String &p_group_name);
	static Ref<PackedNodePtrArray> by_groups(const Node *p_node, const TypedArray<String> &p_group_names);
};

#endif // OMAKE_FIND_H
