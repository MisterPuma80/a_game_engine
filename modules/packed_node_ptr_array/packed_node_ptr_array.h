#ifndef MODULE_PACKED_NODE_PTR_ARRAY_H
#define MODULE_PACKED_NODE_PTR_ARRAY_H


//#include "scene/main/node.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

using namespace godot;

class Node;

class PackedNodePtrArray : public RefCounted {
    GDCLASS(PackedNodePtrArray, RefCounted)

private:
	Vector<Node*> items;
	//int current_index;

protected:
    static void _bind_methods();

public:
	//int current_size;

    PackedNodePtrArray();
    ~PackedNodePtrArray();

    //static PackedNodePtrArray* create2();
    //static void destroy2(PackedNodePtrArray* inst);
	Vector<Node*>* get_node_ptr();
    void add_node(Node *item);
    Node* get_node(int index) const;
	void set(int index, Node *item);
    int size() const;
	void resize(int new_size);
    void clear();

    //bool _iter_init(const Variant &args);
    //bool _iter_next(const Variant &args);
    //Node* _iter_get(const Variant &arg);
};

#endif