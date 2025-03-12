#ifndef PACKED_NODE_PTR_ARRAY_H
#define PACKED_NODE_PTR_ARRAY_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

using namespace godot;

class Node;

class PackedNodePtrArray : public RefCounted {
	GDCLASS(PackedNodePtrArray, RefCounted)

private:
	mutable LocalVector<Node *> nodes;
	uint32_t current_index;

protected:
	static void _bind_methods();

public:
	PackedNodePtrArray();
	~PackedNodePtrArray();

	LocalVector<Node *> *get_node_ptr();
	void add_node(Node *p_node);
	Node *get_node(int p_index) const;
	void set(int p_index, Node *p_node);
	int size() const;
	void resize(int p_new_size);
	void clear();

	Node *front() const;
	Node *back() const;
	Node *pick_random() const;
	TypedArray<Node> to_array() const;
	bool is_empty() const;

	bool _iter_init(const Variant &p_args);
	bool _iter_next(const Variant &p_args);
	Node *_iter_get(const Variant &p_args);
};

#endif // PACKED_NODE_PTR_ARRAY_H
