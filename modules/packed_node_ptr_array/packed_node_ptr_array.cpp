
#include "packed_node_ptr_array.h"
#include "scene/main/node.h"

PackedNodePtrArray::PackedNodePtrArray() {
	//print_line("PackedNodePtrArray created");
}

PackedNodePtrArray::~PackedNodePtrArray() {
	//print_line("PackedNodePtrArray destroyed");
	nodes.clear();
}

LocalVector<Node *> *PackedNodePtrArray::get_node_ptr() {
	return &nodes;
}

void PackedNodePtrArray::add_node(Node *p_node) {
	//	fprintf(stderr, "!!!! called PackedNodePtrArray::add_node\n"); fflush(stderr);
	nodes.push_back(p_node);
}

Node *PackedNodePtrArray::get_node(int p_index) const {
	//	fprintf(stderr, "!!!! called PackedNodePtrArray::get\n"); fflush(stderr);
	if (p_index >= 0 && p_index < (int)nodes.size()) {
		return nodes[p_index];
	}
	return nullptr;
}

void PackedNodePtrArray::set(int p_index, Node *p_node) {
	nodes[p_index] = p_node;
}

int PackedNodePtrArray::size() const {
	return (int)nodes.size();
}

void PackedNodePtrArray::resize(int p_new_size) {
	nodes.resize(p_new_size);
}

void PackedNodePtrArray::clear() {
	nodes.resize(0);
}

Node *PackedNodePtrArray::front() const {
	if (nodes.size() == 0) {
		return nullptr;
	}
	return nodes[0];
}

Node *PackedNodePtrArray::back() const {
	if (nodes.size() == 0) {
		return nullptr;
	}
	int i = nodes.size() - 1;
	return nodes[i];
}

Node *PackedNodePtrArray::pick_random() const {
	if (nodes.size() == 0) {
		return nullptr;
	}

	int i = Math::rand() % nodes.size();
	return nodes[i];
}

TypedArray<Node> PackedNodePtrArray::to_array() const {
	TypedArray<Node> retval;
	int ic = nodes.size();
	retval.resize_uninitialized(ic);
	for (int i = 0; i < ic; i++) {
		retval.set(i, nodes[i]);
	}
	return retval;
}

bool PackedNodePtrArray::is_empty() const {
	return nodes.size() > 0;
}

bool PackedNodePtrArray::_iter_init(const Variant &p_args) {
	//fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_init\n"); fflush(stderr);
	current_index = 0;
	return nodes.size() > 0;
}

bool PackedNodePtrArray::_iter_next(const Variant &p_args) {
	//fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_next: %d\n", current_index); fflush(stderr);
	current_index++;
	return current_index < nodes.size();
}

Node *PackedNodePtrArray::_iter_get(const Variant &p_args) {
	uint32_t idx = current_index;
	//fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_get: %d\n", idx); fflush(stderr);
	if (idx >= 0 && idx < nodes.size()) {
		return nodes[idx];
	}
	return nullptr;
}

void PackedNodePtrArray::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_node", "node"), &PackedNodePtrArray::add_node);
	ClassDB::bind_method(D_METHOD("get_node", "index"), &PackedNodePtrArray::get_node);
	ClassDB::bind_method(D_METHOD("size"), &PackedNodePtrArray::size);
	ClassDB::bind_method(D_METHOD("clear"), &PackedNodePtrArray::clear);

	ClassDB::bind_method(D_METHOD("front"), &PackedNodePtrArray::front);
	ClassDB::bind_method(D_METHOD("back"), &PackedNodePtrArray::back);
	ClassDB::bind_method(D_METHOD("pick_random"), &PackedNodePtrArray::pick_random);
	ClassDB::bind_method(D_METHOD("to_array"), &PackedNodePtrArray::to_array);
	ClassDB::bind_method(D_METHOD("is_empty"), &PackedNodePtrArray::is_empty);

	ClassDB::bind_method(D_METHOD("_iter_init", "args"), &PackedNodePtrArray::_iter_init);
	ClassDB::bind_method(D_METHOD("_iter_next", "args"), &PackedNodePtrArray::_iter_next);
	ClassDB::bind_method(D_METHOD("_iter_get", "arg"), &PackedNodePtrArray::_iter_get);
}
