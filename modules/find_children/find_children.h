#ifndef MODULE_FIND_CHILDREN_H
#define MODULE_FIND_CHILDREN_H


//#include "scene/main/node.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "modules/packed_node_ptr_array/packed_node_ptr_array.h"

using namespace godot;

class Node;
//class PackedNodePtrArray;

class Find : public RefCounted {
    GDCLASS(Find, RefCounted)

private:

protected:
    static void _bind_methods();

public:
    Find();
    ~Find();

    static PackedNodePtrArray *children(const Node* node, bool p_include_internal = true);
    static PackedNodePtrArray *all(const Node* node);
    static PackedNodePtrArray *by(const Node* node, const String &p_pattern, const String &p_type, const bool p_recursive = true, const bool p_owned = true);
    static PackedNodePtrArray *by_name(const Node* node, const String &p_node_name);
    static PackedNodePtrArray *by_type(const Node* node, const String &p_type_name);
    static PackedNodePtrArray *by_group(const Node* node, const String &p_group_name);
    static PackedNodePtrArray *by_groups(const Node* node, const TypedArray<String> &p_group_names);
};

#endif
